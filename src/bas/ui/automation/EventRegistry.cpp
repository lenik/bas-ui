#include "EventRegistry.hpp"

#include "DefaultAutomation.hpp"

#include <algorithm>
#include <cctype>

namespace bas::ui::automation {

namespace {

std::string lowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

} // namespace

const EventRegistry& EventRegistry::instance() {
    static const EventRegistry reg;
    return reg;
}

EventRegistry::EventRegistry() {
    // --- Pointer / mouse ---
    add("click", &DefaultAutomation::handleClick);
    addAlias("press", "click");
    addAlias("tap", "click");

    add("dblclick", &DefaultAutomation::handleDblClick);
    addAlias("dbl_click", "dblclick");
    addAlias("doubleclick", "dblclick");
    addAlias("double-click", "dblclick");

    add("rightclick", &DefaultAutomation::handleRightClick);
    addAlias("contextmenu", "rightclick");
    addAlias("context-menu", "rightclick");

    add("middleclick", &DefaultAutomation::handleMiddleClick);

    add("mousedown", &DefaultAutomation::handleMouseDown);
    add("mouseup", &DefaultAutomation::handleMouseUp);
    add("mousemove", &DefaultAutomation::handleMouseMove);
    addAlias("motion", "mousemove");

    add("mouseenter", &DefaultAutomation::handleMouseEnter);
    addAlias("enter", "mouseenter");
    addAlias("enterwindow", "mouseenter");

    add("mouseleave", &DefaultAutomation::handleMouseLeave);
    addAlias("leave", "mouseleave");
    addAlias("leavewindow", "mouseleave");

    add("mousewheel", &DefaultAutomation::handleMouseWheel);
    addAlias("wheel", "mousewheel");
    addAlias("scrollwheel", "mousewheel");

    add("aux1click", &DefaultAutomation::handleAux1Click);
    addAlias("aux1", "aux1click");
    add("aux2click", &DefaultAutomation::handleAux2Click);
    addAlias("aux2", "aux2click");

#if BAS_WX_MODERN
    // wx 3.1+ mouse / gesture events
    add("magnify", &DefaultAutomation::handleMagnify);
    addAlias("pinch", "magnify");

    add("gesturepan", &DefaultAutomation::handleGesturePan);
    addAlias("pan", "gesturepan");

    add("gesturezoom", &DefaultAutomation::handleGestureZoom);
    addAlias("zoom", "gesturezoom");

    add("gesturerotate", &DefaultAutomation::handleGestureRotate);
    addAlias("rotate", "gesturerotate");

    add("twofingertap", &DefaultAutomation::handleTwoFingerTap);
    addAlias("two_finger_tap", "twofingertap");

    add("longpress", &DefaultAutomation::handleLongPress);
    addAlias("long_press", "longpress");
#endif

    // --- Text ---
    add("type", &DefaultAutomation::handleType);
    addAlias("input", "type");
    addAlias("append", "type");

    add("setvalue", &DefaultAutomation::handleSetValue);
    addAlias("set", "setvalue");
    addAlias("settext", "setvalue");
    addAlias("set_text", "setvalue");
    addAlias("fill", "setvalue");

    add("clear", &DefaultAutomation::handleClear);

    // --- Keyboard ---
    add("key", &DefaultAutomation::handleKey);
    add("keydown", &DefaultAutomation::handleKeyDown);
    add("keyup", &DefaultAutomation::handleKeyUp);
    add("char", &DefaultAutomation::handleChar);
    add("charhook", &DefaultAutomation::handleCharHook);
    addAlias("char_hook", "charhook");

    // --- Selection / check ---
    add("select", &DefaultAutomation::handleSelect);
    add("check", &DefaultAutomation::handleCheck);
    add("uncheck", &DefaultAutomation::handleUncheck);
    add("toggle", &DefaultAutomation::handleToggle);

    // --- Focus / activate / scroll ---
    add("focus", &DefaultAutomation::handleFocus);
    add("blur", &DefaultAutomation::handleBlur);
    add("activate", &DefaultAutomation::handleActivate);
    add("scroll", &DefaultAutomation::handleScroll);

    // --- UI arch (UIAction / UIState) ---
    add("perform", &DefaultAutomation::handlePerform);
    addAlias("action", "perform");
    addAlias("invoke", "perform");

    add("setstate", &DefaultAutomation::handleSetState);
    addAlias("set_state", "setstate");

    add("togglestate", &DefaultAutomation::handleToggleState);
    addAlias("toggle_state", "togglestate");
}

void EventRegistry::add(const char* name, Handler handler) {
    m_handlers[lowerCopy(name)] = handler;
}

void EventRegistry::addAlias(const char* alias, const char* canonical) {
    const auto it = m_handlers.find(lowerCopy(canonical));
    if (it == m_handlers.end()) {
        return;
    }
    m_handlers[lowerCopy(alias)] = it->second;
}

const EventRegistry::Handler* EventRegistry::find(const std::string& eventName) const {
    const auto it = m_handlers.find(lowerCopy(eventName));
    if (it == m_handlers.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<std::string> EventRegistry::names() const {
    std::vector<std::string> out;
    out.reserve(m_handlers.size());
    for (const auto& [name, _] : m_handlers) {
        out.push_back(name);
    }
    std::sort(out.begin(), out.end());
    return out;
}

} // namespace bas::ui::automation
