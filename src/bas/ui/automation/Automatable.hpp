#ifndef BAS_UI_AUTOMATION_AUTOMATABLE_HPP
#define BAS_UI_AUTOMATION_AUTOMATABLE_HPP

#include "AutomationMap.hpp"
#include "AutomationQuery.hpp"
#include "DefaultAutomation.hpp"
#include "IAutomation.hpp"

#include <wx/window.h>

#include <boost/json.hpp>

#include <string>

class UIElement;
class UIGroup;

namespace bas::ui::automation {

/**
 * Mixin that wires IAutomation onto a wxWindow (dialog/frame/panel).
 *
 * Typical usage:
 *
 *   class MyDialog : public wxDialog, public Automatable {
 *   public:
 *     MyDialog(wxWindow* parent)
 *       : wxDialog(parent, ...), Automatable(this) {
 *         automationMap().bind("ok", wxID_OK);
 *       }
 *   };
 */
class Automatable : public IAutomation {
  public:
    explicit Automatable(wxWindow* root, bool nothrow = false)
        : m_automation(root, nothrow) {}

    bool emulate(const std::string& eventName, const std::string& objId,
                 const boost::json::object& data = {}) override {
        return m_automation.emulate(eventName, objId, data);
    }

    bool tryEmulate(const std::string& eventName, const std::string& objId,
                    const boost::json::object& data = {}) {
        return m_automation.tryEmulate(eventName, objId, data);
    }

    AutomationMap& automationMap() { return m_automation.map(); }
    const AutomationMap& automationMap() const { return m_automation.map(); }

    DefaultAutomation& automation() { return m_automation; }
    const DefaultAutomation& automation() const { return m_automation; }

    AutomationQuery query() const {
        return AutomationQuery(m_automation.root(), m_automation.map());
    }

    void bindArch(UIElement* element) { m_automation.bindArch(element); }
    void bindArch(UIGroup* group) { m_automation.bindArch(group); }

  protected:
    DefaultAutomation m_automation;
};

} // namespace bas::ui::automation

#endif
