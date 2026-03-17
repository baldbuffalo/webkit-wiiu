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
#include "HTMLInputElement.h"
#include "HTMLMediaElement.h"
#include "HTMLMeterElement.h"
#include "HTMLSelectElement.h"
#include "LocalizedStrings.h"
#include "PaintInfo.h"
#include "Path.h"
#include "RenderMeter.h"
#include "RenderProgress.h"
#include "RenderTheme.h"
#include "StringTruncator.h"
#include "TimeRanges.h"
#include <wtf/text/StringBuilder.h>

#include <wkc/wkcgpeer.h>
#include <wkc/wkcpeer.h>
#if ENABLE(VIDEO)
#include <wkc/wkcmediapeer.h>
#endif

namespace WebCore {

class RenderThemeWKC final : public RenderTheme {
public:
    static Ref<RenderTheme> create();
    ~RenderThemeWKC() override = default;

    bool supportsHover(const RenderStyle&) const override { return true; }
    bool supportsFocusRing(const RenderStyle&) const override { return false; }

    bool paintCheckbox(const RenderObject&, const PaintInfo&, const FloatRect&) override;
    void setCheckboxSize(RenderStyle&) const override;

    bool paintRadio(const RenderObject&, const PaintInfo&, const FloatRect&) override;
    void setRadioSize(RenderStyle&) const override;

    void adjustButtonStyle(RenderStyle&, const Element*) const override;
    bool paintButton(const RenderObject&, const PaintInfo&, const IntRect&) override;

    void adjustTextFieldStyle(RenderStyle&, const Element*) const override;
    bool paintTextField(const RenderObject&, const PaintInfo&, const FloatRect&) override;

    void adjustTextAreaStyle(RenderStyle&, const Element*) const override;
    bool paintTextArea(const RenderObject&, const PaintInfo&, const FloatRect&) override;

    void adjustSearchFieldStyle(RenderStyle&, const Element*) const override;
    bool paintSearchField(const RenderObject&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldCancelButtonStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldCancelButton(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldDecorationPartStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldDecorationPart(const RenderObject&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldResultsDecorationPartStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldResultsDecorationPart(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSearchFieldResultsButtonStyle(RenderStyle&, const Element*) const override;
    bool paintSearchFieldResultsButton(const RenderBox&, const PaintInfo&, const IntRect&) override;

    void adjustSliderTrackStyle(RenderStyle&, const Element*) const override;
    bool paintSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&) override;

    void adjustSliderThumbStyle(RenderStyle&, const Element*) const override;
    bool paintSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&) override;

    void adjustSliderThumbSize(RenderStyle&, const Element*) const override;

    int minimumMenuListSize(const RenderStyle&) const override;

    void adjustMenuListStyle(RenderStyle&, const Element*) const override;
    bool paintMenuList(const RenderObject&, const PaintInfo&, const FloatRect&) override;

    void adjustMenuListButtonStyle(RenderStyle&, const Element*) const override;
    bool paintMenuListButton(const RenderObject&, const PaintInfo&, const FloatRect&) override;

    bool isControlStyled(const RenderStyle&, const RenderStyle& userAgentStyle) const override;
    bool controlSupportsTints(const RenderObject&) const override;

    Color platformFocusRingColor(OptionSet<StyleColorOptions>) const override;

    void systemFont(CSSValueID, FontCascadeDescription&) const override;
    Color systemColor(CSSValueID, OptionSet<StyleColorOptions>) const override;

    Color platformActiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformInactiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformActiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const override;
    Color platformInactiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const override;

    int popupInternalPaddingLeft(const RenderStyle&, const Settings&) const override;
    int popupInternalPaddingRight(const RenderStyle&, const Settings&) const override;
    int popupInternalPaddingTop(const RenderStyle&, const Settings&) const override;
    int popupInternalPaddingBottom(const RenderStyle&, const Settings&) const override;

    String extraDefaultStyleSheet() override;
    String extraQuirksStyleSheet() override;

#if ENABLE(VIDEO)
    bool supportsClosedCaptioning() const override { return false; }
    bool hasOwnDisabledStateHandlingFor(ControlPart) const override { return false; }
    bool usesMediaControlVolumeSlider() const override { return true; }
    double mediaControlsFadeInDuration() override { return 0.1; }
    double mediaControlsFadeOutDuration() override { return 0.3; }
    String formatMediaControlsTime(float time) const override;
    String formatMediaControlsCurrentTime(float currentTime, float duration) const override;
    String formatMediaControlsRemainingTime(float currentTime, float duration) const override;
    IntPoint volumeSliderOffsetFromMuteButton(RenderBox*, const IntSize&) const override;

    bool paintMediaFullscreenButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaPlayButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaOverlayPlayButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaMuteButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaSeekBackButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaSeekForwardButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaVolumeSliderContainer(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaVolumeSliderTrack(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaVolumeSliderThumb(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaRewindButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaReturnToRealtimeButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaToggleClosedCaptionsButton(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaControlsBackground(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaCurrentTime(const RenderObject&, const PaintInfo&, const IntRect&) override;
    bool paintMediaTimeRemaining(const RenderObject&, const PaintInfo&, const IntRect&) override;
    String extraMediaControlsStyleSheet() override;

    bool paintMediaButton(int index, const RenderObject&, const PaintInfo&, const IntRect&, int state = WKC_IMAGE_MEDIA_STATE_NORMAL);
#endif

    double animationRepeatIntervalForProgressBar(const RenderProgress&) const override;
    double animationDurationForProgressBar(const RenderProgress&) const override;
    void adjustProgressBarStyle(RenderStyle&, const Element*) const override { }
    bool paintProgressBar(const RenderObject&, const PaintInfo&, const IntRect&) override;

    bool supportsMeter(ControlPart, const HTMLMeterElement&) const override { return true; }
    void adjustMeterStyle(RenderStyle&, const Element*) const override { }
    bool paintMeter(const RenderObject&, const PaintInfo&, const IntRect&) override;

    bool shouldHaveSpinButton(const HTMLInputElement&) const override { return false; }
    void adjustInnerSpinButtonStyle(RenderStyle&, const Element*) const override;
    bool paintInnerSpinButton(const RenderObject&, const PaintInfo&, const IntRect&) override;

    bool delegatesMenuListRendering() const override {
#if defined(__ghs__) || defined(__ARMCC_VERSION)
        return true;
#else
        return false;
#endif
    }
    bool popsMenuByArrowKeys() const override { return false; }
    bool popsMenuBySpaceOrReturn() const override { return true; }

    String fileListNameForWidth(const FileList*, const FontCascade&, int width, bool multipleFilesAllowed) const override;

private:
    void addIntrinsicMargins(RenderStyle&) const;
    bool supportsFocus(ControlPart) const;
};

// Popup padding constants
static const int cPopupInternalPaddingLeft   = 4;
static const int cPopupInternalPaddingRight  = 4;
static const int cPopupInternalPaddingTop    = 1;
static const int cPopupInternalPaddingBottom = 1;

Ref<RenderTheme> RenderThemeWKC::create()
{
    return adoptRef(*new RenderThemeWKC());
}

RenderTheme& RenderTheme::singleton()
{
    static RenderTheme* gTheme = &RenderThemeWKC::create().leakRef();
    return *gTheme;
}

bool RenderThemeWKC::isControlStyled(const RenderStyle& style, const RenderStyle& userAgentStyle) const
{
    switch (style.appearance()) {
    case TextFieldPart:
    case TextAreaPart:
    case ListboxPart:
        return style.border() != userAgentStyle.border();
    default:
        return RenderTheme::isControlStyled(style, userAgentStyle);
    }
}

bool RenderThemeWKC::controlSupportsTints(const RenderObject& o) const
{
    if (!isEnabled(o))
        return false;
    if (o.style().appearance() == CheckboxPart)
        return isChecked(o);
    return true;
}

void RenderThemeWKC::systemFont(CSSValueID cssValueId, FontCascadeDescription& fontDescription) const
{
    int type = 0;
    switch (cssValueId) {
    case CSSValueCaption:           type = WKC_SYSTEMFONT_TYPE_CAPTION; break;
    case CSSValueIcon:              type = WKC_SYSTEMFONT_TYPE_ICON; break;
    case CSSValueMenu:              type = WKC_SYSTEMFONT_TYPE_MENU; break;
    case CSSValueMessageBox:        type = WKC_SYSTEMFONT_TYPE_MESSAGE_BOX; break;
    case CSSValueSmallCaption:      type = WKC_SYSTEMFONT_TYPE_SMALL_CAPTION; break;
    case CSSValueWebkitMiniControl: type = WKC_SYSTEMFONT_TYPE_WEBKIT_MINI_CONTROL; break;
    case CSSValueWebkitSmallControl:type = WKC_SYSTEMFONT_TYPE_WEBKIT_SMALL_CONTROL; break;
    case CSSValueWebkitControl:     type = WKC_SYSTEMFONT_TYPE_WEBKIT_CONTROL; break;
    case CSSValueStatusBar:         type = WKC_SYSTEMFONT_TYPE_STATUS_BAR; break;
    default:
        return;
    }
    float size = wkcStockImageGetSystemFontSizePeer(type);
    const char* familyName = wkcStockImageGetSystemFontFamilyNamePeer(type);
    if (size && familyName) {
        fontDescription.setSpecifiedSize(size);
        fontDescription.setIsAbsoluteSize(true);
        fontDescription.setGenericFamily(FontDescription::NoFamily);
        fontDescription.setOneFamily(familyName);
        fontDescription.setWeight(FontSelectionValue(400));
        fontDescription.setItalic(FontSelectionValue(0));
    }
}

Color RenderThemeWKC::systemColor(CSSValueID cssValueId, OptionSet<StyleColorOptions>) const
{
    int id = 0;
    switch (cssValueId) {
    case CSSValueActiveborder:         id = WKC_SKINCOLOR_ACTIVEBORDER; break;
    case CSSValueActivecaption:        id = WKC_SKINCOLOR_ACTIVECAPTION; break;
    case CSSValueAppworkspace:         id = WKC_SKINCOLOR_APPWORKSPACE; break;
    case CSSValueBackground:           id = WKC_SKINCOLOR_BACKGROUND; break;
    case CSSValueButtonface:           id = WKC_SKINCOLOR_BUTTONFACE; break;
    case CSSValueButtonhighlight:      id = WKC_SKINCOLOR_BUTTONHIGHLIGHT; break;
    case CSSValueButtonshadow:         id = WKC_SKINCOLOR_BUTTONSHADOW; break;
    case CSSValueButtontext:           id = WKC_SKINCOLOR_BUTTONTEXT; break;
    case CSSValueCaptiontext:          id = WKC_SKINCOLOR_CAPTIONTEXT; break;
    case CSSValueGraytext:             id = WKC_SKINCOLOR_GRAYTEXT; break;
    case CSSValueHighlight:            id = WKC_SKINCOLOR_HIGHLIGHT; break;
    case CSSValueHighlighttext:        id = WKC_SKINCOLOR_HIGHLIGHTTEXT; break;
    case CSSValueInactiveborder:       id = WKC_SKINCOLOR_INACTIVEBORDER; break;
    case CSSValueInactivecaption:      id = WKC_SKINCOLOR_INACTIVECAPTION; break;
    case CSSValueInactivecaptiontext:  id = WKC_SKINCOLOR_INACTIVECAPTIONTEXT; break;
    case CSSValueInfobackground:       id = WKC_SKINCOLOR_INFOBACKGROUND; break;
    case CSSValueInfotext:             id = WKC_SKINCOLOR_INFOTEXT; break;
    case CSSValueMenu:                 id = WKC_SKINCOLOR_MENU; break;
    case CSSValueMenutext:             id = WKC_SKINCOLOR_MENUTEXT; break;
    case CSSValueScrollbar:            id = WKC_SKINCOLOR_SCROLLBAR; break;
    case CSSValueText:                 id = WKC_SKINCOLOR_TEXT; break;
    case CSSValueThreeddarkshadow:     id = WKC_SKINCOLOR_THREEDDARKSHADOW; break;
    case CSSValueThreedface:           id = WKC_SKINCOLOR_THREEDFACE; break;
    case CSSValueThreedhighlight:      id = WKC_SKINCOLOR_THREEDHIGHLIGHTA; break;
    case CSSValueThreedlightshadow:    id = WKC_SKINCOLOR_THREEDLIGHTSHADOW; break;
    case CSSValueThreedshadow:         id = WKC_SKINCOLOR_THREEDSHADOW; break;
    case CSSValueWindow:               id = WKC_SKINCOLOR_WINDOW; break;
    case CSSValueWindowframe:          id = WKC_SKINCOLOR_WINDOWFRAME; break;
    case CSSValueWindowtext:           id = WKC_SKINCOLOR_WINDOWTEXT; break;
    default:
        return RenderTheme::systemColor(cssValueId, { });
    }
    return Color(wkcStockImageGetSkinColorPeer(id));
}

Color RenderThemeWKC::platformFocusRingColor(OptionSet<StyleColorOptions>) const
{
    return Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_FOCUSRING));
}

void RenderThemeWKC::addIntrinsicMargins(RenderStyle& style) const
{
    if (style.fontSize() < 11)
        return;
    const int m = 2;
    if (style.width().isIntrinsicOrAuto()) {
        if (style.marginLeft().quirk())
            style.setMarginLeft(Length(m, LengthType::Fixed));
        if (style.marginRight().quirk())
            style.setMarginRight(Length(m, LengthType::Fixed));
    }
    if (style.height().isAuto()) {
        if (style.marginTop().quirk())
            style.setMarginTop(Length(m, LengthType::Fixed));
        if (style.marginBottom().quirk())
            style.setMarginBottom(Length(m, LengthType::Fixed));
    }
}

// ─── Drawing helpers ────────────────────────────────────────────────────────

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
            dest.fWidth  = r.width();
            dest.fHeight = (float)skinH * r.width() / skinW;
        } else {
            dest.fWidth  = (float)skinW * r.height() / skinH;
            dest.fHeight = r.height();
        }
    } else {
        dest.fWidth  = skinW;
        dest.fHeight = skinH;
    }
    dest.fX = r.x() + (r.width()  - dest.fWidth)  / 2;
    dest.fY = r.y() + (r.height() - dest.fHeight) / 2;
}

static void drawScalingBitmapPeer(void* ctx, void* bitmap, int rowbytes,
                                  WKCSize* size, const WKCPoint* pts, const WKCRect* dst, int op)
{
    WKCFloatRect src, dest;
    const int T = WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN;

#define BLIT(sx,sy,sw,sh, dx,dy,dw,dh) \
    do { \
        src.fX=sx; src.fY=sy; src.fWidth=sw; src.fHeight=sh; \
        dest.fX=dx; dest.fY=dy; dest.fWidth=dw; dest.fHeight=dh; \
        if (sw>0&&sh>0&&dw>0&&dh>0) _bitblt(ctx,T,bitmap,rowbytes,0,0,&src,&dest,op); \
    } while(0)

    // corners
    BLIT(0,0, pts[0].fX,pts[0].fY, dst->fX,dst->fY, pts[0].fX,pts[0].fY);
    BLIT(pts[1].fX,0, size->fWidth-pts[1].fX,pts[0].fY, dst->fX+dst->fWidth-(size->fWidth-pts[1].fX),dst->fY, size->fWidth-pts[1].fX,pts[0].fY);
    BLIT(0,pts[2].fY, pts[2].fX,size->fHeight-pts[2].fY, dst->fX,dst->fY+dst->fHeight-(size->fHeight-pts[2].fY), pts[2].fX,size->fHeight-pts[2].fY);
    BLIT(pts[3].fX,pts[3].fY, size->fWidth-pts[3].fX,size->fHeight-pts[3].fY, dst->fX+dst->fWidth-(size->fWidth-pts[3].fX),dst->fY+dst->fHeight-(size->fHeight-pts[3].fY), size->fWidth-pts[3].fX,size->fHeight-pts[3].fY);
    // edges
    BLIT(pts[0].fX,0, pts[1].fX-pts[0].fX,pts[0].fY, dst->fX+pts[0].fX,dst->fY, dst->fWidth-pts[0].fX-(size->fWidth-pts[1].fX),pts[0].fY);
    BLIT(0,pts[0].fY, pts[0].fX,pts[2].fY-pts[0].fY, dst->fX,dst->fY+pts[0].fY, pts[0].fX,dst->fHeight-pts[0].fY-(size->fHeight-pts[2].fY));
    BLIT(pts[1].fX,pts[1].fY, size->fWidth-pts[1].fX,pts[3].fY-pts[1].fY, dst->fX+dst->fWidth-(size->fWidth-pts[1].fX),dst->fY+pts[1].fY, size->fWidth-pts[1].fX,dst->fHeight-pts[1].fY-(size->fHeight-pts[3].fY));
    BLIT(pts[2].fX,pts[2].fY, pts[3].fX-pts[2].fX,size->fHeight-pts[2].fY, dst->fX+pts[2].fX,dst->fY+dst->fHeight-(size->fHeight-pts[2].fY), dst->fWidth-pts[2].fX-(size->fWidth-pts[3].fX),size->fHeight-pts[2].fY);
    // center
    BLIT(pts[0].fX,pts[0].fY, pts[3].fX-pts[0].fX,pts[3].fY-pts[0].fY, dst->fX+pts[0].fX,dst->fY+pts[0].fY, dst->fWidth-pts[0].fX-(size->fWidth-pts[3].fX),dst->fHeight-pts[0].fY-(size->fHeight-pts[3].fY));
#undef BLIT
}

static void _setBorder(GraphicsContext& ctx, const Color& color, float thickness)
{
    ctx.setStrokeColor(color);
    ctx.setStrokeThickness(thickness);
    ctx.setStrokeStyle(StrokeStyle::SolidStroke);
}

// ─── Checkbox / Radio ───────────────────────────────────────────────────────

void RenderThemeWKC::setCheckboxSize(RenderStyle& style) const
{
    if (!style.width().isIntrinsicOrAuto() && !style.height().isAuto())
        return;
    unsigned int w = 0, dummy = 0;
    wkcStockImageGetSizePeer(WKC_IMAGE_CHECKBOX_UNCHECKED, &w, &dummy);
    if (style.width().isIntrinsicOrAuto())
        style.setWidth(Length((int)w, LengthType::Fixed));
    if (style.height().isAuto())
        style.setHeight(Length((int)w, LengthType::Fixed));
}

void RenderThemeWKC::setRadioSize(RenderStyle& style) const
{
    setCheckboxSize(style);
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

    WKCFloatRect src = { 0, 0, (float)w, (float)h };
    WKCFloatRect dest;
    calcSkinRect(w, h, r, dest);
    _bitblt(ctx, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN,
            (void*)buf, w * 4, nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return false;
}

bool RenderThemeWKC::paintCheckbox(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    int index;
    if (isEnabled(o)) {
        if (isPressed(o))
            index = WKC_IMAGE_CHECKBOX_PRESSED;
        else if (isFocused(o) || isHovered(o))
            index = isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED_FOCUSED : WKC_IMAGE_CHECKBOX_UNCHECKED_FOCUSED;
        else
            index = isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED : WKC_IMAGE_CHECKBOX_UNCHECKED;
    } else {
        index = isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED_DISABLED : WKC_IMAGE_CHECKBOX_UNCHECKED_DISABLED;
    }
    return paintStockImage(o, i, IntRect(r), index);
}

bool RenderThemeWKC::paintRadio(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    int index;
    if (isEnabled(o)) {
        if (isPressed(o))
            index = WKC_IMAGE_RADIO_PRESSED;
        else if (isFocused(o) || isHovered(o))
            index = isChecked(o) ? WKC_IMAGE_RADIO_CHECKED_FOCUSED : WKC_IMAGE_RADIO_UNCHECKED_FOCUSED;
        else
            index = isChecked(o) ? WKC_IMAGE_RADIO_CHECKED : WKC_IMAGE_RADIO_UNCHECKED;
    } else {
        index = isChecked(o) ? WKC_IMAGE_RADIO_CHECKED_DISABLED : WKC_IMAGE_RADIO_UNCHECKED_DISABLED;
    }
    return paintStockImage(o, i, IntRect(r), index);
}

// ─── Button ─────────────────────────────────────────────────────────────────

void RenderThemeWKC::adjustButtonStyle(RenderStyle&, const Element*) const { }

bool RenderThemeWKC::paintButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    void* ctx = i.context().platformContext();
    if (!ctx) return false;

    int index;
    if (isEnabled(o)) {
        index = WKC_IMAGE_BUTTON;
        if (isHovered(o) || isFocused(o)) index = WKC_IMAGE_BUTTON_HOVERED;
        if (isPressed(o))                  index = WKC_IMAGE_BUTTON_PRESSED;
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
    WKCRect rect = { r.x(), r.y(), r.width(), r.height() };
    drawScalingBitmapPeer(ctx, (void*)buf, w * 4, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return false;
}

// ─── TextField / TextArea / SearchField ─────────────────────────────────────

void RenderThemeWKC::adjustTextFieldStyle(RenderStyle& style, const Element*) const
{
    if (!style.hasBackgroundImage())
        return;
    style.resetBorder();
    const unsigned int c = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BORDER);
    const short w = 1;
    const BorderStyle bs = BorderStyle::Solid;
    style.setBorderLeftWidth(w);   style.setBorderLeftStyle(bs);   style.setBorderLeftColor(Color(c));
    style.setBorderRightWidth(w);  style.setBorderRightStyle(bs);  style.setBorderRightColor(Color(c));
    style.setBorderTopWidth(w);    style.setBorderTopStyle(bs);    style.setBorderTopColor(Color(c));
    style.setBorderBottomWidth(w); style.setBorderBottomStyle(bs); style.setBorderBottomColor(Color(c));
}

bool RenderThemeWKC::paintTextField(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    const Color borderColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BORDER));
    i.context().save();
    _setBorder(i.context(), borderColor, 1.0f);

    if (o.style().hasBackgroundImage()) {
        i.context().restore();
        return true;
    }

    Color bg = o.style().hasBackground()
        ? o.style().visitedDependentColor(CSSPropertyBackgroundColor)
        : Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BACKGROUND));

    i.context().setFillColor(bg);
    i.context().drawRect(IntRect(r));
    i.context().restore();
    return false;
}

void RenderThemeWKC::adjustTextAreaStyle(RenderStyle& style, const Element* e) const
{
    adjustTextFieldStyle(style, e);
}

bool RenderThemeWKC::paintTextArea(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    return paintTextField(o, i, r);
}

void RenderThemeWKC::adjustSearchFieldStyle(RenderStyle& style, const Element* e) const
{
    adjustTextFieldStyle(style, e);
}

bool RenderThemeWKC::paintSearchField(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    return paintTextField(o, i, FloatRect(r));
}

void RenderThemeWKC::adjustSearchFieldCancelButtonStyle(RenderStyle& style, const Element*) const
{
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(WKC_IMAGE_SEARCHFIELD_CANCELBUTTON, &w, &h);
    style.setWidth(Length((int)w, LengthType::Fixed));
    style.setHeight(Length((int)h, LengthType::Fixed));
}

bool RenderThemeWKC::paintSearchFieldCancelButton(const RenderBox& o, const PaintInfo& i, const IntRect& r)
{
    return paintStockImage(o, i, r, WKC_IMAGE_SEARCHFIELD_CANCELBUTTON);
}

void RenderThemeWKC::adjustSearchFieldDecorationPartStyle(RenderStyle&, const Element*) const { }
bool RenderThemeWKC::paintSearchFieldDecorationPart(const RenderObject&, const PaintInfo&, const IntRect&) { return false; }

void RenderThemeWKC::adjustSearchFieldResultsDecorationPartStyle(RenderStyle&, const Element*) const { }
bool RenderThemeWKC::paintSearchFieldResultsDecorationPart(const RenderBox&, const PaintInfo&, const IntRect&) { return true; }

void RenderThemeWKC::adjustSearchFieldResultsButtonStyle(RenderStyle& style, const Element*) const
{
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(WKC_IMAGE_SEARCHFIELD_RESULTBUTTON, &w, &h);
    style.setWidth(Length((int)w, LengthType::Fixed));
    style.setHeight(Length((int)h, LengthType::Fixed));
}

bool RenderThemeWKC::paintSearchFieldResultsButton(const RenderBox& o, const PaintInfo& i, const IntRect& r)
{
    return paintStockImage(o, i, r, WKC_IMAGE_SEARCHFIELD_RESULTBUTTON);
}

// ─── MenuList ───────────────────────────────────────────────────────────────

int RenderThemeWKC::minimumMenuListSize(const RenderStyle& style) const
{
    int fs = style.fontSize();
    if (fs >= 13) return 9;
    if (fs >= 11) return 5;
    return 0;
}

void RenderThemeWKC::adjustMenuListStyle(RenderStyle& style, const Element*) const
{
    style.resetBorder();
    style.resetPadding();
    style.setHeight(Length(LengthType::Auto));
    style.setWhiteSpace(WhiteSpace::Pre);
}

bool RenderThemeWKC::paintMenuList(const RenderObject& o, const PaintInfo& i, const FloatRect& r)
{
    paintTextField(o, i, r);
    return paintMenuListButton(o, i, r);
}

void RenderThemeWKC::adjustMenuListButtonStyle(RenderStyle& style, const Element*) const
{
    style.resetPadding();
    style.setLineHeight(RenderStyle::initialLineHeight());
}

bool RenderThemeWKC::paintMenuListButton(const RenderObject& o, const PaintInfo& i, const FloatRect& fr)
{
    const IntRect r(fr);
    void* ctx = i.context().platformContext();
    if (!ctx) return false;

    int index;
    if (isEnabled(o)) {
        if (isPressed(o))                  index = WKC_IMAGE_MENU_LIST_BUTTON_PRESSED;
        else if (isFocused(o) || isHovered(o)) index = WKC_IMAGE_MENU_LIST_BUTTON_FOCUSED;
        else                               index = WKC_IMAGE_MENU_LIST_BUTTON;
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
        src  = { 0, (float)pts[1].fY, (float)w, (float)(pts[2].fY - pts[1].fY) };
        dest = { (float)(r.x() + r.width() - (int)w), (float)(r.y() + (r.height() - src.fHeight) / 2), (float)w, src.fHeight };
        _bitblt(ctx, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN,
                (void*)buf, rowbytes, nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
        return false;
    }

    WKCSize sz = { (int)w, (int)h };
    WKCRect rect = { r.x(), r.y(), r.width(), r.height() };
    drawScalingBitmapPeer(ctx, (void*)buf, rowbytes, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return false;
}

// ─── Slider ─────────────────────────────────────────────────────────────────

void RenderThemeWKC::adjustSliderTrackStyle(RenderStyle& style, const Element* e) const
{
    RenderTheme::adjustSliderTrackStyle(style, e);
}

bool RenderThemeWKC::paintSliderTrack(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    IntRect rect(r);
    const Color borderColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_RANGE_BORDER));

    if (o.style().appearance() == SliderHorizontalPart) {
        rect.setHeight(4);
        rect.move(0, (r.height() - 4) / 2);
    } else if (o.style().appearance() == SliderVerticalPart) {
        rect.setWidth(4);
        rect.move((r.width() - 4) / 2, 0);
    } else {
        return false;
    }

    i.context().save();
    _setBorder(i.context(), borderColor, 1.0f);
    Color bg = isEnabled(o)
        ? o.style().visitedDependentColor(CSSPropertyBackgroundColor)
        : Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_RANGE_BACKGROUND_DISABLED));
    i.context().setFillColor(bg);
    i.context().drawRect(rect);
    i.context().restore();
    return false;
}

void RenderThemeWKC::adjustSliderThumbStyle(RenderStyle& style, const Element* e) const
{
    RenderTheme::adjustSliderThumbStyle(style, e);
    style.setBoxShadow(nullptr);
}

bool RenderThemeWKC::paintSliderThumb(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    int index;
    bool horizontal = (o.style().appearance() == SliderThumbHorizontalPart);
    bool vertical   = (o.style().appearance() == SliderThumbVerticalPart);
    if (!horizontal && !vertical) return false;

    if (isEnabled(o)) {
        index = horizontal ? WKC_IMAGE_H_RANGE : WKC_IMAGE_V_RANGE;
        if (isHovered(o) || isFocused(o))
            index = horizontal ? WKC_IMAGE_H_RANGE_HOVERED : WKC_IMAGE_V_RANGE_HOVERED;
        if (isPressed(o))
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

    WKCFloatRect src  = { 0, 0, (float)w, (float)h };
    WKCFloatRect dest = { (float)r.x(), (float)(r.y() + (r.height() - (int)h) / 2), (float)w, (float)h };
    _bitblt(ctx, WKC_IMAGETYPE_ARGB8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN,
            (void*)buf, w * 4, nullptr, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return false;
}

void RenderThemeWKC::adjustSliderThumbSize(RenderStyle& style, const Element*) const
{
    unsigned int w = 0, h = 0;
    switch (style.appearance()) {
    case SliderThumbHorizontalPart: wkcStockImageGetSizePeer(WKC_IMAGE_H_RANGE, &w, &h); break;
    case SliderThumbVerticalPart:   wkcStockImageGetSizePeer(WKC_IMAGE_V_RANGE, &w, &h); break;
    case MediaSliderThumbPart:      wkcMediaPlayerSkinGetSizePeer(WKC_IMAGE_MEDIA_SLIDER_THUMB, &w, &h); break;
    default: return;
    }
    style.setWidth(Length((int)w, LengthType::Fixed));
    style.setHeight(Length((int)h, LengthType::Fixed));
}

// ─── Colors ─────────────────────────────────────────────────────────────────

Color RenderThemeWKC::platformActiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const
{
    return Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_ACTIVESELECTIONBACKGROUND));
}
Color RenderThemeWKC::platformInactiveSelectionBackgroundColor(OptionSet<StyleColorOptions>) const
{
    return Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_INACTIVESELECTIONBACKGROUND));
}
Color RenderThemeWKC::platformActiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const
{
    return Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_ACTIVESELECTIONFOREGROUND));
}
Color RenderThemeWKC::platformInactiveSelectionForegroundColor(OptionSet<StyleColorOptions>) const
{
    return Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_INACTIVESELECTIONFOREGROUND));
}

// ─── Popup padding ──────────────────────────────────────────────────────────

int RenderThemeWKC::popupInternalPaddingLeft(const RenderStyle&, const Settings&) const { return cPopupInternalPaddingLeft; }
int RenderThemeWKC::popupInternalPaddingTop(const RenderStyle&, const Settings&) const  { return cPopupInternalPaddingTop; }
int RenderThemeWKC::popupInternalPaddingBottom(const RenderStyle&, const Settings&) const { return cPopupInternalPaddingBottom; }
int RenderThemeWKC::popupInternalPaddingRight(const RenderStyle&, const Settings&) const
{
    unsigned int w = 0, h = 0;
    wkcStockImageGetSizePeer(WKC_IMAGE_MENU_LIST_BUTTON, &w, &h);
    return w + cPopupInternalPaddingRight;
}

// ─── Style sheets ───────────────────────────────────────────────────────────

String RenderThemeWKC::extraDefaultStyleSheet()
{
    return String(wkcStockImageGetDefaultStyleSheetPeer());
}
String RenderThemeWKC::extraQuirksStyleSheet()
{
    return String(wkcStockImageGetQuirksStyleSheetPeer());
}

// ─── File list ──────────────────────────────────────────────────────────────

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
    if (gResolveFilenameForDisplayProc) {
        String file = fileList->isEmpty() ? String() : fileList->item(0)->path();
        unsigned short buf[MAX_PATH] = { 0 };
        int len = 0;
        auto chars = file.charactersWithNullTermination();
        (*gResolveFilenameForDisplayProc)(chars.data(), file.length(), buf, &len, MAX_PATH);
        str = String(buf, len);
    } else {
        str = fileList->isEmpty() ? fileButtonNoFileSelectedLabel() : fileList->item(0)->path();
    }
    return StringTruncator::centerTruncate(str, width, font);
}

// ─── Progress bar ───────────────────────────────────────────────────────────

static const double gProgressFrameRate = 0.033;
static const double gProgressDuration  = 2.0;

double RenderThemeWKC::animationRepeatIntervalForProgressBar(const RenderProgress&) const
{
    return gProgressFrameRate;
}
double RenderThemeWKC::animationDurationForProgressBar(const RenderProgress& p) const
{
    return p.isDeterminate() ? gProgressFrameRate : gProgressDuration;
}

bool RenderThemeWKC::paintProgressBar(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    if (!is<RenderProgress>(o)) return true;
    const auto& renderProgress = downcast<RenderProgress>(o);

    const Color borderColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BORDER));
    const Color bodyColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BODY));

    i.context().save();
    _setBorder(i.context(), borderColor, 1.0f);

    Color bg;
    if (!isEnabled(o))
        bg = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BACKGROUND_DISABLED));
    else if (o.style().hasBackground())
        bg = o.style().visitedDependentColor(CSSPropertyBackgroundColor);
    else
        bg = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_PROGRESSBAR_BACKGROUND));

    if (!o.style().hasBackgroundImage()) {
        i.context().setFillColor(bg);
        i.context().drawRect(r);
    }

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
            vr.expand(-vrr.width(), 0);
        }
    }

    _setBorder(i.context(), Color(), 0.0f);
    i.context().setFillColor(bodyColor);
    i.context().fillRect(FloatRect(vr));
    if (!vrr.isEmpty())
        i.context().fillRect(FloatRect(vrr));

    i.context().restore();
    return true;
}

// ─── Meter ──────────────────────────────────────────────────────────────────

bool RenderThemeWKC::paintMeter(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    if (!is<RenderMeter>(o)) return true;
    const auto& renderMeter = downcast<RenderMeter>(o);

    const Color borderColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BORDER));
    const Color bodyColor(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BODY));

    i.context().save();
    _setBorder(i.context(), borderColor, 1.0f);

    Color bg;
    if (!isEnabled(o))
        bg = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BACKGROUND_DISABLED));
    else if (o.style().hasBackground())
        bg = o.style().visitedDependentColor(CSSPropertyBackgroundColor);
    else
        bg = Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_METER_BACKGROUND));

    if (!o.style().hasBackgroundImage()) {
        i.context().setFillColor(bg);
        i.context().drawRect(r);
    }

    double value = 0;
    if (auto* e = dynamicDowncast<HTMLMeterElement>(renderMeter.element()))
        value = e->valueRatio();

    IntRect vr = r;
    vr.expand(-2, -2);
    vr.move(1, 1);
    vr.setWidth((int)(vr.width() * value));

    _setBorder(i.context(), Color(), 0.0f);
    i.context().setFillColor(bodyColor);
    i.context().fillRect(FloatRect(vr));
    i.context().restore();
    return true;
}

// ─── Spin button ────────────────────────────────────────────────────────────

void RenderThemeWKC::adjustInnerSpinButtonStyle(RenderStyle& style, const Element*) const
{
    int w = std::max(style.fontSize(), 18);
    style.setWidth(Length(w, LengthType::Fixed));
    style.setMinWidth(Length(w, LengthType::Fixed));
}

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
    WKCRect rect = { r.x(), r.y(), r.width(), r.height() };
    drawScalingBitmapPeer(ctx, (void*)buf, w * 4, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);

    int cx0 = rect.fX + pts[0].fX;
    int cy0 = rect.fY + pts[0].fY;
    int cw  = rect.fWidth  - pts[0].fX - ((int)sz.fWidth  - pts[3].fX);
    int ch  = rect.fHeight - pts[0].fY - ((int)sz.fHeight - pts[3].fY);
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
    i.context().setFillColor(Color(wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_BUTTONTEXT)));
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

    int idx1, idx2;
    if (isEnabled(o)) {
        idx1 = (isHovered(o) && isSpinUpButtonPartHovered(o)) ? WKC_IMAGE_BUTTON_HOVERED :
               (isPressed(o) && isSpinUpButtonPartPressed(o)) ? WKC_IMAGE_BUTTON_PRESSED  : WKC_IMAGE_BUTTON;
        idx2 = (isHovered(o) && !isSpinUpButtonPartHovered(o)) ? WKC_IMAGE_BUTTON_HOVERED :
               (isPressed(o) && !isSpinUpButtonPartPressed(o)) ? WKC_IMAGE_BUTTON_PRESSED  : WKC_IMAGE_BUTTON;
    } else {
        idx1 = idx2 = WKC_IMAGE_BUTTON_DISABLED;
    }

    _paintSpinButton(o, i, r1, idx1, true);
    _paintSpinButton(o, i, r2, idx2, false);
    return true;
}

bool RenderThemeWKC::supportsFocus(ControlPart part) const
{
    switch (part) {
    case PushButtonPart:
    case ButtonPart:
    case DefaultButtonPart:
    case RadioPart:
    case CheckboxPart:
    case TextFieldPart:
    case SearchFieldPart:
    case TextAreaPart:
        return true;
    default:
        return false;
    }
}

// ─── Video / Media ──────────────────────────────────────────────────────────

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
    if (o.style().hasBorder())
        _setBorder(i.context(), o.style().visitedDependentColor(CSSPropertyBorderLeftColor), 1.0f);
    else
        _setBorder(i.context(), Color::transparentBlack, 0.0f);

    if (!o.style().hasBackgroundImage()) {
        Color bg = o.style().hasBackground()
            ? o.style().visitedDependentColor(CSSPropertyBackgroundColor)
            : Color(color);
        i.context().setFillColor(bg);
        i.context().drawRect(r);
    }
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

    HTMLMediaElement* m = toParentMediaElement(o);
    if (m && !hasSource(m)) {
        state = WKC_IMAGE_MEDIA_STATE_DISABLED;
        if (index == WKC_IMAGE_MEDIA_PAUSE_BUTTON)
            index = WKC_IMAGE_MEDIA_PLAY_BUTTON;
    } else {
        if (isHovered(o) || isFocused(o)) state = WKC_IMAGE_MEDIA_STATE_HOVERED;
        if (isPressed(o))                  state = WKC_IMAGE_MEDIA_STATE_PRESSED;
    }

    const unsigned char* buf = wkcMediaPlayerSkinGetBitmapPeer(index, state);
    if (!buf) return false;
    unsigned int w = 0, h = 0;
    wkcMediaPlayerSkinGetSizePeer(index, &w, &h);
    if (!w || !h) return false;
    const WKCPoint* pts = wkcMediaPlayerSkinGetLayoutPointsPeer(index, state);
    if (!pts) return false;

    WKCSize sz = { (int)w, (int)h };
    WKCRect rect = { r.x(), r.y(), r.width(), r.height() };
    drawScalingBitmapPeer(ctx, (void*)buf, w * 4, &sz, pts, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);
    return true;
}

bool RenderThemeWKC::paintMediaFullscreenButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)
{
    return paintMediaButton(WKC_IMAGE_MEDIA_FULLSCREEN_BUTTON, o, i, r);
}

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

bool RenderThemeWKC::paintMediaSeekBackButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)   { return paintMediaButton(WKC_IMAGE_MEDIA_SEEKBACK_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaSeekForwardButton(const RenderObject& o, const PaintInfo& i, const IntRect& r){ return paintMediaButton(WKC_IMAGE_MEDIA_SEEKFORWARD_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaRewindButton(const RenderObject& o, const PaintInfo& i, const IntRect& r)     { return paintMediaButton(WKC_IMAGE_MEDIA_REWIND_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaReturnToRealtimeButton(const RenderObject& o, const PaintInfo& i, const IntRect& r){ return paintMediaButton(WKC_IMAGE_MEDIA_RETURNTOREALTIME_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaToggleClosedCaptionsButton(const RenderObject& o, const PaintInfo& i, const IntRect& r){ return paintMediaButton(WKC_IMAGE_MEDIA_TOGGLECLOSEDCAPTION_BUTTON, o, i, r); }
bool RenderThemeWKC::paintMediaSliderThumb(const RenderObject& o, const PaintInfo& i, const IntRect& r)      { return paintMediaButton(WKC_IMAGE_MEDIA_SLIDER_THUMB, o, i, r); }

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

    const Color bgColor(0, 0, 0);
    const Color playedColor(0, 196, 222);
    const Color bufferedColor(0, 78, 89);
    int br = r.height() / 2;
    IntSize radii(br, br);

    info.context().save();

    FloatRoundedRect bgRect(r, radii, radii, radii, radii);
    info.context().fillRoundedRect(bgRect, bgColor);

    IntRect playedR(r);
    playedR.setWidth(curPos);
    if (br <= playedR.width()) {
        FloatRoundedRect pr(playedR, radii, IntSize(), radii, IntSize());
        info.context().fillRoundedRect(pr, playedColor);
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
                info.context().fillRoundedRect(bfr, bufferedColor);
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
