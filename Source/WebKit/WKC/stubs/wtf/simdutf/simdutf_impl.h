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

// Slim Wii U replacement for the vendored simdutf amalgamation header.
//
// The bundled simdutf states in its own source that it "does not support 32-bit
// platforms"; the Wii U's PowerPC 750 (Espresso) is 32-bit AND big-endian, an
// untested combination whose scalar fallback fails to compile (a uint16_t* /
// char16_t* mismatch in convert_latin1_to_utf16). Rather than build the
// unsupported amalgamation, this header exposes exactly the simdutf surface the
// four WTF consumers use (StringImpl, StringCommon, Base64, UTF8Conversion) and
// binds it to a real scalar backend in wtf/wiiu/SIMDUTFWKC.cpp. It is found
// ahead of Source/WTF/wtf/simdutf via the WKC stubs include dir, and
// Source/WTF/wtf/SIMDUTF.cpp (which pulls the amalgamation .cpp) is excluded
// from the build.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <tuple>
#include <type_traits>

namespace simdutf {

enum error_code {
    SUCCESS = 0,
    HEADER_BITS,
    TOO_SHORT,
    TOO_LONG,
    OVERLONG,
    TOO_LARGE,
    SURROGATE,
    INVALID_BASE64_CHARACTER,
    BASE64_INPUT_REMAINDER,
    BASE64_EXTRA_BITS,
    OUTPUT_BUFFER_TOO_SMALL,
    OTHER
};

struct result {
    error_code error { SUCCESS };
    size_t count { 0 }; // On success: code units written/validated. On error: input position of the error.
};

// Matches the vendored simdutf bit layout: bit0 = URL alphabet, bit1 = reverse
// the alphabet's default padding (default alphabet pads by default; URL does not).
enum base64_options {
    base64_default = 0,
    base64_url = 1,
    base64_reverse_padding = 2,
    base64_default_no_padding = base64_default | base64_reverse_padding,
    base64_url_with_padding = base64_url | base64_reverse_padding,
};

enum class last_chunk_handling_options {
    loose,
    strict,
    stop_before_partial,
};

// ---- real scalar backend (defined in wtf/wiiu/SIMDUTFWKC.cpp) ----
namespace detail {
size_t convert_utf8_to_utf16(const char8_t*, size_t, char16_t*) noexcept;
result convert_utf16_to_utf8_with_errors(const char16_t*, size_t, char8_t*, bool inputIsLittleEndian) noexcept;
size_t utf8_length_from_utf16(const char16_t*, size_t, bool inputIsLittleEndian) noexcept;
bool validate_utf16(const char16_t*, size_t) noexcept;
void to_well_formed_utf16(const char16_t*, size_t, char16_t*) noexcept;
result validate_utf8_with_errors(const char8_t*, size_t) noexcept;
size_t utf16_length_from_utf8(const char8_t*, size_t) noexcept;
size_t binary_to_base64(const uint8_t*, size_t, char*, base64_options) noexcept;
size_t base64_length_from_binary(size_t, base64_options) noexcept;
size_t maximal_binary_length_from_base64(size_t inputLength) noexcept;
result base64_to_binary_safe(const uint8_t*, size_t, size_t bytesPerInputUnit, bool inputIs16, uint8_t*, size_t& outlen, base64_options, last_chunk_handling_options, bool decodeUpToBadChar) noexcept;
} // namespace detail

// ---- public span API used by WTF ----

template<typename In, typename Out>
inline size_t convert_utf8_to_utf16(In&& input, Out&& output) noexcept
{
    return detail::convert_utf8_to_utf16(reinterpret_cast<const char8_t*>(std::data(input)), std::size(input), reinterpret_cast<char16_t*>(std::data(output)));
}

inline result convert_utf16be_to_utf8_with_errors(std::span<const char16_t> input, std::span<char8_t> output) noexcept
{
    return detail::convert_utf16_to_utf8_with_errors(input.data(), input.size(), output.data(), /* little-endian input */ false);
}

inline result convert_utf16le_to_utf8_with_errors(std::span<const char16_t> input, std::span<char8_t> output) noexcept
{
    return detail::convert_utf16_to_utf8_with_errors(input.data(), input.size(), output.data(), /* little-endian input */ true);
}

inline size_t utf8_length_from_utf16be(std::span<const char16_t> input) noexcept
{
    return detail::utf8_length_from_utf16(input.data(), input.size(), false);
}

inline size_t utf8_length_from_utf16le(std::span<const char16_t> input) noexcept
{
    return detail::utf8_length_from_utf16(input.data(), input.size(), true);
}

inline bool validate_utf16(std::span<const char16_t> input) noexcept
{
    return detail::validate_utf16(input.data(), input.size());
}

inline void to_well_formed_utf16(std::span<const char16_t> input, std::span<char16_t> output) noexcept
{
    detail::to_well_formed_utf16(input.data(), input.size(), output.data());
}

inline result validate_utf8_with_errors(std::span<const char8_t> input) noexcept
{
    return detail::validate_utf8_with_errors(input.data(), input.size());
}

inline size_t utf16_length_from_utf8(std::span<const char8_t> input) noexcept
{
    return detail::utf16_length_from_utf8(input.data(), input.size());
}

template<typename In, typename Out>
inline size_t binary_to_base64(In&& input, Out&& output, base64_options options = base64_default) noexcept
{
    return detail::binary_to_base64(reinterpret_cast<const uint8_t*>(std::data(input)), std::size(input), reinterpret_cast<char*>(std::data(output)), options);
}

inline size_t base64_length_from_binary(size_t length, base64_options options = base64_default) noexcept
{
    return detail::base64_length_from_binary(length, options);
}

template<typename In>
inline size_t maximal_binary_length_from_base64(In&& input) noexcept
{
    return detail::maximal_binary_length_from_base64(std::size(input));
}

template<typename In, typename Out>
inline std::tuple<result, size_t> base64_to_binary_safe(In&& input, Out&& output, base64_options options = base64_default, last_chunk_handling_options lastChunk = last_chunk_handling_options::loose, bool decodeUpToBadChar = false) noexcept
{
    using Element = std::remove_cv_t<std::remove_reference_t<decltype(*std::data(input))>>;
    size_t outlen = std::size(output);
    result r = detail::base64_to_binary_safe(
        reinterpret_cast<const uint8_t*>(std::data(input)), std::size(input),
        sizeof(Element), sizeof(Element) == sizeof(char16_t),
        reinterpret_cast<uint8_t*>(std::data(output)), outlen,
        options, lastChunk, decodeUpToBadChar);
    return { r, outlen };
}

} // namespace simdutf
