/*
 *  Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

// Real scalar backend for the slim simdutf surface WTF uses on Wii U (see
// stubs/wtf/simdutf/simdutf_impl.h for why the vendored amalgamation is not
// built here). These are straightforward, endian-aware scalar implementations
// of exactly the functions StringImpl / StringCommon / Base64 / UTF8Conversion
// call. UTF-8 -> UTF-16 delegates to WTF's own tested converter; UTF-16 -> UTF-8
// is implemented directly, because WTF::Unicode::convert(UTF16->UTF8) itself
// routes through simdutf and delegating back would recurse.

#include "config.h"
#include <wtf/simdutf/simdutf_impl.h>

#include <wtf/unicode/UTF8Conversion.h>

namespace simdutf {
namespace detail {

static inline uint16_t loadUnit(char16_t c, bool littleEndian)
{
    uint16_t v = static_cast<uint16_t>(c);
    if (littleEndian)
        v = static_cast<uint16_t>((v >> 8) | (v << 8));
    return v;
}

size_t convert_utf8_to_utf16(const char8_t* input, size_t length, char16_t* output) noexcept
{
    // The char8_t -> char16_t overload of WTF::Unicode::convert is a pure scalar
    // path (it does not route back through simdutf), so delegating is safe.
    auto result = WTF::Unicode::convert(std::span<const char8_t> { input, length }, std::span<char16_t> { output, length });
    if (result.code != WTF::Unicode::ConversionResultCode::Success)
        return 0;
    return result.buffer.size();
}

result convert_utf16_to_utf8_with_errors(const char16_t* input, size_t length, char8_t* output, bool littleEndian) noexcept
{
    size_t o = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t c = loadUnit(input[i], littleEndian);
        if (c >= 0xD800 && c <= 0xDBFF) {
            if (i + 1 >= length)
                return { SURROGATE, i };
            uint32_t low = loadUnit(input[i + 1], littleEndian);
            if (low < 0xDC00 || low > 0xDFFF)
                return { SURROGATE, i };
            c = 0x10000 + ((c - 0xD800) << 10) + (low - 0xDC00);
            ++i;
        } else if (c >= 0xDC00 && c <= 0xDFFF)
            return { SURROGATE, i };

        if (c < 0x80)
            output[o++] = static_cast<char8_t>(c);
        else if (c < 0x800) {
            output[o++] = static_cast<char8_t>(0xC0 | (c >> 6));
            output[o++] = static_cast<char8_t>(0x80 | (c & 0x3F));
        } else if (c < 0x10000) {
            output[o++] = static_cast<char8_t>(0xE0 | (c >> 12));
            output[o++] = static_cast<char8_t>(0x80 | ((c >> 6) & 0x3F));
            output[o++] = static_cast<char8_t>(0x80 | (c & 0x3F));
        } else {
            output[o++] = static_cast<char8_t>(0xF0 | (c >> 18));
            output[o++] = static_cast<char8_t>(0x80 | ((c >> 12) & 0x3F));
            output[o++] = static_cast<char8_t>(0x80 | ((c >> 6) & 0x3F));
            output[o++] = static_cast<char8_t>(0x80 | (c & 0x3F));
        }
    }
    return { SUCCESS, o };
}

size_t utf8_length_from_utf16(const char16_t* input, size_t length, bool littleEndian) noexcept
{
    size_t n = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t c = loadUnit(input[i], littleEndian);
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < length) {
            uint32_t low = loadUnit(input[i + 1], littleEndian);
            if (low >= 0xDC00 && low <= 0xDFFF) {
                n += 4;
                ++i;
                continue;
            }
        }
        if (c < 0x80)
            n += 1;
        else if (c < 0x800)
            n += 2;
        else
            n += 3;
    }
    return n;
}

bool validate_utf16(const char16_t* input, size_t length) noexcept
{
    for (size_t i = 0; i < length; ++i) {
        uint32_t c = static_cast<uint16_t>(input[i]);
        if (c >= 0xD800 && c <= 0xDBFF) {
            if (i + 1 >= length)
                return false;
            uint32_t low = static_cast<uint16_t>(input[i + 1]);
            if (low < 0xDC00 || low > 0xDFFF)
                return false;
            ++i;
        } else if (c >= 0xDC00 && c <= 0xDFFF)
            return false;
    }
    return true;
}

void to_well_formed_utf16(const char16_t* input, size_t length, char16_t* output) noexcept
{
    for (size_t i = 0; i < length; ++i) {
        uint32_t c = static_cast<uint16_t>(input[i]);
        if (c >= 0xD800 && c <= 0xDBFF) {
            if (i + 1 < length) {
                uint32_t low = static_cast<uint16_t>(input[i + 1]);
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    output[i] = input[i];
                    output[i + 1] = input[i + 1];
                    ++i;
                    continue;
                }
            }
            output[i] = static_cast<char16_t>(0xFFFD);
        } else if (c >= 0xDC00 && c <= 0xDFFF)
            output[i] = static_cast<char16_t>(0xFFFD);
        else
            output[i] = input[i];
    }
}

result validate_utf8_with_errors(const char8_t* input, size_t length) noexcept
{
    size_t i = 0;
    while (i < length) {
        uint8_t b = static_cast<uint8_t>(input[i]);
        size_t extra;
        uint32_t cp;
        uint32_t minimum;
        if (b < 0x80) {
            ++i;
            continue;
        } else if ((b & 0xE0) == 0xC0) {
            extra = 1;
            cp = b & 0x1F;
            minimum = 0x80;
        } else if ((b & 0xF0) == 0xE0) {
            extra = 2;
            cp = b & 0x0F;
            minimum = 0x800;
        } else if ((b & 0xF8) == 0xF0) {
            extra = 3;
            cp = b & 0x07;
            minimum = 0x10000;
        } else
            return { TOO_LONG, i };

        if (i + extra >= length)
            return { TOO_SHORT, i };
        for (size_t k = 1; k <= extra; ++k) {
            uint8_t cb = static_cast<uint8_t>(input[i + k]);
            if ((cb & 0xC0) != 0x80)
                return { TOO_SHORT, i };
            cp = (cp << 6) | (cb & 0x3F);
        }
        if (cp < minimum)
            return { OVERLONG, i };
        if (cp > 0x10FFFF)
            return { TOO_LARGE, i };
        if (cp >= 0xD800 && cp <= 0xDFFF)
            return { SURROGATE, i };
        i += extra + 1;
    }
    return { SUCCESS, length };
}

size_t utf16_length_from_utf8(const char8_t* input, size_t length) noexcept
{
    size_t n = 0;
    size_t i = 0;
    while (i < length) {
        uint8_t b = static_cast<uint8_t>(input[i]);
        if (b < 0x80) {
            n += 1;
            i += 1;
        } else if ((b & 0xE0) == 0xC0) {
            n += 1;
            i += 2;
        } else if ((b & 0xF0) == 0xE0) {
            n += 1;
            i += 3;
        } else if ((b & 0xF8) == 0xF0) {
            n += 2; // encodes as a surrogate pair
            i += 4;
        } else {
            n += 1;
            i += 1;
        }
    }
    return n;
}

// ---- base64 ----

static inline const char* base64Alphabet(bool url)
{
    return url
        ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
        : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}

// Default alphabet pads by default; the URL alphabet does not. base64_reverse_padding flips that.
static inline bool base64WantsPadding(base64_options options)
{
    bool url = options & base64_url;
    bool reverse = options & base64_reverse_padding;
    bool defaultPad = !url;
    return reverse ? !defaultPad : defaultPad;
}

size_t binary_to_base64(const uint8_t* input, size_t length, char* output, base64_options options) noexcept
{
    const char* table = base64Alphabet(options & base64_url);
    bool pad = base64WantsPadding(options);
    size_t o = 0;
    size_t i = 0;
    for (; i + 3 <= length; i += 3) {
        uint32_t n = (static_cast<uint32_t>(input[i]) << 16) | (static_cast<uint32_t>(input[i + 1]) << 8) | input[i + 2];
        output[o++] = table[(n >> 18) & 0x3F];
        output[o++] = table[(n >> 12) & 0x3F];
        output[o++] = table[(n >> 6) & 0x3F];
        output[o++] = table[n & 0x3F];
    }
    size_t remainder = length - i;
    if (remainder == 1) {
        uint32_t n = static_cast<uint32_t>(input[i]) << 16;
        output[o++] = table[(n >> 18) & 0x3F];
        output[o++] = table[(n >> 12) & 0x3F];
        if (pad) {
            output[o++] = '=';
            output[o++] = '=';
        }
    } else if (remainder == 2) {
        uint32_t n = (static_cast<uint32_t>(input[i]) << 16) | (static_cast<uint32_t>(input[i + 1]) << 8);
        output[o++] = table[(n >> 18) & 0x3F];
        output[o++] = table[(n >> 12) & 0x3F];
        output[o++] = table[(n >> 6) & 0x3F];
        if (pad)
            output[o++] = '=';
    }
    return o;
}

size_t base64_length_from_binary(size_t length, base64_options options) noexcept
{
    if (base64WantsPadding(options))
        return ((length + 2) / 3) * 4;
    size_t full = (length / 3) * 4;
    size_t remainder = length % 3;
    if (remainder)
        full += remainder + 1;
    return full;
}

size_t maximal_binary_length_from_base64(size_t inputLength) noexcept
{
    return (inputLength + 3) / 4 * 3;
}

// Decodes base64 leniently: ASCII whitespace is skipped, '=' terminates input.
// This covers atob()/base64Decode and the common Uint8Array.fromBase64 paths;
// the strict / stop_before_partial last-chunk distinctions are treated as loose.
result base64_to_binary_safe(const uint8_t* input, size_t length, size_t /*bytesPerInputUnit*/, bool inputIs16, uint8_t* output, size_t& outlen, base64_options options, last_chunk_handling_options /*lastChunk*/, bool decodeUpToBadChar) noexcept
{
    bool url = options & base64_url;
    const size_t capacity = outlen;

    auto readChar = [&](size_t index) -> uint32_t {
        if (inputIs16)
            return static_cast<uint16_t>(reinterpret_cast<const char16_t*>(input)[index]);
        return input[index];
    };
    auto decodeValue = [&](uint32_t c) -> int {
        if (c >= 'A' && c <= 'Z')
            return static_cast<int>(c - 'A');
        if (c >= 'a' && c <= 'z')
            return static_cast<int>(c - 'a' + 26);
        if (c >= '0' && c <= '9')
            return static_cast<int>(c - '0' + 52);
        if (url) {
            if (c == '-')
                return 62;
            if (c == '_')
                return 63;
        } else {
            if (c == '+')
                return 62;
            if (c == '/')
                return 63;
        }
        return -1;
    };
    auto isWhitespace = [](uint32_t c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
    };

    size_t o = 0;
    uint32_t accumulator = 0;
    int bits = 0;
    size_t i = 0;
    for (; i < length; ++i) {
        uint32_t c = readChar(i);
        if (c == '=')
            break;
        if (isWhitespace(c))
            continue;
        int value = decodeValue(c);
        if (value < 0) {
            outlen = o;
            if (decodeUpToBadChar)
                return { SUCCESS, i };
            return { INVALID_BASE64_CHARACTER, i };
        }
        accumulator = (accumulator << 6) | static_cast<uint32_t>(value);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            if (o >= capacity) {
                outlen = o;
                return { OUTPUT_BUFFER_TOO_SMALL, i };
            }
            output[o++] = static_cast<uint8_t>((accumulator >> bits) & 0xFF);
        }
    }
    outlen = o;
    return { SUCCESS, i };
}

} // namespace detail
} // namespace simdutf
