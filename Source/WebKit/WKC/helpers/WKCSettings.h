/*
 * Copyright (C) 2003, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *           (C) 2006 Graham Dennis (graham.dennis@gmail.com)
 * Copyright (c) 2010-2014 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef WKCSetting_h
#define WKCSetting_h

#include <stddef.h>

namespace WKC {

class WKCWebViewPrivate;
class WKCSettingsPrivate;

class WKC_API WKCSettings {
public:
    // same as WebCore::Settings
#ifdef WKC_USE_ANDROID_LAYOUT
    enum LayoutAlgorithm {
        kLayoutNormal,
        kLayoutFitColumnToScreen
    };
#endif
    enum EditableLinkBehavior {
        EditableLinkDefaultBehavior,
        EditableLinkAlwaysLive,
        EditableLinkOnlyLiveWithShiftKey,
        EditableLinkLiveWhenNotFocused,
        EditableLinkNeverLive
    };
    enum TextDirectionSubmenuInclusionBehavior {
        TextDirectionSubmenuNeverIncluded,
        TextDirectionSubmenuAutomaticallyIncluded,
        TextDirectionSubmenuAlwaysIncluded
    };
    enum EditingBehaviorType { EditingMacBehavior, EditingWindowsBehavior, EditingUnixBehavior };

    // from FontRenderingMode.h
    enum FontRenderingMode { NormalRenderingMode, AlternateRenderingMode };

    void setStandardFontFamily(const char*);
    const char* standardFontFamily();

    void setFixedFontFamily(const char*);
    const char* fixedFontFamily() const;

    void setSerifFontFamily(const char*);
    const char* serifFontFamily();

    void setSansSerifFontFamily(const char*);
    const char* sansSerifFontFamily();

    void setCursiveFontFamily(const char*);
    const char* cursiveFontFamily() const;

    void setFantasyFontFamily(const char*);
    const char* fantasyFontFamily() const;

    void setMinimumFontSize(int);
    int minimumFontSize() const;

    void setMinimumLogicalFontSize(int);
    int minimumLogicalFontSize() const;

    void setDefaultFontSize(int);
    int defaultFontSize() const;

    void setDefaultFixedFontSize(int);
    int defaultFixedFontSize() const;

    void setLoadsImagesAutomatically(bool);
    bool loadsImagesAutomatically() const;

    void setScriptEnabled(bool);
    bool isScriptEnabled() const;

    void setWebSecurityEnabled(bool);
    bool isWebSecurityEnabled() const;

    void setAllowUniversalAccessFromFileURLs(bool);
    bool allowUniversalAccessFromFileURLs() const;

    void setJavaScriptCanOpenWindowsAutomatically(bool);
    bool javaScriptCanOpenWindowsAutomatically() const;

#if 1
    // added at webkit.org trunk r58703
    void setJavaScriptCanAccessClipboard(bool);
    bool javaScriptCanAccessClipboard() const;
#endif

    void setSpatialNavigationEnabled(bool);
    bool isSpatialNavigationEnabled() const;

    void setLocalStorageEnabled(bool);
    bool localStorageEnabled() const;

    void setSessionStorageQuota(unsigned);
    unsigned sessionStorageQuota() const;

    void setCaretBrowsingEnabled(bool);
    bool caretBrowsingEnabled() const;

    void setDefaultTextEncodingName(const char*);
    const char* defaultTextEncodingName();
        
    void setUsesEncodingDetector(bool);
    bool usesEncodingDetector() const;

    void setUserStyleSheetLocation(const char*);
    const char* userStyleSheetLocation() const;

    void setShouldPrintBackgrounds(bool);
    bool shouldPrintBackgrounds() const;

    void setTextAreasAreResizable(bool);
    bool textAreasAreResizable() const;

    void setEditableLinkBehavior(EditableLinkBehavior);
    EditableLinkBehavior editableLinkBehavior();

    void setTextDirectionSubmenuInclusionBehavior(TextDirectionSubmenuInclusionBehavior);
    TextDirectionSubmenuInclusionBehavior textDirectionSubmenuInclusionBehavior() const;
        
    void setNeedsAdobeFrameReloadingQuirk(bool);
    bool needsAcrobatFrameReloadingQuirk() const;

    void setNeedsKeyboardEventDisambiguationQuirks(bool);
    bool needsKeyboardEventDisambiguationQuirks() const;

    void setTreatsAnyTextCSSLinkAsStylesheet(bool);
    bool treatsAnyTextCSSLinkAsStylesheet() const;


    void setDOMPasteAllowed(bool);
    bool isDOMPasteAllowed() const;

    void setUsesPageCache(bool);
    bool usesPageCache() const;

    void setShrinksStandaloneImagesToFit(bool);
    bool shrinksStandaloneImagesToFit() const;

    void setShowsURLsInToolTips(bool);
    bool showsURLsInToolTips() const;

        
        
    void setDeveloperExtrasEnabled(bool);
    bool developerExtrasEnabled() const;


    void setAuthorAndUserStylesEnabled(bool);
    bool authorAndUserStylesEnabled() const;
        

    void setNeedsSiteSpecificQuirks(bool);
    bool needsSiteSpecificQuirks() const;
        
    void setWebArchiveDebugModeEnabled(bool);
    bool webArchiveDebugModeEnabled() const;

    void setLocalFileContentSniffingEnabled(bool);
    bool localFileContentSniffingEnabled() const;

    void setLocalStorageDatabasePath(const char*);
    const char* localStorageDatabasePath() const;
        






    void setEditingBehaviorType(EditingBehaviorType behavior);
    EditingBehaviorType editingBehaviorType() const;
        


    void setCanvasUsesAcceleratedDrawing(bool);
    bool canvasUsesAcceleratedDrawing() const;

    void setAcceleratedDrawingEnabled(bool);
    bool acceleratedDrawingEnabled() const;

    void setAcceleratedCompositingEnabled(bool);
    bool acceleratedCompositingEnabled() const;






    void setAcceleratedCompositingForFixedPositionEnabled(bool);
    bool acceleratedCompositingForFixedPositionEnabled() const;


    void setMockScrollbarsEnabled(bool flag);
    bool mockScrollbarsEnabled() const;

    void setShowDebugBorders(bool);
    bool showDebugBorders() const;

    void setShowRepaintCounter(bool);
    bool showRepaintCounter() const;


    void setWebAudioEnabled(bool);
    bool webAudioEnabled() const;

    void setWebGLEnabled(bool);
    bool webGLEnabled() const;




    void setFullScreenEnabled(bool);
    bool fullScreenEnabled() const;

    void setForceCompositingMode(bool);
    bool forceCompositingMode();


    void setShouldDisplaySubtitles(bool);
    bool shouldDisplaySubtitles() const;

    void setShouldDisplayCaptions(bool);
    bool shouldDisplayCaptions() const;

    void setShouldDisplayTextDescriptions(bool);
    bool shouldDisplayTextDescriptions() const;

#ifdef WKC_USE_ANDROID_LAYOUT
#endif
    void setInteractiveFormValidationEnabled(bool);
    bool interactiveFormValidationEnabled() const;

private:
    friend class WKCWebViewPrivate;
    WKCSettings(WKC::WKCWebViewPrivate*);
    ~WKCSettings();

    WKCSettingsPrivate* m_private;
};

class WKC_API WKCGlobalSettings {
private:
    WKCGlobalSettings();
    ~WKCGlobalSettings();
    static WKCGlobalSettings* create();
    bool construct();

public:
    static bool createSharedInstance(bool autoflag = false);
    static void deleteSharedInstance();
    static bool isExistSharedInstance();
    static bool isAutomatic();
};

}   // namespace
#endif  /* WKCSetting_h */
