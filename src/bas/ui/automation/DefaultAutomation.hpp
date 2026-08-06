#ifndef BAS_UI_AUTOMATION_DEFAULT_AUTOMATION_HPP
#define BAS_UI_AUTOMATION_DEFAULT_AUTOMATION_HPP

#include "AutomationMap.hpp"
#include "IAutomation.hpp"

#include <bas/wx/wx_compat.hpp>

#include <wx/window.h>

#include <boost/json.hpp>

#include <string>

class UIElement;
class UIGroup;

namespace bas::ui::automation {

class EventRegistry;

/**
 * Default ProcessEvent-based automation driver.
 *
 * Event names are dispatched via EventRegistry (version-gated map), not an
 * if/else chain. Arch (UIAction / UIState) targets are resolved through
 * AutomationMap element bindings.
 *
 * @p nothrow when true: emulate() catches AutomationError and returns false.
 */
class DefaultAutomation : public IAutomation {
    friend class EventRegistry;

  public:
    explicit DefaultAutomation(wxWindow* root, bool nothrow = false);
    DefaultAutomation(wxWindow* root, AutomationMap map, bool nothrow = false);

    wxWindow* root() const { return m_root; }
    void setRoot(wxWindow* root) { m_root = root; }

    AutomationMap& map() { return m_map; }
    const AutomationMap& map() const { return m_map; }

    bool nothrow() const { return m_nothrow; }
    void setNothrow(bool v) { m_nothrow = v; }

    /** Bind UIAction/UIState/UIGroup tree under pathFromRoot() and leaf name. */
    void bindArch(UIElement* element);
    void bindArch(UIGroup* group);

    bool emulate(const std::string& eventName, const std::string& objId,
                 const boost::json::object& data = {}) override;

    bool tryEmulate(const std::string& eventName, const std::string& objId,
                    const boost::json::object& data = {});

    wxWindow* requireTarget(const std::string& eventName, const std::string& objId) const;
    UIElement* requireElement(const std::string& eventName, const std::string& objId) const;

  private:
    void composeEvent(const std::string& eventName, const std::string& objId,
                      const boost::json::object& data);

    // --- EventRegistry handlers (objId + data) ---
    void handleClick(const std::string& objId, const boost::json::object& data);
    void handleDblClick(const std::string& objId, const boost::json::object& data);
    void handleRightClick(const std::string& objId, const boost::json::object& data);
    void handleMiddleClick(const std::string& objId, const boost::json::object& data);
    void handleMouseDown(const std::string& objId, const boost::json::object& data);
    void handleMouseUp(const std::string& objId, const boost::json::object& data);
    void handleMouseMove(const std::string& objId, const boost::json::object& data);
    void handleMouseEnter(const std::string& objId, const boost::json::object& data);
    void handleMouseLeave(const std::string& objId, const boost::json::object& data);
    void handleMouseWheel(const std::string& objId, const boost::json::object& data);
    void handleAux1Click(const std::string& objId, const boost::json::object& data);
    void handleAux2Click(const std::string& objId, const boost::json::object& data);
#if BAS_WX_MODERN
    void handleMagnify(const std::string& objId, const boost::json::object& data);
    void handleGesturePan(const std::string& objId, const boost::json::object& data);
    void handleGestureZoom(const std::string& objId, const boost::json::object& data);
    void handleGestureRotate(const std::string& objId, const boost::json::object& data);
    void handleTwoFingerTap(const std::string& objId, const boost::json::object& data);
    void handleLongPress(const std::string& objId, const boost::json::object& data);
#endif
    void handleType(const std::string& objId, const boost::json::object& data);
    void handleSetValue(const std::string& objId, const boost::json::object& data);
    void handleClear(const std::string& objId, const boost::json::object& data);
    void handleKey(const std::string& objId, const boost::json::object& data);
    void handleKeyDown(const std::string& objId, const boost::json::object& data);
    void handleKeyUp(const std::string& objId, const boost::json::object& data);
    void handleChar(const std::string& objId, const boost::json::object& data);
    void handleCharHook(const std::string& objId, const boost::json::object& data);
    void handleSelect(const std::string& objId, const boost::json::object& data);
    void handleCheck(const std::string& objId, const boost::json::object& data);
    void handleUncheck(const std::string& objId, const boost::json::object& data);
    void handleToggle(const std::string& objId, const boost::json::object& data);
    void handleFocus(const std::string& objId, const boost::json::object& data);
    void handleBlur(const std::string& objId, const boost::json::object& data);
    void handleActivate(const std::string& objId, const boost::json::object& data);
    void handleScroll(const std::string& objId, const boost::json::object& data);
    void handlePerform(const std::string& objId, const boost::json::object& data);
    void handleSetState(const std::string& objId, const boost::json::object& data);
    void handleToggleState(const std::string& objId, const boost::json::object& data);

    void composeClick(wxWindow* target, const boost::json::object& data, int button,
                      bool doubleClick);
    void composeMouse(wxWindow* target, wxEventType type, const boost::json::object& data,
                      int button = wxMOUSE_BTN_NONE);
    void composeMouseWheel(wxWindow* target, const boost::json::object& data);
    void composeType(wxWindow* target, const boost::json::object& data, bool clearFirst);
    void composeClear(wxWindow* target);
    void composeKey(wxWindow* target, wxEventType type, const boost::json::object& data,
                    bool fullSequence);
    void composeSelect(wxWindow* target, const boost::json::object& data);
    void composeCheck(wxWindow* target, const std::string& eventName,
                      const boost::json::object& data);
    void composeFocus(wxWindow* target, bool focus);
    void composeActivate(wxWindow* target, const boost::json::object& data);
    void composeScroll(wxWindow* target, const boost::json::object& data);
    void composePerform(UIElement* element, const boost::json::object& data);
    void composeSetState(UIElement* element, const boost::json::object& data);
    void composeToggleState(UIElement* element, const boost::json::object& data);

    void postMouse(wxWindow* target, wxEventType type, int x, int y, int button,
                   int modifiers = 0);
    void postCommand(wxWindow* target, wxEventType type);
    void postKey(wxWindow* target, wxEventType type, int keyCode, int modifiers, wxChar ch);

    static int parseKeyCode(const boost::json::object& data);
    static int parseModifiers(const boost::json::object& data);
    static std::string normalizeEventName(std::string name);

    wxWindow* m_root = nullptr;
    AutomationMap m_map;
    bool m_nothrow = false;
};

} // namespace bas::ui::automation

#endif
