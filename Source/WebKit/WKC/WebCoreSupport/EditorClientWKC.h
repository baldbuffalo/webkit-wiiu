/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.
 * All rights reserved.
 * Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
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

#ifndef EditorClientWKC_h
#define EditorClientWKC_h

#include "EditorClient.h"

#include <optional>
#include <wtf/Deque.h>
#include <wtf/Forward.h>

namespace WebCore {
    class Page;
}

namespace WKC {
class EditorClientIf;
class WKCWebViewPrivate;

class EditorClientWKC : public WebCore::EditorClient {
public:
    static EditorClientWKC* create(WKCWebViewPrivate*);
    ~EditorClientWKC();

    // Methods that forward to the WKC embedder (EditorClientIf). Defined in the .cpp.
    bool shouldDeleteRange(const std::optional<WebCore::SimpleRange>&) override;
    bool smartInsertDeleteEnabled() override;
    bool isSelectTrailingWhitespaceEnabled() const override;
    bool isContinuousSpellCheckingEnabled() override;
    void toggleContinuousSpellChecking() override;
    bool isGrammarCheckingEnabled() override;
    void toggleGrammarChecking() override;
    int spellCheckerDocumentTag() override;

    bool shouldBeginEditing(const WebCore::SimpleRange&) override;
    bool shouldEndEditing(const WebCore::SimpleRange&) override;
    bool shouldInsertNode(WebCore::Node&, const std::optional<WebCore::SimpleRange>&, WebCore::EditorInsertAction) override;
    bool shouldInsertText(const WTF::String&, const std::optional<WebCore::SimpleRange>&, WebCore::EditorInsertAction) override;
    bool shouldChangeSelectedRange(const std::optional<WebCore::SimpleRange>& fromRange, const std::optional<WebCore::SimpleRange>& toRange, WebCore::Affinity, bool stillSelecting) override;

    bool shouldMoveRangeAfterDelete(const WebCore::SimpleRange&, const WebCore::SimpleRange&) override;

    void didBeginEditing() override;
    void respondToChangedContents() override;
    void respondToChangedSelection(WebCore::LocalFrame*) override;
    void didEndEditing() override;
    void didWriteSelectionToPasteboard() override;

    void clearUndoRedoOperations() override;

    bool canCopyCut(WebCore::LocalFrame*, bool defaultValue) const override;
    bool canPaste(WebCore::LocalFrame*, bool defaultValue) const override;
    bool canUndo() const override;
    bool canRedo() const override;

    void undo() override;
    void redo() override;

    void handleKeyboardEvent(WebCore::KeyboardEvent&) override;
    void handleInputMethodKeydown(WebCore::KeyboardEvent&) override;

    void textFieldDidBeginEditing(WebCore::Element&) override;
    void textFieldDidEndEditing(WebCore::Element&) override;
    void textDidChangeInTextField(WebCore::Element&) override;
    bool doTextFieldCommandFromEvent(WebCore::Element&, WebCore::KeyboardEvent*) override;
    void textWillBeDeletedInTextField(WebCore::Element&) override;
    void textDidChangeInTextArea(WebCore::Element&) override;

    WebCore::TextCheckerClient* textChecker() override;

    void updateSpellingUIWithMisspelledWord(const WTF::String&) override;
    void showSpellingUI(bool show) override;
    bool spellingUIIsShowing() override;
    void setInputMethodState(WebCore::Element*) override;

    // Callbacks with no WKC embedder counterpart (new in modern WebKit, or the
    // WKC port never implemented them). Correct as no-ops / defaults.
    bool shouldApplyStyle(const WebCore::StyleProperties&, const std::optional<WebCore::SimpleRange>&) override { return false; }
    void didApplyStyle() override { }
    void updateEditorStateAfterLayoutIfEditabilityChanged() override { }
    void discardedComposition(const WebCore::Document&) override { }
    void canceledComposition() override { }
    void didUpdateComposition() override { }
    void didEndUserTriggeredSelectionChanges() override { }
    void willWriteSelectionToPasteboard(const std::optional<WebCore::SimpleRange>&) override { }
    void getClientPasteboardData(const std::optional<WebCore::SimpleRange>&, Vector<std::pair<WTF::String, RefPtr<WebCore::SharedBuffer>>>&) override { }
    void requestCandidatesForSelection(const WebCore::VisibleSelection&) override { }
    void handleAcceptedCandidateWithSoftSpaces(WebCore::TextCheckingResult) override { }
    void registerUndoStep(WebCore::UndoStep&) override { }
    void registerRedoStep(WebCore::UndoStep&) override { }
    WebCore::DOMPasteAccessResponse requestDOMPasteAccess(WebCore::DOMPasteAccessCategory, WebCore::FrameIdentifier, const WTF::String&) override { return WebCore::DOMPasteAccessResponse::DeniedForGesture; }
    void overflowScrollPositionChanged() override { }
    void subFrameScrollPositionChanged() override { }
    bool performTwoStepDrop(WebCore::DocumentFragment&, const WebCore::SimpleRange&, bool) override { return false; }
    void updateSpellingUIWithGrammarString(const WTF::String&, const WebCore::GrammarDetail&) override { }

private:
   EditorClientWKC(WKCWebViewPrivate*);
   bool construct();

private:
   WKCWebViewPrivate* m_view;
   WKC::EditorClientIf* m_appClient;
};

} // namespace

#endif // EditorClientWKC_h
