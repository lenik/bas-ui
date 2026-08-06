#include "DefaultAutomation.hpp"

#include "AutomationError.hpp"
#include "EventRegistry.hpp"
#include "WidgetTraits.hpp"
#include "json_util.hpp"

#include "../arch/UIAction.hpp"
#include "../arch/UIElement.hpp"
#include "../arch/UIGroup.hpp"
#include "../arch/UIState.hpp"

#include <bas/wx/wx_compat.hpp>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/event.h>
#include <wx/listbox.h>
#include <wx/radiobut.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>

#include <algorithm>
#include <cctype>
#include <cmath>

namespace bas::ui::automation {

namespace {

wxString utf8(const std::string& s) { return wxString::FromUTF8(s.c_str()); }

std::string lowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

int mouseButtonFromName(const std::string& name) {
    if (name == "right" || name == "rightclick" || name == "contextmenu") {
        return wxMOUSE_BTN_RIGHT;
    }
    if (name == "middle" || name == "middleclick") {
        return wxMOUSE_BTN_MIDDLE;
    }
#if defined(wxMOUSE_BTN_AUX1)
    if (name == "aux1") {
        return wxMOUSE_BTN_AUX1;
    }
#endif
#if defined(wxMOUSE_BTN_AUX2)
    if (name == "aux2") {
        return wxMOUSE_BTN_AUX2;
    }
#endif
    return wxMOUSE_BTN_LEFT;
}

wxEventType mouseDownType(int button) {
    switch (button) {
    case wxMOUSE_BTN_RIGHT:
        return wxEVT_RIGHT_DOWN;
    case wxMOUSE_BTN_MIDDLE:
        return wxEVT_MIDDLE_DOWN;
#if defined(wxMOUSE_BTN_AUX1)
    case wxMOUSE_BTN_AUX1:
        return wxEVT_AUX1_DOWN;
#endif
#if defined(wxMOUSE_BTN_AUX2)
    case wxMOUSE_BTN_AUX2:
        return wxEVT_AUX2_DOWN;
#endif
    default:
        return wxEVT_LEFT_DOWN;
    }
}

wxEventType mouseUpType(int button) {
    switch (button) {
    case wxMOUSE_BTN_RIGHT:
        return wxEVT_RIGHT_UP;
    case wxMOUSE_BTN_MIDDLE:
        return wxEVT_MIDDLE_UP;
#if defined(wxMOUSE_BTN_AUX1)
    case wxMOUSE_BTN_AUX1:
        return wxEVT_AUX1_UP;
#endif
#if defined(wxMOUSE_BTN_AUX2)
    case wxMOUSE_BTN_AUX2:
        return wxEVT_AUX2_UP;
#endif
    default:
        return wxEVT_LEFT_UP;
    }
}

wxEventType mouseDClickType(int button) {
    switch (button) {
    case wxMOUSE_BTN_RIGHT:
        return wxEVT_RIGHT_DCLICK;
    case wxMOUSE_BTN_MIDDLE:
        return wxEVT_MIDDLE_DCLICK;
#if defined(wxMOUSE_BTN_AUX1)
    case wxMOUSE_BTN_AUX1:
        return wxEVT_AUX1_DCLICK;
#endif
#if defined(wxMOUSE_BTN_AUX2)
    case wxMOUSE_BTN_AUX2:
        return wxEVT_AUX2_DCLICK;
#endif
    default:
        return wxEVT_LEFT_DCLICK;
    }
}

} // namespace

DefaultAutomation::DefaultAutomation(wxWindow* root, bool nothrow)
    : m_root(root), m_nothrow(nothrow) {}

DefaultAutomation::DefaultAutomation(wxWindow* root, AutomationMap map, bool nothrow)
    : m_root(root), m_map(std::move(map)), m_nothrow(nothrow) {}

void DefaultAutomation::bindArch(UIElement* element) { m_map.bindTree(element); }

void DefaultAutomation::bindArch(UIGroup* group) { m_map.bindTree(group); }

std::string DefaultAutomation::normalizeEventName(std::string name) { return lowerCopy(std::move(name)); }

bool DefaultAutomation::emulate(const std::string& eventName, const std::string& objId,
                                const boost::json::object& data) {
    if (m_nothrow) {
        return tryEmulate(eventName, objId, data);
    }
    composeEvent(eventName, objId, data);
    return true;
}

bool DefaultAutomation::tryEmulate(const std::string& eventName, const std::string& objId,
                                   const boost::json::object& data) {
    try {
        composeEvent(eventName, objId, data);
        return true;
    } catch (const AutomationError&) {
        return false;
    }
}

wxWindow* DefaultAutomation::requireTarget(const std::string& eventName,
                                           const std::string& objId) const {
    if (!m_root) {
        throw AutomationError(eventName, objId, "automation root is null");
    }
    wxWindow* target = m_map.resolve(m_root, objId);
    if (!target) {
        throw AutomationError(eventName, objId, "target not found");
    }
    return target;
}

UIElement* DefaultAutomation::requireElement(const std::string& eventName,
                                             const std::string& objId) const {
    UIElement* el = m_map.resolveElement(objId);
    if (!el) {
        throw AutomationError(eventName, objId, "UI element not found (bindArch?)");
    }
    return el;
}

void DefaultAutomation::composeEvent(const std::string& eventName, const std::string& objId,
                                     const boost::json::object& data) {
    const std::string name = normalizeEventName(eventName);
    if (name == "wait" || name == "sleep" || name == "pause") {
        throw AutomationError(name, objId, "wait is handled by ScriptRunner, not DefaultAutomation");
    }

    const EventRegistry::Handler* handler = EventRegistry::instance().find(name);
    if (!handler) {
        throw AutomationError(eventName, objId, "unknown event '" + eventName + "'");
    }
    (this->*(*handler))(objId, data);
}

// --- Registry handlers ---

void DefaultAutomation::handleClick(const std::string& objId, const boost::json::object& data) {
    composeClick(requireTarget("click", objId), data,
                 mouseButtonFromName(getString(data, "button", "left")), false);
}

void DefaultAutomation::handleDblClick(const std::string& objId, const boost::json::object& data) {
    composeClick(requireTarget("dblclick", objId), data,
                 mouseButtonFromName(getString(data, "button", "left")), true);
}

void DefaultAutomation::handleRightClick(const std::string& objId,
                                         const boost::json::object& data) {
    composeClick(requireTarget("rightclick", objId), data, wxMOUSE_BTN_RIGHT, false);
}

void DefaultAutomation::handleMiddleClick(const std::string& objId,
                                          const boost::json::object& data) {
    composeClick(requireTarget("middleclick", objId), data, wxMOUSE_BTN_MIDDLE, false);
}

void DefaultAutomation::handleMouseDown(const std::string& objId, const boost::json::object& data) {
    const int button = mouseButtonFromName(getString(data, "button", "left"));
    composeMouse(requireTarget("mousedown", objId), mouseDownType(button), data, button);
}

void DefaultAutomation::handleMouseUp(const std::string& objId, const boost::json::object& data) {
    const int button = mouseButtonFromName(getString(data, "button", "left"));
    composeMouse(requireTarget("mouseup", objId), mouseUpType(button), data, button);
}

void DefaultAutomation::handleMouseMove(const std::string& objId, const boost::json::object& data) {
    composeMouse(requireTarget("mousemove", objId), wxEVT_MOTION, data);
}

void DefaultAutomation::handleMouseEnter(const std::string& objId,
                                         const boost::json::object& data) {
    composeMouse(requireTarget("mouseenter", objId), wxEVT_ENTER_WINDOW, data);
}

void DefaultAutomation::handleMouseLeave(const std::string& objId,
                                         const boost::json::object& data) {
    composeMouse(requireTarget("mouseleave", objId), wxEVT_LEAVE_WINDOW, data);
}

void DefaultAutomation::handleMouseWheel(const std::string& objId,
                                         const boost::json::object& data) {
    composeMouseWheel(requireTarget("mousewheel", objId), data);
}

void DefaultAutomation::handleAux1Click(const std::string& objId,
                                        const boost::json::object& data) {
#if defined(wxMOUSE_BTN_AUX1)
    composeClick(requireTarget("aux1click", objId), data, wxMOUSE_BTN_AUX1, false);
#else
    composeMouse(requireTarget("aux1click", objId), wxEVT_AUX1_DOWN, data);
    composeMouse(requireTarget("aux1click", objId), wxEVT_AUX1_UP, data);
    (void)data;
#endif
}

void DefaultAutomation::handleAux2Click(const std::string& objId,
                                        const boost::json::object& data) {
#if defined(wxMOUSE_BTN_AUX2)
    composeClick(requireTarget("aux2click", objId), data, wxMOUSE_BTN_AUX2, false);
#else
    composeMouse(requireTarget("aux2click", objId), wxEVT_AUX2_DOWN, data);
    composeMouse(requireTarget("aux2click", objId), wxEVT_AUX2_UP, data);
    (void)data;
#endif
}

#if BAS_WX_MODERN
void DefaultAutomation::handleMagnify(const std::string& objId, const boost::json::object& data) {
    wxWindow* target = requireTarget("magnify", objId);
    wxMouseEvent event(wxEVT_MAGNIFY);
    event.SetEventObject(target);
    event.SetId(target->GetId());
    event.m_x = getMouseX(data, target->GetClientSize().x / 2);
    event.m_y = getMouseY(data, target->GetClientSize().y / 2);
#if wxCHECK_VERSION(3, 1, 0)
    event.m_magnification = static_cast<float>(getDouble(data, "factor", getDouble(data, "magnification", 1.0)));
#endif
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::handleGesturePan(const std::string& objId,
                                         const boost::json::object& data) {
    wxWindow* target = requireTarget("gesturepan", objId);
    wxPanGestureEvent event(target->GetId());
    event.SetEventObject(target);
#if wxCHECK_VERSION(3, 1, 0)
    event.SetDelta(wxPoint(static_cast<int>(getInt(data, "dx", 0)),
                           static_cast<int>(getInt(data, "dy", 0))));
#endif
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::handleGestureZoom(const std::string& objId,
                                          const boost::json::object& data) {
    wxWindow* target = requireTarget("gesturezoom", objId);
    wxZoomGestureEvent event(target->GetId());
    event.SetEventObject(target);
#if wxCHECK_VERSION(3, 1, 0)
    event.SetZoomFactor(getDouble(data, "factor", getDouble(data, "zoom", 1.0)));
#endif
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::handleGestureRotate(const std::string& objId,
                                            const boost::json::object& data) {
    wxWindow* target = requireTarget("gesturerotate", objId);
    wxRotateGestureEvent event(target->GetId());
    event.SetEventObject(target);
#if wxCHECK_VERSION(3, 1, 0)
    event.SetRotationAngle(getDouble(data, "angle", getDouble(data, "rotation", 0.0)));
#endif
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::handleTwoFingerTap(const std::string& objId,
                                           const boost::json::object& data) {
    wxWindow* target = requireTarget("twofingertap", objId);
    wxTwoFingerTapEvent event(target->GetId());
    event.SetEventObject(target);
    (void)data;
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::handleLongPress(const std::string& objId, const boost::json::object& data) {
    wxWindow* target = requireTarget("longpress", objId);
    wxLongPressEvent event(target->GetId());
    event.SetEventObject(target);
    (void)data;
    target->GetEventHandler()->ProcessEvent(event);
}
#endif // BAS_WX_MODERN

void DefaultAutomation::handleType(const std::string& objId, const boost::json::object& data) {
    composeType(requireTarget("type", objId), data, /*clearFirst=*/false);
}

void DefaultAutomation::handleSetValue(const std::string& objId, const boost::json::object& data) {
    // Prefer arch state if bound; otherwise text-like widget.
    if (UIElement* el = m_map.resolveElement(objId)) {
        if (el->isState()) {
            composeSetState(el, data);
            return;
        }
    }
    composeType(requireTarget("setvalue", objId), data, /*clearFirst=*/true);
}

void DefaultAutomation::handleClear(const std::string& objId, const boost::json::object& data) {
    (void)data;
    composeClear(requireTarget("clear", objId));
}

void DefaultAutomation::handleKey(const std::string& objId, const boost::json::object& data) {
    composeKey(requireTarget("key", objId), wxEVT_KEY_DOWN, data, /*fullSequence=*/true);
}

void DefaultAutomation::handleKeyDown(const std::string& objId, const boost::json::object& data) {
    composeKey(requireTarget("keydown", objId), wxEVT_KEY_DOWN, data, false);
}

void DefaultAutomation::handleKeyUp(const std::string& objId, const boost::json::object& data) {
    composeKey(requireTarget("keyup", objId), wxEVT_KEY_UP, data, false);
}

void DefaultAutomation::handleChar(const std::string& objId, const boost::json::object& data) {
    composeKey(requireTarget("char", objId), wxEVT_CHAR, data, false);
}

void DefaultAutomation::handleCharHook(const std::string& objId, const boost::json::object& data) {
    composeKey(requireTarget("charhook", objId), wxEVT_CHAR_HOOK, data, false);
}

void DefaultAutomation::handleSelect(const std::string& objId, const boost::json::object& data) {
    if (UIElement* el = m_map.resolveElement(objId)) {
        if (el->isState()) {
            composeSetState(el, data);
            return;
        }
    }
    composeSelect(requireTarget("select", objId), data);
}

void DefaultAutomation::handleCheck(const std::string& objId, const boost::json::object& data) {
    if (UIElement* el = m_map.resolveElement(objId)) {
        if (el->isState()) {
            boost::json::object d = data;
            d["value"] = true;
            composeSetState(el, d);
            return;
        }
    }
    composeCheck(requireTarget("check", objId), "check", data);
}

void DefaultAutomation::handleUncheck(const std::string& objId, const boost::json::object& data) {
    if (UIElement* el = m_map.resolveElement(objId)) {
        if (el->isState()) {
            boost::json::object d = data;
            d["value"] = false;
            composeSetState(el, d);
            return;
        }
    }
    composeCheck(requireTarget("uncheck", objId), "uncheck", data);
}

void DefaultAutomation::handleToggle(const std::string& objId, const boost::json::object& data) {
    if (UIElement* el = m_map.resolveElement(objId)) {
        if (el->isState()) {
            composeToggleState(el, data);
            return;
        }
    }
    composeCheck(requireTarget("toggle", objId), "toggle", data);
}

void DefaultAutomation::handleFocus(const std::string& objId, const boost::json::object& data) {
    (void)data;
    composeFocus(requireTarget("focus", objId), true);
}

void DefaultAutomation::handleBlur(const std::string& objId, const boost::json::object& data) {
    (void)data;
    composeFocus(requireTarget("blur", objId), false);
}

void DefaultAutomation::handleActivate(const std::string& objId, const boost::json::object& data) {
    if (UIElement* el = m_map.resolveElement(objId)) {
        if (el->isAction()) {
            composePerform(el, data);
            return;
        }
    }
    if (wxWindow* target = m_map.resolve(m_root, objId)) {
        composeActivate(target, data);
        return;
    }
    throw AutomationError("activate", objId, "target not found");
}

void DefaultAutomation::handleScroll(const std::string& objId, const boost::json::object& data) {
    composeScroll(requireTarget("scroll", objId), data);
}

void DefaultAutomation::handlePerform(const std::string& objId, const boost::json::object& data) {
    composePerform(requireElement("perform", objId), data);
}

void DefaultAutomation::handleSetState(const std::string& objId, const boost::json::object& data) {
    composeSetState(requireElement("setstate", objId), data);
}

void DefaultAutomation::handleToggleState(const std::string& objId,
                                          const boost::json::object& data) {
    composeToggleState(requireElement("togglestate", objId), data);
}

// --- Composers ---

void DefaultAutomation::postMouse(wxWindow* target, wxEventType type, int x, int y, int button,
                                  int modifiers) {
    wxMouseEvent event(type);
    event.SetEventObject(target);
    event.SetId(target->GetId());
    event.m_x = x;
    event.m_y = y;
    event.m_leftDown =
        (button == wxMOUSE_BTN_LEFT &&
         (type == wxEVT_LEFT_DOWN || type == wxEVT_LEFT_DCLICK || type == wxEVT_MOTION));
    event.m_middleDown =
        (button == wxMOUSE_BTN_MIDDLE &&
         (type == wxEVT_MIDDLE_DOWN || type == wxEVT_MIDDLE_DCLICK || type == wxEVT_MOTION));
    event.m_rightDown =
        (button == wxMOUSE_BTN_RIGHT &&
         (type == wxEVT_RIGHT_DOWN || type == wxEVT_RIGHT_DCLICK || type == wxEVT_MOTION));
    event.m_controlDown = (modifiers & wxMOD_CONTROL) != 0;
    event.m_shiftDown = (modifiers & wxMOD_SHIFT) != 0;
    event.m_altDown = (modifiers & wxMOD_ALT) != 0;
    event.m_metaDown = (modifiers & wxMOD_META) != 0;
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::postCommand(wxWindow* target, wxEventType type) {
    wxCommandEvent event(type, target->GetId());
    event.SetEventObject(target);
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::composeMouse(wxWindow* target, wxEventType type,
                                     const boost::json::object& data, int button) {
    postMouse(target, type, getMouseX(data, 0), getMouseY(data, 0), button, parseModifiers(data));
}

void DefaultAutomation::composeMouseWheel(wxWindow* target, const boost::json::object& data) {
    wxMouseEvent event(wxEVT_MOUSEWHEEL);
    event.SetEventObject(target);
    event.SetId(target->GetId());
    event.m_x = getMouseX(data, target->GetClientSize().x / 2);
    event.m_y = getMouseY(data, target->GetClientSize().y / 2);
    event.m_wheelRotation = static_cast<int>(getInt(data, "rotation", getInt(data, "delta", 120)));
    event.m_wheelDelta = static_cast<int>(getInt(data, "wheelDelta", 120));
#if BAS_WX_MODERN
    const std::string axis = lowerCopy(getString(data, "axis", "vertical"));
    event.m_wheelAxis = (axis == "horizontal" || axis == "h") ? wxMOUSE_WHEEL_HORIZONTAL
                                                              : wxMOUSE_WHEEL_VERTICAL;
#endif
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::composeClick(wxWindow* target, const boost::json::object& data, int button,
                                     bool doubleClick) {
    const int x = getMouseX(data, target->GetClientSize().x / 2);
    const int y = getMouseY(data, target->GetClientSize().y / 2);
    const int mods = parseModifiers(data);

    if (doubleClick) {
        postMouse(target, mouseDownType(button), x, y, button, mods);
        postMouse(target, mouseUpType(button), x, y, button, mods);
        postMouse(target, mouseDClickType(button), x, y, button, mods);
        postMouse(target, mouseUpType(button), x, y, button, mods);
    } else {
        postMouse(target, mouseDownType(button), x, y, button, mods);
        postMouse(target, mouseUpType(button), x, y, button, mods);
    }

    if (button == wxMOUSE_BTN_LEFT && !doubleClick) {
        if (asButton(target)) {
            postCommand(target, wxEVT_BUTTON);
        } else if (asCheckBox(target)) {
            auto* box = asCheckBox(target);
            box->SetValue(!box->GetValue());
            postCommand(target, wxEVT_CHECKBOX);
        } else if (asRadioButton(target)) {
            asRadioButton(target)->SetValue(true);
            postCommand(target, wxEVT_RADIOBUTTON);
        }
    }
}

void DefaultAutomation::composeClear(wxWindow* target) {
    if (auto* text = asTextCtrl(target)) {
        text->Clear();
        postCommand(target, wxEVT_TEXT);
        return;
    }
    if (auto* combo = asComboBox(target)) {
        combo->SetValue(wxEmptyString);
        postCommand(target, wxEVT_TEXT);
        return;
    }
    throw AutomationError("clear", std::string(target->GetName().utf8_str()),
                          "target does not support clear");
}

void DefaultAutomation::composeType(wxWindow* target, const boost::json::object& data,
                                    bool clearFirst) {
    const std::string text = getString2(data, "text", "value");
    if (clearFirst && (asTextCtrl(target) || asComboBox(target))) {
        composeClear(target);
    }

    if (auto* ctrl = asTextCtrl(target)) {
        if (clearFirst) {
            ctrl->SetValue(utf8(text));
        } else {
            ctrl->AppendText(utf8(text));
        }
        postCommand(target, wxEVT_TEXT);
        return;
    }
    if (auto* combo = asComboBox(target)) {
        if (clearFirst || combo->GetValue().empty()) {
            combo->SetValue(utf8(text));
        } else {
            combo->SetValue(combo->GetValue() + utf8(text));
        }
        postCommand(target, wxEVT_TEXT);
        return;
    }
    throw AutomationError("type", std::string(target->GetName().utf8_str()),
                          "target does not support text input");
}

int DefaultAutomation::parseModifiers(const boost::json::object& data) {
    int mods = 0;
    if (getBool(data, "ctrl") || getBool(data, "control")) {
        mods |= wxMOD_CONTROL;
    }
    if (getBool(data, "shift")) {
        mods |= wxMOD_SHIFT;
    }
    if (getBool(data, "alt")) {
        mods |= wxMOD_ALT;
    }
    if (getBool(data, "meta") || getBool(data, "cmd")) {
        mods |= wxMOD_META;
    }
    const std::string modsStr = lowerCopy(getString2(data, "mods", "modifiers"));
    if (!modsStr.empty()) {
        if (modsStr.find("ctrl") != std::string::npos ||
            modsStr.find("control") != std::string::npos) {
            mods |= wxMOD_CONTROL;
        }
        if (modsStr.find("shift") != std::string::npos) {
            mods |= wxMOD_SHIFT;
        }
        if (modsStr.find("alt") != std::string::npos) {
            mods |= wxMOD_ALT;
        }
        if (modsStr.find("meta") != std::string::npos || modsStr.find("cmd") != std::string::npos) {
            mods |= wxMOD_META;
        }
    }
    return mods;
}

int DefaultAutomation::parseKeyCode(const boost::json::object& data) {
    if (const auto* v = findValue(data, "keycode")) {
        return static_cast<int>(jsonAsInt(*v, 0));
    }
    if (const auto* v = findValue(data, "keyCode")) {
        return static_cast<int>(jsonAsInt(*v, 0));
    }

    const std::string key = getString2(data, "key", "char");
    if (key.empty()) {
        return 0;
    }
    const std::string k = lowerCopy(key);

    static const std::pair<const char*, int> named[] = {
        {"return", WXK_RETURN}, {"enter", WXK_RETURN},  {"tab", WXK_TAB},
        {"escape", WXK_ESCAPE}, {"esc", WXK_ESCAPE},    {"backspace", WXK_BACK},
        {"back", WXK_BACK},     {"delete", WXK_DELETE}, {"del", WXK_DELETE},
        {"space", WXK_SPACE},   {"up", WXK_UP},         {"down", WXK_DOWN},
        {"left", WXK_LEFT},     {"right", WXK_RIGHT},   {"home", WXK_HOME},
        {"end", WXK_END},       {"pageup", WXK_PAGEUP}, {"pagedown", WXK_PAGEDOWN},
        {"insert", WXK_INSERT}, {"f1", WXK_F1},         {"f2", WXK_F2},
        {"f3", WXK_F3},         {"f4", WXK_F4},         {"f5", WXK_F5},
        {"f6", WXK_F6},         {"f7", WXK_F7},         {"f8", WXK_F8},
        {"f9", WXK_F9},         {"f10", WXK_F10},       {"f11", WXK_F11},
        {"f12", WXK_F12},
    };
    for (const auto& [name, code] : named) {
        if (k == name) {
            return code;
        }
    }
    if (key.size() == 1) {
        return static_cast<unsigned char>(key[0]);
    }
    return 0;
}

void DefaultAutomation::postKey(wxWindow* target, wxEventType type, int keyCode, int modifiers,
                                wxChar ch) {
    wxKeyEvent event(type);
    event.SetEventObject(target);
    event.SetId(target->GetId());
    event.m_keyCode = keyCode;
#if BAS_WX_MODERN
    event.m_uniChar = ch ? ch : static_cast<wxChar>(keyCode);
#elif defined(wxHAS_RAW_KEY_CODES)
    event.m_uniChar = ch ? ch : static_cast<wxChar>(keyCode);
#else
    (void)ch;
#endif
    event.m_controlDown = (modifiers & wxMOD_CONTROL) != 0;
    event.m_shiftDown = (modifiers & wxMOD_SHIFT) != 0;
    event.m_altDown = (modifiers & wxMOD_ALT) != 0;
    event.m_metaDown = (modifiers & wxMOD_META) != 0;
    target->GetEventHandler()->ProcessEvent(event);
}

void DefaultAutomation::composeKey(wxWindow* target, wxEventType type,
                                   const boost::json::object& data, bool fullSequence) {
    const int keyCode = parseKeyCode(data);
    if (keyCode == 0) {
        throw AutomationError("key", std::string(target->GetName().utf8_str()),
                              "key / keyCode is required");
    }
    const int mods = parseModifiers(data);
    wxChar ch = 0;
    const std::string chStr = getString(data, "char");
    if (chStr.size() == 1) {
        ch = static_cast<unsigned char>(chStr[0]);
    } else if (keyCode >= 32 && keyCode < 127) {
        ch = static_cast<wxChar>(keyCode);
    }

    if (fullSequence) {
        postKey(target, wxEVT_KEY_DOWN, keyCode, mods, ch);
        postKey(target, wxEVT_CHAR, keyCode, mods, ch);
        postKey(target, wxEVT_KEY_UP, keyCode, mods, ch);
        return;
    }
    postKey(target, type, keyCode, mods, ch);
}

void DefaultAutomation::composeSelect(wxWindow* target, const boost::json::object& data) {
    const bool hasIndex = findValue(data, "index") != nullptr;
    const int index = static_cast<int>(getInt(data, "index", -1));
    std::string value = getString2(data, "value", "text");
    if (value.empty()) {
        value = getString(data, "label");
    }

    auto selectByIndexOrString = [&](auto* ctrl, wxEventType evt) {
        if (!ctrl) {
            return false;
        }
        if (hasIndex) {
            if (index < 0 || index >= static_cast<int>(ctrl->GetCount())) {
                throw AutomationError("select", std::string(target->GetName().utf8_str()),
                                      "index out of range");
            }
            ctrl->SetSelection(index);
        } else if (!value.empty()) {
            const int found = ctrl->FindString(utf8(value));
            if (found == wxNOT_FOUND) {
                throw AutomationError("select", std::string(target->GetName().utf8_str()),
                                      "value not found: " + value);
            }
            ctrl->SetSelection(found);
        } else {
            throw AutomationError("select", std::string(target->GetName().utf8_str()),
                                  "index or value is required");
        }
        postCommand(target, evt);
        return true;
    };

    if (selectByIndexOrString(asChoice(target), wxEVT_CHOICE)) {
        return;
    }
    if (selectByIndexOrString(asComboBox(target), wxEVT_COMBOBOX)) {
        return;
    }
    if (selectByIndexOrString(asListBox(target), wxEVT_LISTBOX)) {
        return;
    }
    throw AutomationError("select", std::string(target->GetName().utf8_str()),
                          "target does not support select");
}

void DefaultAutomation::composeCheck(wxWindow* target, const std::string& eventName,
                                     const boost::json::object& data) {
    bool want = true;
    if (eventName == "uncheck") {
        want = false;
    } else if (eventName == "toggle") {
        if (auto* box = asCheckBox(target)) {
            want = !box->GetValue();
        } else if (auto* radio = asRadioButton(target)) {
            want = !radio->GetValue();
        } else {
#if wxUSE_CHECKLISTBOX
            if (auto* list = asCheckListBox(target)) {
                const int index = static_cast<int>(getInt(data, "index", 0));
                if (index < 0 || index >= static_cast<int>(list->GetCount())) {
                    throw AutomationError(eventName, std::string(target->GetName().utf8_str()),
                                          "index out of range");
                }
                list->Check(index, !list->IsChecked(index));
                postCommand(target, wxEVT_CHECKLISTBOX);
                return;
            }
#endif
            throw AutomationError(eventName, std::string(target->GetName().utf8_str()),
                                  "target does not support toggle");
        }
    } else if (findValue(data, "value") || findValue(data, "checked")) {
        want = getBool(data, "value", getBool(data, "checked", true));
    }

    if (auto* box = asCheckBox(target)) {
        if (box->GetValue() != want) {
            box->SetValue(want);
            postCommand(target, wxEVT_CHECKBOX);
        }
        return;
    }
    if (auto* radio = asRadioButton(target)) {
        if (want) {
            radio->SetValue(true);
            postCommand(target, wxEVT_RADIOBUTTON);
        }
        return;
    }
#if wxUSE_CHECKLISTBOX
    if (auto* list = asCheckListBox(target)) {
        const int index = static_cast<int>(getInt(data, "index", 0));
        if (index < 0 || index >= static_cast<int>(list->GetCount())) {
            throw AutomationError(eventName, std::string(target->GetName().utf8_str()),
                                  "index out of range");
        }
        list->Check(index, want);
        postCommand(target, wxEVT_CHECKLISTBOX);
        return;
    }
#endif
    throw AutomationError(eventName, std::string(target->GetName().utf8_str()),
                          "target does not support check");
}

void DefaultAutomation::composeFocus(wxWindow* target, bool focus) {
    if (focus) {
        target->SetFocus();
        wxFocusEvent event(wxEVT_SET_FOCUS, target->GetId());
        event.SetEventObject(target);
        target->GetEventHandler()->ProcessEvent(event);
    } else {
        wxFocusEvent event(wxEVT_KILL_FOCUS, target->GetId());
        event.SetEventObject(target);
        target->GetEventHandler()->ProcessEvent(event);
    }
}

void DefaultAutomation::composeActivate(wxWindow* target, const boost::json::object& /*data*/) {
    if (asButton(target)) {
        postCommand(target, wxEVT_BUTTON);
        return;
    }
    if (asCheckBox(target)) {
        postCommand(target, wxEVT_CHECKBOX);
        return;
    }
    if (asRadioButton(target)) {
        postCommand(target, wxEVT_RADIOBUTTON);
        return;
    }
    if (asChoice(target)) {
        postCommand(target, wxEVT_CHOICE);
        return;
    }
    if (asComboBox(target)) {
        postCommand(target, wxEVT_COMBOBOX);
        return;
    }
    if (asListBox(target)) {
        postCommand(target, wxEVT_LISTBOX);
        return;
    }
    postCommand(target, wxEVT_BUTTON);
}

void DefaultAutomation::composeScroll(wxWindow* target, const boost::json::object& data) {
    const int dx = static_cast<int>(getInt2(data, "dx", "x", 0));
    const int dy = static_cast<int>(getInt2(data, "dy", "y", 0));
    const std::string orient = lowerCopy(getString2(data, "orient", "orientation"));

    if (auto* scrolled = dynamic_cast<wxScrolledWindow*>(target)) {
        int x = 0;
        int y = 0;
        scrolled->GetViewStart(&x, &y);
        scrolled->Scroll(x + dx, y + dy);
        return;
    }

    const wxEventType type = wxEVT_SCROLLWIN_THUMBTRACK;
    if (orient == "horizontal" || orient == "h" || dx != 0) {
        wxScrollWinEvent event(type, dx, wxHORIZONTAL);
        event.SetEventObject(target);
        target->GetEventHandler()->ProcessEvent(event);
    }
    if (orient == "vertical" || orient == "v" || dy != 0 || orient.empty()) {
        wxScrollWinEvent event(type, dy, wxVERTICAL);
        event.SetEventObject(target);
        target->GetEventHandler()->ProcessEvent(event);
    }
}

void DefaultAutomation::composePerform(UIElement* element, const boost::json::object& data) {
    auto* action = dynamic_cast<UIAction*>(element);
    if (!action) {
        throw AutomationError("perform", element->name(), "element is not a UIAction");
    }

    // Prefer invoking performFn directly (works without a live menu item).
    std::vector<std::string> argStorage;
    if (const auto* args = findValue(data, "args")) {
        if (args->is_array()) {
            for (const auto& a : args->as_array()) {
                argStorage.push_back(jsonAsString(a));
            }
        }
    }
    std::vector<const char*> argv;
    argv.reserve(argStorage.size() + 1);
    for (const auto& s : argStorage) {
        argv.push_back(s.c_str());
    }
    argv.push_back(nullptr);

    wxCommandEvent cmd(wxEVT_MENU, action->id);
    if (m_root) {
        cmd.SetEventObject(m_root);
    }
    PerformContext ctx(action, static_cast<int>(argStorage.size()),
                       argStorage.empty() ? nullptr : argv.data(), &cmd);
    action->perform(&ctx);

    // Also deliver a command event to the frame so Bound handlers see it.
    if (m_root) {
        m_root->GetEventHandler()->ProcessEvent(cmd);
    }
}

void DefaultAutomation::composeSetState(UIElement* element, const boost::json::object& data) {
    auto* state = dynamic_cast<UIState*>(element);
    if (!state) {
        throw AutomationError("setstate", element->name(), "element is not a UIState");
    }

    switch (state->getType()) {
    case UIStateType::BOOL: {
        const bool v = getBool(data, "value", getBool(data, "checked", true));
        state->value.set(v);
        break;
    }
    case UIStateType::ENUM: {
        if (findValue(data, "index") || findValue(data, "value")) {
            int v = static_cast<int>(getInt2(data, "value", "index", 0));
            // If "value" is a label string, match enum descriptor.
            if (const auto* raw = findValue(data, "value")) {
                if (raw->is_string()) {
                    const std::string label = jsonAsString(*raw);
                    bool matched = false;
                    for (int ev : state->getEnumValues()) {
                        if (auto desc = state->getValueDescriptor(ev)) {
                            if (desc->label == label) {
                                v = ev;
                                matched = true;
                                break;
                            }
                        }
                    }
                    if (!matched) {
                        try {
                            v = std::stoi(label);
                        } catch (...) {
                            throw AutomationError("setstate", element->name(),
                                                  "enum value not found: " + label);
                        }
                    }
                }
            }
            state->value.set(v);
        } else {
            throw AutomationError("setstate", element->name(), "value or index required for ENUM");
        }
        break;
    }
    case UIStateType::STRING: {
        state->value.set(getString2(data, "text", "value"));
        break;
    }
    default:
        throw AutomationError("setstate", element->name(), "unsupported UIStateType");
    }
}

void DefaultAutomation::composeToggleState(UIElement* element, const boost::json::object& data) {
    auto* state = dynamic_cast<UIState*>(element);
    if (!state) {
        throw AutomationError("togglestate", element->name(), "element is not a UIState");
    }
    if (state->getType() == UIStateType::BOOL) {
        const bool cur = std::holds_alternative<bool>(state->value.get())
                             ? std::get<bool>(state->value.get())
                             : false;
        state->value.set(!cur);
        return;
    }
    if (state->getType() == UIStateType::ENUM) {
        const auto& values = state->getEnumValues();
        if (values.empty()) {
            throw AutomationError("togglestate", element->name(), "ENUM has no values");
        }
        int cur = std::holds_alternative<int>(state->value.get()) ? std::get<int>(state->value.get())
                                                                  : values.front();
        auto it = std::find(values.begin(), values.end(), cur);
        if (it == values.end() || std::next(it) == values.end()) {
            state->value.set(values.front());
        } else {
            state->value.set(*std::next(it));
        }
        (void)data;
        return;
    }
    throw AutomationError("togglestate", element->name(), "toggle only for BOOL/ENUM");
}

} // namespace bas::ui::automation
