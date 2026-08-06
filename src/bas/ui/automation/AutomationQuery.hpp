#ifndef BAS_UI_AUTOMATION_QUERY_HPP
#define BAS_UI_AUTOMATION_QUERY_HPP

#include "AutomationMap.hpp"

#include <wx/window.h>

#include <boost/json.hpp>

#include <optional>
#include <string>

class UIElement;

namespace bas::ui::automation {

/**
 * Read-only inspection helpers over a root window + AutomationMap.
 * Used for e2e assertions; does not synthesize events.
 */
class AutomationQuery {
  public:
    AutomationQuery(wxWindow* root, const AutomationMap& map);

    wxWindow* root() const { return m_root; }
    const AutomationMap& map() const { return m_map; }

    wxWindow* find(const std::string& objId) const;
    UIElement* findElement(const std::string& objId) const;
    bool exists(const std::string& objId) const;
    bool elementExists(const std::string& objId) const;

    bool isEnabled(const std::string& objId) const;
    bool isShown(const std::string& objId) const;
    bool isChecked(const std::string& objId) const;

    std::string getLabel(const std::string& objId) const;
    std::string getText(const std::string& objId) const;
    std::string getValue(const std::string& objId) const;

    /** UIState value as JSON (bool / int / string). */
    boost::json::value getStateValue(const std::string& objId) const;

    int getSelection(const std::string& objId) const;
    std::optional<std::string> getSelectedString(const std::string& objId) const;

    /** Dump visible named descendants as a JSON array for debugging. */
    boost::json::array dumpTree() const;

  private:
    wxWindow* require(const std::string& objId) const;
    static void dumpTreeRecursive(wxWindow* node, boost::json::array& out, int depth);

    wxWindow* m_root = nullptr;
    const AutomationMap& m_map;
};

} // namespace bas::ui::automation

#endif
