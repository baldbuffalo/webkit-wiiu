/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include "CSSValueKeywords.h"
#include "FileList.h"
#include "FloatRoundedRect.h"
#include "FontCascade.h"
#include "FontCascadeDescription.h"
#include "FontSelector.h"
#include "GraphicsContext.h"
#include "HTMLElementTypeHelpers.h"
#include "HTMLInputElement.h"
#include "HTMLMediaElement.h"
#include "HTMLMeterElement.h"
#include "HTMLNames.h"
#include "HTMLSelectElement.h"
#include "LocalizedStrings.h"
#include "PaintInfo.h"
#include "Path.h"
#include "RenderElement.h"
#include "RenderMeter.h"
#include "RenderProgress.h"
#include "RenderTheme.h"
#include "RenderTheme.h"
#include "RenderElementInlines.h"
#include "RenderObjectStyle.h"
#include "RenderElementStyleInlines.h"
#include "StringTruncator.h"
#include "TimeRanges.h"
#include <wtf/text/StringBuilder.h>

#include <wkc/wkcgpeer.h>
#include <wkc/wkcpeer.h>
#if ENABLE(VIDEO)
#include <wkc/wkcmediapeer.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// WKC peer struct helpers
static inline void WKCFloatRect_SetRect(WKCFloatRect* dst, const WKCFloatRect* src)
{
    *dst = *src;
}
static inline void WKCFloatSize_Set(WKCFloatSize* s, float w, float h)
{
    s->fWidth = w; s->fHeight = h;
}
static inline void WKCFloatPoint_Set(WKCFloatPoint* p, float x, float y)
{
    p->fX = x; p->fY = y;
}

namespace WebCore {

// ─── Color helper ────────────────────────────────────────────────────────────
static inline Color skinColor(unsigned int argb)
{
    uint8_t a = (argb >> 24) & 0xff;
    uint8_t r = (argb >> 16) & 0xff;
    uint8_t g = (argb >>  8) & 0xff;
    uint8_t b =  argb        & 0xff;
    return Color(SRGBA<uint8_t> { r, g, b, a });
}

class RenderThemeWKC final : public RenderTheme {
public:
    RenderThemeWKC() = default;
    ~RenderThemeWKC() override = default;

    // Not virtual in modern RenderTheme — no override
    void setCheckboxSize(RenderStyle&) const;
    void setRadioSize(RenderStyle&) const;
    bool controlSupportsTints(const RenderObject&) const;
    void systemFont(CSSValueID, FontCascadeDescription&) const;
    bool paintProgressBar(const RenderObject&, const PaintInfo&, const IntRect&);
    bool supportsMeter(StyleAppearance, const HTMLMeterElement&) const { return true; }
    bool paintMeter(const RenderObject&, const PaintInfo&, const IntRect&);

    void adjustButtonStyle(RenderStyle&, const Element*) const override;
    void adjustTextFieldStyle(RenderStyle&, const Element*) const override;
    void adjustTextAreaStyle(RenderStyle&, const Element*) const override;
    void adjustSearchFieldStyle(RenderStyle&, const Element*) const override;
    void adjustSearchFieldCancelButtonStyle(RenderStyle&, const Element*) const override;
    void adjustSearchFieldDecorationPartStyle(RenderStyle&, const Element*) const override;
    void adjustSearchFieldResultsDecorationPartStyle(RenderStyle&, const Element*) const override;
    void adjustSearchFieldResultsButtonStyle(RenderStyle&, const Element*) const override;
    void adjustSliderTrackStyle(RenderStyle&, const Element*) const override;
    void adjustSliderThumbStyle(RenderStyle&, const Element*) const override;
    void adjustSliderThumbSize(RenderStyle&, const Element*) const override;
    void adjustMenuListStyle(RenderStyle&, const Element*) const override;
    void adjustMenuListButtonStyle(RenderStyle&, const Element*) const override;
    bool isControlStyled(const RenderStyle&) const override;
    Color platformFocusRingColor(OptionSet<StyleColorOptions>) const override;
    Color systemColor(CSSValueID, OptionSet<StyleColorOptions>) const override;
    Color platformActiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformInactiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformActiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformInactiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const override;
    String extraDefaultStyleSheet() override;
    Seconds animationRepeatIntervalForProgressBar(const RenderProgress&) const override;
    void adjustProgressBarStyle(RenderStyle&, const Element*) const override { }
    void adjustMeterStyle(RenderStyle&, const Element*) const override { }
    bool shouldHaveSpinButton(const HTMLInputElement&) const override { return false; }
    void adjustInnerSpinButtonStyle(RenderStyle&, const Element*) const override { }
    bool popsMenuBySpaceOrReturn() const override { return true; }
    String fileListNameForWidth(const FileList*, const FontCascade&, int width, bool) const override;

    bool paintCheckbox(const RenderObject&, const PaintInfo&, const FloatRect&);
    bool paintRadio(const RenderObject&, const PaintInfo&, const FloatRect&);
    bool paintButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintTextField(const RenderObject&, const PaintInfo&, const FloatRect&);
    bool paintTextArea(const RenderObject&, const PaintInfo&, const FloatRect&);
    bool paintSearchField(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintSearchFieldCancelButton(const RenderBox&, const PaintInfo&, const IntRect&);
    bool paintSearchFieldDecorationPart(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintSearchFieldResultsDecorationPart(const RenderBox&, const PaintInfo&, const IntRect&);
    bool paintSearchFieldResultsButton(const RenderBox&, const PaintInfo&, const IntRect&);
    bool paintSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&);
    int minimumMenuListSize(const RenderStyle&) const;
    bool paintMenuList(const RenderObject&, const PaintInfo&, const FloatRect&);
    bool paintMenuListButton(const RenderObject&, const PaintInfo&, const FloatRect&);
    bool paintInnerSpinButton(const RenderObject&, const PaintInfo&, const IntRect&);

#if ENABLE(VIDEO)
    String formatMediaControlsTime(float) const;
    String formatMediaControlsCurrentTime(float, float) const;
    String formatMediaControlsRemainingTime(float, float) const;
    IntPoint volumeSliderOffsetFromMuteButton(RenderBox*, const IntSize&) const;
    bool paintMediaFullscreenButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaPlayButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaOverlayPlayButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaMuteButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaSeekBackButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaSeekForwardButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaVolumeSliderContainer(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaVolumeSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaVolumeSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaRewindButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaReturnToRealtimeButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaToggleClosedCaptionsButton(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaControlsBackground(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaCurrentTime(const RenderObject&, const PaintInfo&, const IntRect&);
    bool paintMediaTimeRemaining(const RenderObject&, const PaintInfo&, const IntRect&);
    String extraMediaControlsStyleSheet();
    bool paintMediaButton(int index, const RenderObject&, const PaintInfo&, const IntRect&, int state = WKC_IMAGE_MEDIA_STATE_NORMAL);
#endif

private:
    bool supportsFocus(StyleAppearance) const;
};

// ─── Singleton ────────────────────────────────────────────────────────────────
RenderTheme& RenderTheme::singleton()
{
    static RenderThemeWKC* gTheme = new RenderThemeWKC;
    return *gTheme;
}

bool RenderThemeWKC::isControlStyled(const RenderStyle& style) const
{
    return RenderTheme::isControlStyled(style);
}

bool RenderThemeWKC::controlSupportsTints(const RenderObject& o) const
{
    auto* re = dynamicDowncast<RenderElement>(o);
    if (!re || !isEnabled(*re))
        return false;
    if (o.style().usedAppearance() == StyleAppearance::Checkbox)
        return isChecked(*re);
    return true;
}

void RenderThemeWKC::systemFont(CSSValueID cssValueId, FontCascadeDescription& fontDescription) const
{
    int type = 0;
    switch (cssValueId) {
    case CSSValueCaption:            type = WKC_SYSTEMFONT_TYPE_CAPTION; break;
    case CSSValueIcon:               type = WKC_SYSTEMFONT_TYPE_ICON; break;
    case CSSValueMenu:               type = WKC_SYSTEMFONT_TYPE_MENU; break;
    case CSSValueMessageBox:         type = WKC_SYSTEMFONT_TYPE_MESSAGE_BOX; break;
    case CSSValueSmallCaption:       type = WKC_SYSTEMFONT_TYPE_SMALL_CAPTION; break;
    case CSSValueWebkitMiniControl:  type = WKC_SYSTEMFONT_TYPE_WEBKIT_MINI_CONTROL; break;
    case CSSValueWebkitSmallControl: type = WKC_SYSTEMFONT_TYPE_WEBKIT_SMALL_CONTROL; break;
    case CSSValueWebkitControl:      type = WKC_SYSTEMFONT_TYPE_WEBKIT_CONTROL; break;
    case CSSValueStatusBar:          type = WKC_SYSTEMFONT_TYPE_STATUS_BAR; break;
    default: return;
    }
    float size = wkcStockImageGetSystemFontSizePeer(type);
    const char* familyName = wkcStockImageGetSystemFontFamilyNamePeer(type);
    if (size && familyName) {
        fontDescription.setSpecifiedSize(size);
        fontDescription.setIsAbsoluteSize(true);
        // AtomString::fromLatin1 not available — construct via String
        fontDescription.setOneFamily(AtomString(String::fromLatin1(familyName)));
        fontDescription.setWeight(FontSelectionValue(400));
        fontDescription.setIsItalic(false);
    }
}

Color RenderThemeWKC::systemColor(CSSValueID cssValueId, OptionSet<StyleColorOptions>) const
{
    int id = 0;
    switch (cssValueId) {
    case CSSValueActiveborder:        id = WKC_SKINCOLOR_ACTIVEBORDER; break;
    case CSSValueActivecaption:       id = WKC_SKINCOLOR_ACTIVECAPTION; break;
    case CSSValueAppworkspace:        id = WKC_SKINCOLOR_APPWORKSPACE; break;
    case CSSValueBackground:          id = WKC_SKINCOLOR_BACKGROUND; break;
    case CSSValueButtonface:          id = WKC_SKINCOLOR_BUTTONFACE; break;
    case CSSValueButtonhighlight:     id = WKC_SKINCOLOR_BUTTONHIGHLIGHT; break;
    case CSSValueButtonshadow:        id = WKC_SKINCOLOR_BUTTONSHADOW; break;
    case CSSValueButtontext:          id = WKC_SKINCOLOR_BUTTONTEXT; break;
    case CSSValueCaptiontext:         id = WKC_SKINCOLOR_CAPTIONTEXT; break;
    case CSSValueGraytext:            id = WKC_SKINCOLOR_GRAYTEXT; break;
    case CSSValueHighlight:           id = WKC_SKINCOLOR_HIGHLIGHT; break;
    case CSSValueHighlighttext:       id = WKC_SKINCOLOR_HIGHLIGHTTEXT; break;
    case CSSValueInactiveborder:      id = WKC_SKINCOLOR_INACTIVEBORDER; break;
    case CSSValueInactivecaption:     id = WKC_SKINCOLOR_INACTIVECAPTION; break;
    case CSSValueInactivecaptiontext: id = WKC_SKINCOLOR_INACTIVECAPTIONTEXT; break;
    case CSSValueInfobackground:      id = WKC_SKINCOLOR_INFOBACKGROUND; break;
    case CSSValueInfotext:            id = WKC_SKINCOLOR_INFOTEXT; break;
    case CSSValueMenu:                id = WKC_SKINCOLOR_MENU; break;
    case CSSValueMenutext:            id = WKC_SKINCOLOR_MENUTEXT; break;
    case CSSValueScrollbar:           id = WKC_SKINCOLOR_SCROLLBAR; break;
    case CSSValueText:                id = WKC_SKINCOLOR_TEXT; break;
    case CSSValueThreeddarkshadow:    id = WKC_SKINCOLOR_THREEDDARKSHADOW; break;
    case CSSValueThreedface:          id = WKC_SKINCOLOR_THREEDFACE; break;
    case CSSValueThreedhighlight:     id = WKC_SKINCOLOR_THREEDHIGHLIGHTA; break;
    case CSSValueThreedlightshadow:   id = WKC_SKINCOLOR_THREEDLIGHTSHADOW; break;
    case CSSValueThreedshadow:        id = WKC_SKINCOLOR_THREEDSHADOW; break;
    case CSSValueWindow:              id = WKC_SKINCOLOR_WINDOW; break;
    case CSSValueWindowframe:         id = WKC_SKINCOLOR_WINDOWFRAME; break;
    case CSSValueWindowtext:          id = WKC_SKINCOLOR_WINDOWTEXT; break;
    default:
        return RenderTheme::systemColor(cssValueId, { });
    }
    return skinColor(wkcStockImageGetSkinColorPeer(id));
}

Color RenderThemeWKC::platformFocusRingColor(OptionSet<StyleColorOptions>) const
{
    return skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_FOCUSRING));
}

void RenderThemeWKC::setCheckboxSize(RenderStyle&) const { }
void RenderThemeWKC::setRadioSize(RenderStyle&) const { }
void RenderThemeWKC::adjustButtonStyle(RenderStyle&, const Element*) const { }
void RenderThemeWKC::adjustTextFieldStyle(RenderStyle&, const Element*) const { }
void RenderThemeWKC::adjustTextAreaStyle(RenderStyle& style, const Element* e) const { adjustTextFieldStyle(style, e); }
void RenderThemeWKC::adjustSearchFieldStyle(RenderStyle& style, const Element* e) const { adjustTextFieldStyle(style, e); }
void RenderThemeWKC::adjustSearchFieldCancelButtonStyle(RenderStyle&, const Element*) const { }
void RenderThemeWKC::adjustSearchFieldDecorationPartStyle(RenderStyle&, const Element*) const { }
void RenderThemeWKC::adjustSearchFieldResultsDecorationPartStyle(RenderStyle&, const Element*) const { }
void RenderThemeWKC::adjustSearchFieldResultsButtonStyle(RenderStyle&, const Element*) const { }
void RenderThemeWKC::adjustSliderThumbStyle(RenderStyle& style, const Element* e) const
{
    RenderTheme::adjustSliderThumbStyle(style, e);
}
void RenderThemeWKC::adjustSliderThumbSize(RenderStyle&, const Element*) const { }
void RenderThemeWKC::adjustSliderTrackStyle(RenderStyle& style, const Element* e) const
{
    RenderTheme::adjustSliderTrackStyle(style, e);
}
void RenderThemeWKC::adjustMenuListStyle(RenderStyle&, const Element*) const { }
void RenderThemeWKC::adjustMenuListButtonStyle(RenderStyle&, const Element*) const { }

Color RenderThemeWKC::platformActiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const
{
    return skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_ACTIVESELECTIONBACKGROUND));
}
Color RenderThemeWKC::platformInactiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const
{
    return skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_INACTIVESELECTIONBACKGROUND));
}
Color RenderThemeWKC::platformActiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const
{
    return skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_ACTIVESELECTIONFOREGROUND));
}
Color RenderThemeWKC::platformInactiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const
{
    return skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_INACTIVESELECTIONFOREGROUND));
}

String RenderThemeWKC::extraDefaultStyleSheet()
{
    const char* css = wkcStockImageGetDefaultStyleSheetPeer();
    return css ? String::fromLatin1(css) : String();
}

// ─── Drawing helpers ──────────────────────────────────────────────────────────
static void _bitblt(void* ctx, int type, void* bitmap, int rowbytes, void* mask, int maskrowbytes,
                    const WKCFloatRect* srcrect, const WKCFloatRect* destrect, int op)
{
    WKCPeerImage img = { 0 };
    img.fType = type;
    img.fBitmap = bitmap;
    img.fRowBytes = rowbytes;
    img.fMask = mask;
    img.fMaskRowBytes = maskrowbytes;
    WKCFloatRect_SetRect(&img.fSrcRect, srcrect);
    WKCFloatSize_Set(&img.fScale, 1, 1);
    WKCFloatSize_Set(&img.fiScale, 1, 1);
    WKCFloatPoint_Set(&img.fPhase, 0, 0);
    WKCFloatSize_Set(&img.fiTransform, 1, 1);
    wkcDrawContextBitBltPeer(ctx, &img, destrect, op);
}

static void calcSkinRect(unsigned int skinW, unsigned int skinH, const IntRect& r, WKCFloatRect& dest)
{
    if (r.width() < (int)skinW || r.height() < (int)skinH) {
        if (r.width() * skinH < r.height() * skinW) {
            dest.fSize.fWidth  = r.width();
            dest.fSize.fHeight = (float)skinH * r.width() / skinW;
        } else {
            dest.fSize.fWidth  = (float)skinW * r.height() / skinH;
            dest.fSize.fHeight = r.height();
        }
    } else {
        dest.fSize.fWidth  = skinW;
        dest.fSize.fHeight = skinH;
    }
    dest.fPoint.fX = r.x() + (r.width()  - dest.fSize.fWidth)  / 2;
    dest.fPoint.fY = r.y() + (r.height() - dest.fSize.fHeight) / 2;
}

static void drawScalingBitmapPeer(void* ctx, void* bitmap, int rowbytes,
                                  WKCSize* sz, const WKCPoint* pts, const WKCRect* dst, int op)
{
    WKCFloatRect src, dest;
    const int T = WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN;

#define DST_X  (dst->fPoint.fX)
#define DST_Y  (dst->fPoint.fY)
#define DST_W  (dst->fSize.fWidth)
#define DST_H  (dst->fSize.fHeight)

#define BLIT(sx,sy,sw,sh, dx,dy,dw,dh) \
    do { \
        src.fPoint.fX=sx; src.fPoint.fY=sy; src.fSize.fWidth=sw; src.fSize.fHeight=sh; \
        dest.fPoint.fX=dx; dest.fPoint.fY=dy; dest.fSize.fWidth=dw; dest.fSize.fHeight=dh; \
        if (sw>0&&sh>0&&dw>0&&dh>0) _bitblt(ctx,T,bitmap,rowbytes,nullptr,0,&src,&dest,op); \
    } while(0)

    // corners
    BLIT(0,0, pts[0].fX,pts[0].fY, DST_X,DST_Y, pts[0].fX,pts[0].fY);
    BLIT(pts[1].fX,0, sz->fWidth-pts[1].fX,pts[0].fY, DST_X+DST_W-(sz->fWidth-pts[1].fX),DST_Y, sz->fWidth-pts[1].fX,pts[0].fY);
    BLIT(0,pts[2].fY, pts[2].fX,sz->fHeight-pts[2].fY, DST_X,DST_Y+DST_H-(sz->fHeight-pts[2].fY), pts[2].fX,sz->fHeight-pts[2].fY);
    BLIT(pts[3].fX,pts[3].fY, sz->fWidth-pts[3].fX,sz->fHeight-pts[3].fY, DST_X+DST_W-(sz->fWidth-pts[3].fX),DST_Y+DST_H-(sz->fHeight-pts[3].fY), sz->fWidth-pts[3].fX,sz->fHeight-pts[3].fY);
    // edges
    BLIT(pts[0].fX,0, pts[1].fX-pts[0].fX,pts[0].fY, DST_X+pts[0].fX,DST_Y, DST_W-pts[0].fX-(sz->fWidth-pts[1].fX),pts[0].fY);
    BLIT(0,pts[0].fY, pts[0].fX,pts[2].fY-pts[0].fY, DST_X,DST_Y+pts[0].fY, pts[0].fX,DST_H-pts[0].fY-(sz->fHeight-pts[2].fY));
    BLIT(pts[1].fX,pts[1].fY, sz->fWidth-pts[1].fX,pts[3].fY-pts[1].fY, DST_X+DST_W-(sz->fWidth-pts[1].fX),DST_Y+pts[1].fY, sz->fWidth-pts[1].fX,DST_H-pts[1].fY-(sz->fHeight-pts[3].fY));
    BLIT(pts[2].fX,pts[2].fY, pts[3].fX-pts[2].fX,sz->fHeight-pts[2].fY, DST_X+pts[2].fX,DST_Y+DST_H-(sz->fHeight-pts[2].fY), DST_W-pts[2].fX-(sz->fWidth-pts[3].fX),sz->fHeight-pts[2].fY);
    // center
    BLIT(pts[0].fX,pts[0].fY, pts[3].fX-pts[0].fX,pts[3].fY-pts[0].fY, DST_X+pts[0].fX,DST_Y+pts[0].fY, DST_W-pts[0].fX-(sz->fWidth-pts[3].fX),DST_H-pts[0].fY-(sz->fHeight-pts[3].fY));

#undef BLIT
#undef DST_X
#undef DST_Y
#undef DST_W
#undef DST_H
}

static void _setBorder(GraphicsContext& ctx, const Color& color, float thickness)
{
    ctx.setStrokeColor(color);
    ctx.setStrokeThickness(thickness);
    ctx.setStrokeStyle(StrokeStyle::SolidStroke);
}

static bool paintStockImage(const RenderObject& o, const PaintInfo& i, const IntRect& r, int index)
{
    void* ctx = i.context().platformContext();
    if (!ctx) return false;
    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return false;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return false;
    WKCFloatRect src = { {0.0f, 0.0f}, {(float)w, (float)h} };
    WKCFloatRect dest;
    calcSkinRect(w, h, r, dest);
    _bitblt(ctx, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN,
            (void*)buf, w * 4, nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return false;
}

// ─── Checkbox / Radio ─────────────────────────────────────────────────────────
bool RenderThemeWKC::paintCheckbox(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    auto* re = dynamicDowncast<RenderElement>(o);
    int index;
    if (re && isEnabled(*re)) {
        if (isPressed(*re))
            index = WKC_IMAGE_CHECKBOX_PRESSED;
        else if (isFocused(*re) || isHovered(*re))
            index = isChecked(*re) ? WKC_IMAGE_CHECKBOX_CHECKED_FOCUSED : WKC_IMAGE_CHECKBOX_UNCHECKED_FOCUSED;
        else
            index = isChecked(*re) ? WKC_IMAGE_CHECKBOX_CHECKED : WKC_IMAGE_CHECKBOX_UNCHECKED;
    } else {
        index = re && isChecked(*re) ? WKC_IMAGE_CHECKBOX_CHECKED_DISABLED : WKC_IMAGE_CHECKBOX_UNCHECKED_DISABLED;
    }
    return paintStockImage(o, i, IntRect(r), index);
}

bool RenderThemeWKC::paintRadio(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    auto* re = dynamicDowncast<RenderElement>(o);
    int index;
    if (re && isEnabled(*re)) {
        if (isPressed(*re))
            index = WKC_IMAGE_RADIO_PRESSED;
        else if (isFocused(*re) || isHovered(*re))
            index = isChecked(*re) ? WKC_IMAGE_RADIO_CHECKED_FOCUSED : WKC_IMAGE_RADIO_UNCHECKED_FOCUSED;
        else
            index = isChecked(*re) ? WKC_IMAGE_RADIO_CHECKED : WKC_IMAGE_RADIO_UNCHECKED;
    } else {
        index = re && isChecked(*re) ? WKC_IMAGE_RADIO_CHECKED_DISABLED : WKC_IMAGE_RADIO_UNCHECKED_DISABLED;
    }
    return paintStockImage(o, i, IntRect(r), index);
}

// ─── Button ───────────────────────────────────────────────────────────────────
bool RenderThemeWKC::paintButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    void* ctx = i.context().platformContext();
    if (!ctx) return false;
    auto* re = dynamicDowncast<RenderElement>(o);
    int index;
    if (re && isEnabled(*re)) {
        index = WKC_IMAGE_BUTTON;
        if (isHovered(*re) || isFocused(*re)) index = WKC_IMAGE_BUTTON_HOVERED;
        if (isPressed(*re))                   index = WKC_IMAGE_BUTTON_PRESSED;
    } else {
        index = WKC_IMAGE_BUTTON_DISABLED;
    }
    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return false;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return false;
    const WKCPoint* pts = wkcStockImageGetLayoutPointsPeer(index);
    if (!pts) return false;
    WKCSize sz = { (int)w, (int)h };
    WKCRect rect = { {r.x(), r.y()}, {r.width(), r.height()} };
    drawScalingBitmapPeer(ctx, (void*)buf, w * 4, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return false;
}

// ─── TextField / TextArea / Search ────────────────────────────────────────────
bool RenderThemeWKC::paintTextField(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    const Color borderColor = skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BORDER));
    i.context().save();
    _setBorder(i.context(), borderColor, 1.0f);
    Color bg = skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BACKGROUND));
    i.context().setFillColor(bg);
    i.context().drawRect(IntRect(r));
    i.context().restore();
    return false;
}

bool RenderThemeWKC::paintTextArea(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    return paintTextField(o, i, r);
}

bool RenderThemeWKC::paintSearchField(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    return paintTextField(o, i, FloatRect(r));
}

bool RenderThemeWKC::paintSearchFieldCancelButton(const RenderBox& o, const PaintInfo& i, const IntRect& r)
{
    return paintStockImage(o, i, r, WKC_IMAGE_SEARCHFIELD_CANCELBUTTON);
}

bool RenderThemeWKC::paintSearchFieldDecorationPart(const RenderObject&, const PaintInfo&, const IntRect&) { return false; }
bool RenderThemeWKC::paintSearchFieldResultsDecorationPart(const RenderBox&, const PaintInfo&, const IntRect&) { return true; }

bool RenderThemeWKC::paintSearchFieldResultsButton(const RenderBox& o, const PaintInfo& i, const IntRect& r)
{
    return paintStockImage(o, i, r, WKC_IMAGE_SEARCHFIELD_RESULTBUTTON);
}

// ─── MenuList ─────────────────────────────────────────────────────────────────
int RenderThemeWKC::minimumMenuListSize(const RenderStyle& style) const
{
    int fs = style.computedFontSize();
    if (fs >= 13) return 9;
    if (fs >= 11) return 5;
    return 0;
}

bool RenderThemeWKC::paintMenuList(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    paintTextField(o, i, r);
    return paintMenuListButton(o, i, r);
}

bool RenderThemeWKC::paintMenuListButton(const RenderObject& o, const PaintInfo& i, const FloatRect& fr)
{
    const IntRect r(fr);
    void* ctx = i.context().platformContext();
    if (!ctx) return false;
    auto* re = dynamicDowncast<RenderElement>(o);
    int index;
    if (re && isEnabled(*re)) {
        if (isPressed(*re))                        index = WKC_IMAGE_MENU_LIST_BUTTON_PRESSED;
        else if (isFocused(*re) || isHovered(*re)) index = WKC_IMAGE_MENU_LIST_BUTTON_FOCUSED;
        else                                       index = WKC_IMAGE_MENU_LIST_BUTTON;
    } else {
        index = WKC_IMAGE_MENU_LIST_BUTTON_DISABLED;
    }
    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return false;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return false;
    const WKCPoint* pts = wkcStockImageGetLayoutPointsPeer(index);
    if (!pts) return false;
    const int minimumHeight = 8;
    unsigned int rowbytes = w * 4;
    WKCFloatRect src, dest;
    if (r.height() <= minimumHeight) {
        src.fPoint.fX = 0; src.fPoint.fY = (float)pts[1].fY;
        src.fSize.fWidth = (float)w; src.fSize.fHeight = (float)(pts[2].fY - pts[1].fY);
        dest.fPoint.fX = (float)(r.x() + r.width() - (int)w);
        dest.fPoint.fY = (float)(r.y() + (r.height() - src.fSize.fHeight) / 2);
        dest.fSize.fWidth = (float)w; dest.fSize.fHeight = src.fSize.fHeight;
        _bitblt(ctx, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN,
                (void*)buf, rowbytes, nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
        return false;
    }
    WKCSize sz = { (int)w, (int)h };
    WKCRect rect = { {r.x(), r.y()}, {r.width(), r.height()} };
    drawScalingBitmapPeer(ctx, (void*)buf, rowbytes, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return false;
}

// ─── Slider ───────────────────────────────────────────────────────────────────
bool RenderThemeWKC::paintSliderTrack(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    IntRect rect(r);
    auto* re = dynamicDowncast<RenderElement>(o);
    const Color borderColor = skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_RANGE_BORDER));
    auto appearance = o.style().usedAppearance();
    if (appearance == StyleAppearance::SliderHorizontal) {
        rect.setHeight(4);
        rect.move(0, (r.height() - 4) / 2);
    } else if (appearance == StyleAppearance::SliderVertical) {
        rect.setWidth(4);
        rect.move((r.width() - 4) / 2, 0);
    } else {
        return false;
    }
    i.context().save();
    _setBorder(i.context(), borderColor, 1.0f);
    Color bg = (re && !isEnabled(*re))
        ? skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_RANGE_BACKGROUND_DISABLED))
        : skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_RANGE_BACKGROUND_DISABLED));
    i.context().setFillColor(bg);
    i.context().drawRect(rect);
    i.context().restore();
    return false;
}

bool RenderThemeWKC::paintSliderThumb(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    auto* re = dynamicDowncast<RenderElement>(o);
    bool horizontal = (o.style().usedAppearance() == StyleAppearance::SliderThumbHorizontal);
    bool vertical   = (o.style().usedAppearance() == StyleAppearance::SliderThumbVertical);
    if (!horizontal && !vertical) return false;
    int index;
    if (re && isEnabled(*re)) {
        index = horizontal ? WKC_IMAGE_H_RANGE : WKC_IMAGE_V_RANGE;
        if (isHovered(*re) || isFocused(*re))
            index = horizontal ? WKC_IMAGE_H_RANGE_HOVERED : WKC_IMAGE_V_RANGE_HOVERED;
        if (isPressed(*re))
            index = horizontal ? WKC_IMAGE_H_RANGE_PRESSED : WKC_IMAGE_V_RANGE_PRESSED;
    } else {
        index = horizontal ? WKC_IMAGE_H_RANGE_DISABLED : WKC_IMAGE_V_RANGE_DISABLED;
    }
    void* ctx = i.context().platformContext();
    if (!ctx) return false;
    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return false;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return false;
    WKCFloatRect src = { {0.0f, 0.0f}, {(float)w, (float)h} };
    WKCFloatRect dest = { {(float)r.x(), (float)(r.y() + (r.height() - (int)h) / 2)}, {(float)w, (float)h} };
    _bitblt(ctx, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN,
            (void*)buf, w * 4, nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return false;
}

// ─── File list ────────────────────────────────────────────────────────────────
typedef void (*ResolveFilenameForDisplayProc)(const unsigned short* path, int pathLen,
                                              unsigned short* outPath, int* outLen, int maxLen);
static ResolveFilenameForDisplayProc gResolveFilenameForDisplayProc = nullptr;

void RenderTheme_SetResolveFilenameForDisplayProc(ResolveFilenameForDisplayProc proc)
{
    gResolveFilenameForDisplayProc = proc;
}

String RenderThemeWKC::fileListNameForWidth(const FileList* fileList, const FontCascade& font,
                                             int width, bool multipleFilesAllowed) const
{
    if (width <= 0)
        return String();
    if (fileList->length() > 1)
        return StringTruncator::rightTruncate(multipleFileUploadText(fileList->length()), width, font);

    String str;
    if (gResolveFilenameForDisplayProc && !fileList->isEmpty()) {
        String file = fileList->item(0)->path();
        unsigned short buf[MAX_PATH] = { 0 };
        int len = 0;
        auto charsResult = file.charactersWithNullTermination();
        if (charsResult.has_value()) {
            (*gResolveFilenameForDisplayProc)(
                reinterpret_cast<const unsigned short*>(charsResult->span().data()),
                file.length(), buf, &len, MAX_PATH);
            str = String(std::span<const char16_t>(reinterpret_cast<const char16_t*>(buf), len));
        } else {
            str = file;
        }
    } else {
        str = fileList->isEmpty() ? fileButtonNoFileSelectedLabel() : fileList->item(0)->path();
    }
    return StringTruncator::centerTruncate(str, width, font);
}

// ─── Progress bar ─────────────────────────────────────────────────────────────
static const double gProgressFrameRate = 0.033;

Seconds RenderThemeWKC::animationRepeatIntervalForProgressBar(const RenderProgress&) const
{
    return Seconds(gProgressFrameRate);
}

bool RenderThemeWKC::paintProgressBar(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    if (!is<RenderProgress>(o)) return true;
    const auto& renderProgress = downcast<RenderProgress>(o);
    auto* re = dynamicDowncast<RenderElement>(o);

    i.context().save();
    _setBorder(i.context(), skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BORDER)), 1.0f);

    Color bg = (re && !isEnabled(*re))
        ? skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BACKGROUND_DISABLED))
        : skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BACKGROUND));

    i.context().setFillColor(bg);
    i.context().drawRect(r);

    IntRect vr, vrr;
    if (renderProgress.isDeterminate()) {
        vr = r;
        vr.expand(-2, -2);
        vr.move(1, 1);
        vr.setWidth((int)(vr.width() * renderProgress.position()));
    } else {
        vr = r;
        vr.expand(-2, -2);
        int mx = vr.maxX() - 1;
        int x = (int)(vr.width() * renderProgress.animationProgress());
        vr.move(x, 1);
        vr.setWidth((int)(vr.width() * 0.25));
        if (vr.maxX() > mx) {
            vrr = r;
            vrr.expand(-2, -2);
            vrr.move(1, 1);
            vrr.setWidth(vr.maxX() - mx);
            vr.expand(-(int)vrr.width(), 0);
        }
    }

    _setBorder(i.context(), Color(), 0.0f);
    i.context().setFillColor(skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BODY)));
    i.context().fillRect(FloatRect(vr));
    if (!vrr.isEmpty())
        i.context().fillRect(FloatRect(vrr));

    i.context().restore();
    return true;
}

// ─── Meter ────────────────────────────────────────────────────────────────────
bool RenderThemeWKC::paintMeter(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    if (!is<RenderMeter>(o)) return true;
    const auto& renderMeter = downcast<RenderMeter>(o);
    auto* re = dynamicDowncast<RenderElement>(o);

    i.context().save();
    _setBorder(i.context(), skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BORDER)), 1.0f);

    Color bg = (re && !isEnabled(*re))
        ? skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BACKGROUND_DISABLED))
        : skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BACKGROUND));

    i.context().setFillColor(bg);
    i.context().drawRect(r);

    double value = 0;
    // Avoid dynamicDowncast<HTMLMeterElement> — use tag check instead
    if (auto* node = renderMeter.element()) {
        if (node->localName() == "meter")
            value = static_cast<HTMLMeterElement*>(node)->valueRatio();
    }

    IntRect vr = r;
    vr.expand(-2, -2);
    vr.move(1, 1);
    vr.setWidth((int)(vr.width() * value));

    _setBorder(i.context(), Color(), 0.0f);
    i.context().setFillColor(skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BODY)));
    i.context().fillRect(FloatRect(vr));
    i.context().restore();
    return true;
}

// ─── Spin button ──────────────────────────────────────────────────────────────
static bool _paintSpinButton(const RenderObject& o, const PaintInfo& i, const IntRect& r, int index, bool up)
{
    void* ctx = i.context().platformContext();
    if (!ctx) return false;
    const unsigned char* buf = wkcStockImageGetBitmapPeer(index);
    if (!buf) return false;
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(index, &w, &h);
    if (!w || !h) return false;
    const WKCPoint* pts = wkcStockImageGetLayoutPointsPeer(index);
    if (!pts) return false;

    WKCSize sz = { (int)w, (int)h };
    WKCRect rect = { {r.x(), r.y()}, {r.width(), r.height()} };
    drawScalingBitmapPeer(ctx, (void*)buf, w * 4, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);

    int cx0 = rect.fPoint.fX + pts[0].fX;
    int cy0 = rect.fPoint.fY + pts[0].fY;
    int cw  = rect.fSize.fWidth  - pts[0].fX - (sz.fWidth  - pts[3].fX);
    int ch  = rect.fSize.fHeight - pts[0].fY - (sz.fHeight - pts[3].fY);
    int cx1 = cx0 + cw, cy1 = cy0 + ch;
    int cx  = cx0 + cw / 2;

    Path path;
    if (up) {
        path.moveTo(FloatPoint(cx0, cy1));
        path.addLineTo(FloatPoint(cx, cy0));
        path.addLineTo(FloatPoint(cx1, cy1));
        path.closeSubpath();
    } else {
        path.moveTo(FloatPoint(cx0, cy0));
        path.addLineTo(FloatPoint(cx, cy1));
        path.addLineTo(FloatPoint(cx1, cy0));
        path.closeSubpath();
    }
    i.context().save();
    i.context().setFillColor(skinColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_BUTTONTEXT)));
    i.context().fillPath(path);
    i.context().restore();
    return false;
}

bool RenderThemeWKC::paintInnerSpinButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    IntRect r1(r), r2(r);
    r1.setHeight(r.height() / 2);
    r2.setHeight(r.height() / 2);
    r2.move(0, r1.height());

    auto* re = dynamicDowncast<RenderElement>(o);
    int idx1, idx2;
    if (re && isEnabled(*re)) {
        idx1 = (isHovered(*re) && isSpinUpButtonPartHovered(*re)) ? WKC_IMAGE_BUTTON_HOVERED :
               (isPressed(*re) && isSpinUpButtonPartPressed(*re)) ? WKC_IMAGE_BUTTON_PRESSED  : WKC_IMAGE_BUTTON;
        idx2 = (isHovered(*re) && !isSpinUpButtonPartHovered(*re)) ? WKC_IMAGE_BUTTON_HOVERED :
               (isPressed(*re) && !isSpinUpButtonPartPressed(*re)) ? WKC_IMAGE_BUTTON_PRESSED  : WKC_IMAGE_BUTTON;
    } else {
        idx1 = idx2 = WKC_IMAGE_BUTTON_DISABLED;
    }

    _paintSpinButton(o, i, r1, idx1, true);
    _paintSpinButton(o, i, r2, idx2, false);
    return true;
}

bool RenderThemeWKC::supportsFocus(StyleAppearance part) const
{
    switch (part) {
    case StyleAppearance::Button:
    case StyleAppearance::DefaultButton:
    case StyleAppearance::Radio:
    case StyleAppearance::Checkbox:
    case StyleAppearance::TextField:
    case StyleAppearance::SearchField:
    case StyleAppearance::TextArea:
        return true;
    default:
        return false;
    }
}

// ─── Video / Media ────────────────────────────────────────────────────────────
#if ENABLE(VIDEO)

String RenderThemeWKC::formatMediaControlsTime(float time) const
{
    if (!std::isfinite(time)) time = 0;
    int seconds = (int)std::fabsf(time);
    int hours   = seconds / 3600;
    int minutes = (seconds / 60) % 60;
    seconds %= 60;
    char buf[32];
    if (hours)
        snprintf(buf, sizeof(buf), "%s%01d:%02d:%02d", time < 0 ? "-" : "", hours, minutes, seconds);
    else
        snprintf(buf, sizeof(buf), "%s%01d:%02d", time < 0 ? "-" : "", minutes, seconds);
    return String::fromLatin1(buf);
}

String RenderThemeWKC::formatMediaControlsCurrentTime(float cur, float dur) const
{
    return RenderTheme::formatMediaControlsCurrentTime(cur, dur);
}

String RenderThemeWKC::formatMediaControlsRemainingTime(float cur, float dur) const
{
    return RenderTheme::formatMediaControlsRemainingTime(cur, dur);
}

IntPoint RenderThemeWKC::volumeSliderOffsetFromMuteButton(RenderBox*, const IntSize&) const
{
    return IntPoint(0, 0);
}

static void _fillRect(const RenderObject& o, const PaintInfo& i, const IntRect& r, unsigned int color)
{
    i.context().save();
    i.context().setFillColor(skinColor(color));
    i.context().drawRect(r);
    i.context().restore();
}

static bool hasSource(const HTMLMediaElement* m)
{
    return m->networkState() != HTMLMediaElement::NETWORK_EMPTY
        && m->networkState() != HTMLMediaElement::NETWORK_NO_SOURCE;
}

bool RenderThemeWKC::paintMediaButton(int index, const RenderObject& o, const PaintInfo& i, const IntRect& r, int state)
{
    void* ctx = i.context().platformContext();
    if (!ctx) return false;
    auto* re = dynamicDowncast<RenderElement>(o);

    HTMLMediaElement* m = toParentMediaElement(o);
    if (m && !hasSource(m)) {
        state = WKC_IMAGE_MEDIA_STATE_DISABLED;
        if (index == WKC_IMAGE_MEDIA_PAUSE_BUTTON)
            index = WKC_IMAGE_MEDIA_PLAY_BUTTON;
    } else if (re) {
        if (isHovered(*re) || isFocused(*re)) state = WKC_IMAGE_MEDIA_STATE_HOVERED;
        if (isPressed(*re))                   state = WKC_IMAGE_MEDIA_STATE_PRESSED;
    }

    const unsigned char* buf = wkcMediaPlayerSkinGetBitmapPeer(index, state);
    if (!buf) return false;
    unsigned int w = 0, h = 0;
    wkcMediaPlayerSkinGetSizePeer(index, &w, &h);
    if (!w || !h) return false;
    const WKCPoint* pts = wkcMediaPlayerSkinGetLayoutPointsPeer(index, state);
    if (!pts) return false;

    WKCSize sz = { (int)w, (int)h };
    WKCRect rect = { {r.x(), r.y()}, {r.width(), r.height()} };
    drawScalingBitmapPeer(ctx, (void*)buf, w * 4, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return true;
}

bool RenderThemeWKC::paintMediaFullscreenButton(const RenderObject& o, const PaintInfo& i, const IntRect& r) { return paintMediaButton(WKC_IMAGE_MEDIA_FULLSCREEN_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaSeekBackButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)   { return paintMediaButton(WKC_IMAGE_MEDIA_SEEKBACK_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaSeekForwardButton(const RenderObject& o, const PaintInfo& i, const IntRect& r){ return paintMediaButton(WKC_IMAGE_MEDIA_SEEKFORWARD_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaRewindButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)     { return paintMediaButton(WKC_IMAGE_MEDIA_REWIND_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaReturnToRealtimeButton(const RenderObject& o, const PaintInfo& i, const IntRect& r) { return paintMediaButton(WKC_IMAGE_MEDIA_RETURNTOREALTIME_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaToggleClosedCaptionsButton(const RenderObject& o, const PaintInfo& i, const IntRect& r) { return paintMediaButton(WKC_IMAGE_MEDIA_TOGGLECLOSEDCAPTION_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaSliderThumb(const RenderObject& o, const PaintInfo& i, const IntRect& r)      { return paintMediaButton(WKC_IMAGE_MEDIA_SLIDER_THUMB, o, i, r); }

bool RenderThemeWKC::paintMediaPlayButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    HTMLMediaElement* m = toParentMediaElement(o);
    if (!m) return false;
    int type = (m->paused() || m->ended()) ? WKC_IMAGE_MEDIA_PLAY_BUTTON : WKC_IMAGE_MEDIA_PAUSE_BUTTON;
    return paintMediaButton(type, o, i, r);
}

bool RenderThemeWKC::paintMediaOverlayPlayButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    HTMLMediaElement* m = toParentMediaElement(o);
    if (!m) return false;
    if (!m->paused() && !m->ended() && hasSource(m)) return false;
    return paintMediaButton(WKC_IMAGE_MEDIA_OVERLAY_PLAY_BUTTON, o, i, r);
}

bool RenderThemeWKC::paintMediaMuteButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    HTMLMediaElement* m = toParentMediaElement(o);
    if (!m) return false;
    int state = m->hasAudio() ? WKC_IMAGE_MEDIA_STATE_NORMAL : WKC_IMAGE_MEDIA_STATE_DISABLED;
    int type  = m->muted() ? WKC_IMAGE_MEDIA_VOLUME_BUTTON : WKC_IMAGE_MEDIA_MUTE_BUTTON;
    return paintMediaButton(type, o, i, r, state);
}

bool RenderThemeWKC::paintMediaSliderTrack(const RenderObject& o, const PaintInfo& info, const IntRect& r)
{
    HTMLMediaElement* m = toParentMediaElement(o);
    if (!m) return false;
    float duration = m->duration();
    if (!duration) return false;
    float currentTime = m->currentTime();

    unsigned int thumbW = 0, thumbH = 0;
    wkcMediaPlayerSkinGetSizePeer(WKC_IMAGE_MEDIA_SLIDER_THUMB, &thumbW, &thumbH);

    int offset = (thumbW / 2) * (1.0f - 2.0f * currentTime / duration);
    int curPos = (int)(currentTime / duration * r.width()) + offset;

    int br = r.height() / 2;
    IntSize radii(br, br);

    info.context().save();
    FloatRoundedRect bgRect(r, radii, radii, radii, radii);
    info.context().fillRoundedRect(bgRect, Color(0, 0, 0));

    IntRect playedR(r);
    playedR.setWidth(curPos);
    if (br <= playedR.width()) {
        FloatRoundedRect pr(playedR, radii, IntSize(), radii, IntSize());
        info.context().fillRoundedRect(pr, Color(0, 196, 222));
    }

    auto buffered = m->buffered();
    for (unsigned idx = 0; idx < buffered->length(); ++idx) {
        float start = buffered->start(idx, ASSERT_NO_EXCEPTION);
        float end   = buffered->end(idx, ASSERT_NO_EXCEPTION);
        if (start <= currentTime && currentTime <= end) {
            IntRect bufR(r);
            bufR.move(curPos, 0);
            bufR.setWidth((int)(end / duration * r.width()) - curPos);
            if (br <= bufR.width()) {
                FloatRoundedRect bfr(bufR, IntSize(), radii, IntSize(), radii);
                info.context().fillRoundedRect(bfr, Color(0, 78, 89));
            }
            break;
        }
    }
    info.context().restore();
    return true;
}

bool RenderThemeWKC::paintMediaVolumeSliderContainer(const RenderObject& o, const PaintInfo& i, const IntRect& r) { _fillRect(o, i, r, 0x80888888); return false; }
bool RenderThemeWKC::paintMediaVolumeSliderTrack(const RenderObject& o, const PaintInfo& i, const IntRect& r)     { _fillRect(o, i, r, 0xff880088); return true; }
bool RenderThemeWKC::paintMediaVolumeSliderThumb(const RenderObject& o, const PaintInfo& i, const IntRect& r)     { _fillRect(o, i, r, 0xff888800); return true; }
bool RenderThemeWKC::paintMediaControlsBackground(const RenderObject& o, const PaintInfo& i, const IntRect& r)   { _fillRect(o, i, r, 0x00000000); return true; }
bool RenderThemeWKC::paintMediaCurrentTime(const RenderObject& o, const PaintInfo& i, const IntRect& r)          { _fillRect(o, i, r, 0x00000000); return true; }
bool RenderThemeWKC::paintMediaTimeRemaining(const RenderObject& o, const PaintInfo& i, const IntRect& r)        { _fillRect(o, i, r, 0x00000000); return true; }

String RenderThemeWKC::extraMediaControlsStyleSheet()
{
    const char* css = wkcMediaPlayerSkinGetStyleSheetPeer();
    return css ? String::fromLatin1(css) : String();
}

#endif // ENABLE(VIDEO)

} // namespace WebCore
