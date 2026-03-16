#include "config.h"
#include "PlatformKeyboardEvent.h"
#include "WindowsKeyboardCodes.h"
#include "NotImplemented.h"
#include <wkc/wkcbase.h>
#include "WKCPlatformEvents.h"
#include <wtf/HexNumber.h>
#include <wtf/text/StringBuilder.h>

namespace WebCore {

static OptionSet<PlatformEvent::Modifier> gCurrentModifiers;

static String keyForWKCKeyCode(int keyCode)
{
    switch (keyCode) {
    case WKC::EKeyBack:       return "Backspace"_s;
    case WKC::EKeyTab:        return "Tab"_s;
    case WKC::EKeyClear:      return "Clear"_s;
    case WKC::EKeyReturn:     return "Enter"_s;
    case WKC::EKeyMenu:       return "Alt"_s;
    case WKC::EKeyPause:      return "Pause"_s;
    case WKC::EKeyPrior:      return "PageUp"_s;
    case WKC::EKeyNext:       return "PageDown"_s;
    case WKC::EKeyEnd:        return "End"_s;
    case WKC::EKeyHome:       return "Home"_s;
    case WKC::EKeyLeft:       return "ArrowLeft"_s;
    case WKC::EKeyUp:         return "ArrowUp"_s;
    case WKC::EKeyRight:      return "ArrowRight"_s;
    case WKC::EKeyDown:       return "ArrowDown"_s;
    case WKC::EKeySelect:     return "Select"_s;
    case WKC::EKeyExecute:    return "Execute"_s;
    case WKC::EKeySnapShot:   return "PrintScreen"_s;
    case WKC::EKeyInsert:     return "Insert"_s;
    case WKC::EKeyDelete:     return "Delete"_s;
    case WKC::EKeyHelp:       return "Help"_s;
    case WKC::EKeyEscape:     return "Escape"_s;
    case WKC::EKeySpace:      return " "_s;
    case WKC::EKeyF1:         return "F1"_s;
    case WKC::EKeyF2:         return "F2"_s;
    case WKC::EKeyF3:         return "F3"_s;
    case WKC::EKeyF4:         return "F4"_s;
    case WKC::EKeyF5:         return "F5"_s;
    case WKC::EKeyF6:         return "F6"_s;
    case WKC::EKeyF7:         return "F7"_s;
    case WKC::EKeyF8:         return "F8"_s;
    case WKC::EKeyF9:         return "F9"_s;
    case WKC::EKeyF10:        return "F10"_s;
    case WKC::EKeyF11:        return "F11"_s;
    case WKC::EKeyF12:        return "F12"_s;
    case WKC::EKeyF13:        return "F13"_s;
    case WKC::EKeyF14:        return "F14"_s;
    case WKC::EKeyF15:        return "F15"_s;
    case WKC::EKeyF16:        return "F16"_s;
    case WKC::EKeyF17:        return "F17"_s;
    case WKC::EKeyF18:        return "F18"_s;
    case WKC::EKeyF19:        return "F19"_s;
    case WKC::EKeyF20:        return "F20"_s;
    case WKC::EKeyF21:        return "F21"_s;
    case WKC::EKeyF22:        return "F22"_s;
    case WKC::EKeyF23:        return "F23"_s;
    case WKC::EKeyF24:        return "F24"_s;
    default:
        break;
    }
    StringBuilder sb;
    sb.append("U+"_s);
    sb.append(hex(toASCIIUpper(keyCode), 4));
    return sb.toString();
}

static int windowsKeyCodeForKeyEvent(int keyCode)
{
    switch (keyCode) {
    case WKC::EKeyBack:                   return VK_BACK;
    case WKC::EKeyTab:                    return VK_TAB;
    case WKC::EKeyClear:                  return VK_CLEAR;
    case WKC::EKeyReturn:                 return VK_RETURN;
    case WKC::EKeyShift:                  return VK_SHIFT;
    case WKC::EKeyControl:                return VK_CONTROL;
    case WKC::EKeyMenu:                   return VK_MENU;
    case WKC::EKeyPause:                  return VK_PAUSE;
    case WKC::EKeyCapital:                return VK_CAPITAL;
    case WKC::EKeyKana:                   return VK_KANA;
    case WKC::EKeyJunja:                  return VK_JUNJA;
    case WKC::EKeyFinal:                  return VK_FINAL;
    case WKC::EKeyHanja:                  return VK_HANJA;
    case WKC::EKeyEscape:                 return VK_ESCAPE;
    case WKC::EKeyConvert:                return VK_CONVERT;
    case WKC::EKeyNonConvert:             return VK_NONCONVERT;
    case WKC::EKeyAccept:                 return VK_ACCEPT;
    case WKC::EKeyModeChange:             return VK_MODECHANGE;
    case WKC::EKeySpace:                  return VK_SPACE;
    case WKC::EKeyPrior:                  return VK_PRIOR;
    case WKC::EKeyNext:                   return VK_NEXT;
    case WKC::EKeyEnd:                    return VK_END;
    case WKC::EKeyHome:                   return VK_HOME;
    case WKC::EKeyLeft:                   return VK_LEFT;
    case WKC::EKeyUp:                     return VK_UP;
    case WKC::EKeyRight:                  return VK_RIGHT;
    case WKC::EKeyDown:                   return VK_DOWN;
    case WKC::EKeySelect:                 return VK_SELECT;
    case WKC::EKeyPrint:                  return VK_PRINT;
    case WKC::EKeyExecute:                return VK_EXECUTE;
    case WKC::EKeySnapShot:               return VK_SNAPSHOT;
    case WKC::EKeyInsert:                 return VK_INSERT;
    case WKC::EKeyDelete:                 return VK_DELETE;
    case WKC::EKeyHelp:                   return VK_HELP;
    case WKC::EKey0:                      return VK_0;
    case WKC::EKey1:                      return VK_1;
    case WKC::EKey2:                      return VK_2;
    case WKC::EKey3:                      return VK_3;
    case WKC::EKey4:                      return VK_4;
    case WKC::EKey5:                      return VK_5;
    case WKC::EKey6:                      return VK_6;
    case WKC::EKey7:                      return VK_7;
    case WKC::EKey8:                      return VK_8;
    case WKC::EKey9:                      return VK_9;
    case WKC::EKeyA:                      return VK_A;
    case WKC::EKeyB:                      return VK_B;
    case WKC::EKeyC:                      return VK_C;
    case WKC::EKeyD:                      return VK_D;
    case WKC::EKeyE:                      return VK_E;
    case WKC::EKeyF:                      return VK_F;
    case WKC::EKeyG:                      return VK_G;
    case WKC::EKeyH:                      return VK_H;
    case WKC::EKeyI:                      return VK_I;
    case WKC::EKeyJ:                      return VK_J;
    case WKC::EKeyK:                      return VK_K;
    case WKC::EKeyL:                      return VK_L;
    case WKC::EKeyM:                      return VK_M;
    case WKC::EKeyN:                      return VK_N;
    case WKC::EKeyO:                      return VK_O;
    case WKC::EKeyP:                      return VK_P;
    case WKC::EKeyQ:                      return VK_Q;
    case WKC::EKeyR:                      return VK_R;
    case WKC::EKeyS:                      return VK_S;
    case WKC::EKeyT:                      return VK_T;
    case WKC::EKeyU:                      return VK_U;
    case WKC::EKeyV:                      return VK_V;
    case WKC::EKeyW:                      return VK_W;
    case WKC::EKeyX:                      return VK_X;
    case WKC::EKeyY:                      return VK_Y;
    case WKC::EKeyZ:                      return VK_Z;
    case WKC::EKeyLWin:                   return VK_LWIN;
    case WKC::EKeyRWin:                   return VK_RWIN;
    case WKC::EKeyApps:                   return VK_APPS;
    case WKC::EKeySleep:                  return VK_SLEEP;
    case WKC::EKeyNumPad0:                return VK_NUMPAD0;
    case WKC::EKeyNumPad1:                return VK_NUMPAD1;
    case WKC::EKeyNumPad2:                return VK_NUMPAD2;
    case WKC::EKeyNumPad3:                return VK_NUMPAD3;
    case WKC::EKeyNumPad4:                return VK_NUMPAD4;
    case WKC::EKeyNumPad5:                return VK_NUMPAD5;
    case WKC::EKeyNumPad6:                return VK_NUMPAD6;
    case WKC::EKeyNumPad7:                return VK_NUMPAD7;
    case WKC::EKeyNumPad8:                return VK_NUMPAD8;
    case WKC::EKeyNumPad9:                return VK_NUMPAD9;
    case WKC::EKeyMultiply:               return VK_MULTIPLY;
    case WKC::EKeyAdd:                    return VK_ADD;
    case WKC::EKeySeparator:              return VK_SEPARATOR;
    case WKC::EKeySubtract:               return VK_SUBTRACT;
    case WKC::EKeyDecimal:                return VK_DECIMAL;
    case WKC::EKeyDivide:                 return VK_DIVIDE;
    case WKC::EKeyF1:                     return VK_F1;
    case WKC::EKeyF2:                     return VK_F2;
    case WKC::EKeyF3:                     return VK_F3;
    case WKC::EKeyF4:                     return VK_F4;
    case WKC::EKeyF5:                     return VK_F5;
    case WKC::EKeyF6:                     return VK_F6;
    case WKC::EKeyF7:                     return VK_F7;
    case WKC::EKeyF8:                     return VK_F8;
    case WKC::EKeyF9:                     return VK_F9;
    case WKC::EKeyF10:                    return VK_F10;
    case WKC::EKeyF11:                    return VK_F11;
    case WKC::EKeyF12:                    return VK_F12;
    case WKC::EKeyF13:                    return VK_F13;
    case WKC::EKeyF14:                    return VK_F14;
    case WKC::EKeyF15:                    return VK_F15;
    case WKC::EKeyF16:                    return VK_F16;
    case WKC::EKeyF17:                    return VK_F17;
    case WKC::EKeyF18:                    return VK_F18;
    case WKC::EKeyF19:                    return VK_F19;
    case WKC::EKeyF20:                    return VK_F20;
    case WKC::EKeyF21:                    return VK_F21;
    case WKC::EKeyF22:                    return VK_F22;
    case WKC::EKeyF23:                    return VK_F23;
    case WKC::EKeyF24:                    return VK_F24;
    case WKC::EKeyNumLock:                return VK_NUMLOCK;
    case WKC::EKeyScroll:                 return VK_SCROLL;
    case WKC::EKeyLShift:                 return VK_LSHIFT;
    case WKC::EKeyRShift:                 return VK_RSHIFT;
    case WKC::EKeyLControl:               return VK_LCONTROL;
    case WKC::EKeyRControl:               return VK_RCONTROL;
    case WKC::EKeyLMenu:                  return VK_LMENU;
    case WKC::EKeyRMenu:                  return VK_RMENU;
    case WKC::EKeyBrowserBack:            return VK_BROWSER_BACK;
    case WKC::EKeyBrowserForward:         return VK_BROWSER_FORWARD;
    case WKC::EKeyBrowserRefresh:         return VK_BROWSER_REFRESH;
    case WKC::EKeyBrowserStop:            return VK_BROWSER_STOP;
    case WKC::EKeyBrowserSearch:          return VK_BROWSER_SEARCH;
    case WKC::EKeyBrowserFavorites:       return VK_BROWSER_FAVORITES;
    case WKC::EKeyBrowserHome:            return VK_BROWSER_HOME;
    case WKC::EKeyVolumeMute:             return VK_VOLUME_MUTE;
    case WKC::EKeyVolumeDown:             return VK_VOLUME_DOWN;
    case WKC::EKeyVolumeUp:               return VK_VOLUME_UP;
    case WKC::EKeyMediaNextTrack:         return VK_MEDIA_NEXT_TRACK;
    case WKC::EKeyMediaPrevTrack:         return VK_MEDIA_PREV_TRACK;
    case WKC::EKeyMediaStop:              return VK_MEDIA_STOP;
    case WKC::EKeyMediaPlayPause:         return VK_MEDIA_PLAY_PAUSE;
    case WKC::EKeyMediaLaunchMail:        return VK_MEDIA_LAUNCH_MAIL;
    case WKC::EKeyMediaLaunchMediaselect: return VK_MEDIA_LAUNCH_MEDIA_SELECT;
    case WKC::EKeyMediaLaunchApp1:        return VK_MEDIA_LAUNCH_APP1;
    case WKC::EKeyMediaLaunchApp2:        return VK_MEDIA_LAUNCH_APP2;
    case WKC::EKeyOem1:                   return VK_OEM_1;
    case WKC::EKeyOemPlus:                return VK_OEM_PLUS;
    case WKC::EKeyOemComma:               return VK_OEM_COMMA;
    case WKC::EKeyOemMinus:               return VK_OEM_MINUS;
    case WKC::EKeyOemPeriod:              return VK_OEM_PERIOD;
    case WKC::EKeyOem2:                   return VK_OEM_2;
    case WKC::EKeyOem3:                   return VK_OEM_3;
    case WKC::EKeyOem4:                   return VK_OEM_4;
    case WKC::EKeyOem5:                   return VK_OEM_5;
    case WKC::EKeyOem6:                   return VK_OEM_6;
    case WKC::EKeyOem7:                   return VK_OEM_7;
    case WKC::EKeyOem8:                   return VK_OEM_8;
    case WKC::EKeyOem102:                 return VK_OEM_102;
    case WKC::EKeyProcessKey:             return VK_PROCESSKEY;
    case WKC::EKeyPacket:                 return VK_PACKET;
    case WKC::EKeyAttn:                   return VK_ATTN;
    case WKC::EKeyCrSel:                  return VK_CRSEL;
    case WKC::EKeyExSel:                  return VK_EXSEL;
    case WKC::EKeyErEOF:                  return VK_EREOF;
    case WKC::EKeyPlay:                   return VK_PLAY;
    case WKC::EKeyZoom:                   return VK_ZOOM;
    case WKC::EKeyNoName:                 return VK_NONAME;
    case WKC::EKeyPA1:                    return VK_PA1;
    case WKC::EKeyOemClear:               return VK_OEM_CLEAR;
    default:
        break;
    }
    return 0;
}

static String singleCharacterString(unsigned int keyChar)
{
    char16_t buf[1] = { static_cast<char16_t>(keyChar) };
    return String(std::span<const char16_t>(buf, 1));
}

PlatformKeyboardEvent wkcCreateKeyboardEvent(void* event)
{
    WKC::WKCKeyEvent* ev = static_cast<WKC::WKCKeyEvent*>(event);

    PlatformEvent::Type type = PlatformEvent::Type::KeyUp;
    switch (ev->m_type) {
    case WKC::EKeyEventPressed:
        type = PlatformEvent::Type::RawKeyDown;
        break;
    case WKC::EKeyEventReleased:
        type = PlatformEvent::Type::KeyUp;
        break;
    case WKC::EKeyEventChar:
        type = PlatformEvent::Type::Char;
        break;
    case WKC::EKeyEventAccessKey:
        type = PlatformEvent::Type::KeyDown;
        break;
    default:
        type = PlatformEvent::Type::KeyUp;
    }

    OptionSet<PlatformEvent::Modifier> modifiers;
    if (ev->m_modifiers & WKC::EModifierShift) {
        modifiers.add(PlatformEvent::Modifier::ShiftKey);
        if (type == PlatformEvent::Type::KeyUp)
            gCurrentModifiers.remove(PlatformEvent::Modifier::ShiftKey);
        else
            gCurrentModifiers.add(PlatformEvent::Modifier::ShiftKey);
    }
    if (ev->m_modifiers & WKC::EModifierCtrl) {
        modifiers.add(PlatformEvent::Modifier::ControlKey);
        if (type == PlatformEvent::Type::KeyUp)
            gCurrentModifiers.remove(PlatformEvent::Modifier::ControlKey);
        else
            gCurrentModifiers.add(PlatformEvent::Modifier::ControlKey);
    }
    if (ev->m_modifiers & WKC::EModifierAlt) {
        modifiers.add(PlatformEvent::Modifier::AltKey);
        if (type == PlatformEvent::Type::KeyUp)
            gCurrentModifiers.remove(PlatformEvent::Modifier::AltKey);
        else
            gCurrentModifiers.add(PlatformEvent::Modifier::AltKey);
    }
    if (ev->m_modifiers & WKC::EModifierMod1) {
        modifiers.add(PlatformEvent::Modifier::MetaKey);
        if (type == PlatformEvent::Type::KeyUp)
            gCurrentModifiers.remove(PlatformEvent::Modifier::MetaKey);
        else
            gCurrentModifiers.add(PlatformEvent::Modifier::MetaKey);
    }

    String text;
    String unmodifiedText;
    String key;
    String code;
    int windowsVirtualKeyCode = 0;
    bool isKeypad = false;

    if (type == PlatformEvent::Type::Char) {
        text = singleCharacterString(ev->m_char);
        unmodifiedText = text;
    } else {
        key = keyForWKCKeyCode(ev->m_key);
        windowsVirtualKeyCode = windowsKeyCodeForKeyEvent(ev->m_key);
        if (ev->m_type == WKC::EKeyEventAccessKey) {
            text = singleCharacterString(ev->m_char);
            unmodifiedText = text;
        }
        switch (ev->m_key) {
        case WKC::EKeyNumPad0: case WKC::EKeyNumPad1: case WKC::EKeyNumPad2:
        case WKC::EKeyNumPad3: case WKC::EKeyNumPad4: case WKC::EKeyNumPad5:
        case WKC::EKeyNumPad6: case WKC::EKeyNumPad7: case WKC::EKeyNumPad8:
        case WKC::EKeyNumPad9: case WKC::EKeyMultiply: case WKC::EKeyAdd:
        case WKC::EKeySeparator: case WKC::EKeySubtract: case WKC::EKeyDecimal:
        case WKC::EKeyDivide:
            isKeypad = true;
            break;
        default:
            break;
        }
    }

    return PlatformKeyboardEvent(type, text, unmodifiedText, key, code,
        String(), windowsVirtualKeyCode, ev->m_autoRepeat, isKeypad, false,
        modifiers, MonotonicTime::now());
}

void PlatformKeyboardEvent::disambiguateKeyDownEvent(Type type, bool backwardCompatibilityMode)
{
    ASSERT(m_type == PlatformEvent::Type::KeyDown);
    m_type = type;
    if (type == PlatformEvent::Type::RawKeyDown) {
        m_text = String();
        m_unmodifiedText = String();
    } else {
        m_key = String();
        m_windowsVirtualKeyCode = 0;
    }
}

bool PlatformKeyboardEvent::currentCapsLockState()
{
    return false;
}

void PlatformKeyboardEvent::getCurrentModifierState(bool& shiftKey, bool& ctrlKey, bool& altKey, bool& metaKey)
{
    shiftKey = gCurrentModifiers.contains(PlatformEvent::Modifier::ShiftKey);
    ctrlKey  = gCurrentModifiers.contains(PlatformEvent::Modifier::ControlKey);
    altKey   = gCurrentModifiers.contains(PlatformEvent::Modifier::AltKey);
    metaKey  = gCurrentModifiers.contains(PlatformEvent::Modifier::MetaKey);
}

} // namespace WebCore
