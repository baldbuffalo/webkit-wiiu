/*
 * Copyright (c) 2013 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "config.h"

#include "helpers/WKCTextTrackCue.h"
#include "helpers/privates/WKCTextTrackCuePrivate.h"

#include "TextTrackCue.h"
#include "VTTCue.h"

namespace WKC {

TextTrackCuePrivate::TextTrackCuePrivate(WebCore::TextTrackCue* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_id()
    , m_vertical()
    , m_align()
    , m_text()
{
}

TextTrackCuePrivate::~TextTrackCuePrivate()
{
}

const String&
TextTrackCuePrivate::id()
{
    m_id = m_webcore->id().string(); // id() returns AtomString now
    return m_id;
}
double
TextTrackCuePrivate::startTime() const
{
    return m_webcore->startTime();
}
double
TextTrackCuePrivate::endTime() const
{
    return m_webcore->endTime();
}
bool
TextTrackCuePrivate::pauseOnExit() const
{
    return m_webcore->pauseOnExit();
}
// 2026: vertical/snapToLines/line/position/size/align moved from TextTrackCue to
// the VTTCue subclass and now return scoped enums / LineAndPositionSetting variants.
// Map them back to the legacy WKC string/int representation.
const String&
TextTrackCuePrivate::vertical()
{
    m_vertical = emptyString();
    if (auto* vtt = dynamicDowncast<WebCore::VTTCue>(m_webcore)) {
        switch (vtt->vertical()) {
        case WebCore::VTTDirectionSetting::VerticalGrowingLeft:  m_vertical = "rl"_s; break;
        case WebCore::VTTDirectionSetting::VerticalGrowingRight: m_vertical = "lr"_s; break;
        default: break; // Horizontal -> empty string
        }
    }
    return m_vertical;
}
bool
TextTrackCuePrivate::snapToLines() const
{
    if (auto* vtt = dynamicDowncast<WebCore::VTTCue>(m_webcore))
        return vtt->snapToLines();
    return false;
}
int
TextTrackCuePrivate::line() const
{
    if (auto* vtt = dynamicDowncast<WebCore::VTTCue>(m_webcore)) {
        auto value = vtt->line();
        if (std::holds_alternative<double>(value))
            return static_cast<int>(std::get<double>(value));
    }
    return -1; // "auto"
}
int
TextTrackCuePrivate::position() const
{
    if (auto* vtt = dynamicDowncast<WebCore::VTTCue>(m_webcore)) {
        auto value = vtt->position();
        if (std::holds_alternative<double>(value))
            return static_cast<int>(std::get<double>(value));
    }
    return -1; // "auto"
}
int
TextTrackCuePrivate::size() const
{
    if (auto* vtt = dynamicDowncast<WebCore::VTTCue>(m_webcore))
        return static_cast<int>(vtt->size());
    return 0;
}
const String&
TextTrackCuePrivate::align()
{
    m_align = emptyString();
    if (auto* vtt = dynamicDowncast<WebCore::VTTCue>(m_webcore)) {
        switch (vtt->align()) {
        case WebCore::VTTAlignSetting::Start:  m_align = "start"_s;  break;
        case WebCore::VTTAlignSetting::Center: m_align = "center"_s; break;
        case WebCore::VTTAlignSetting::End:    m_align = "end"_s;    break;
        case WebCore::VTTAlignSetting::Left:   m_align = "left"_s;   break;
        case WebCore::VTTAlignSetting::Right:  m_align = "right"_s;  break;
        }
    }
    return m_align;
}
const String&
TextTrackCuePrivate::text()
{
    m_text = m_webcore->text();
    return m_text;
}


TextTrackCue::TextTrackCue(TextTrackCuePrivate& parent)
    : m_private(parent)
{
}

TextTrackCue::~TextTrackCue()
{
}

const String&
TextTrackCue::id() const
{
    return m_private.id();
}
double
TextTrackCue::startTime() const
{
    return m_private.startTime();
}
double
TextTrackCue::endTime() const
{
    return m_private.endTime();
}
bool
TextTrackCue::pauseOnExit() const
{
    return m_private.pauseOnExit();
}
const String&
TextTrackCue::vertical()
{
    return m_private.vertical();
}
bool
TextTrackCue::snapToLines() const
{
    return m_private.snapToLines();
}
int
TextTrackCue::line() const
{
    return m_private.line();
}
int
TextTrackCue::position() const
{
    return m_private.position();
}
int
TextTrackCue::size() const
{
    return m_private.size();
}
const String&
TextTrackCue::align() const
{
    return m_private.align();
}
const String&
TextTrackCue::text() const
{
    return m_private.text();
}


} // namespace
