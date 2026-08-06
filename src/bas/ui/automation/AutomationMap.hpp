#ifndef BAS_UI_AUTOMATION_MAP_HPP
#define BAS_UI_AUTOMATION_MAP_HPP

#include <wx/window.h>

#include <string>
#include <unordered_map>
#include <variant>

class UIElement;

namespace bas::ui::automation {

/**
 * Maps logical automation names to wx widgets (by ID or pointer) or UI arch
 * elements (UIAction / UIState / UIGroup).
 *
 * Resolution order for resolve(root, nameOrId):
 *   1. Explicit map entry (ID → FindWindow under root; pointer → as-is)
 *   2. Numeric / "#id" string → FindWindow
 *   3. Recursive wxWindow::GetName() match under root
 *
 * Element resolution (resolveElement):
 *   1. Explicit UIElement* binding
 *   2. ID binding → scan is not available; returns nullptr (use bindArch)
 *   3. Path / name lookup in bound elements by pathFromRoot() / name()
 */
class AutomationMap {
  public:
    using Entry = std::variant<int, wxWindow*, UIElement*>;

    void bind(const std::string& name, int id);
    void bind(const std::string& name, wxWindow* window);
    void bind(const std::string& name, UIElement* element);

    /** Bind element under pathFromRoot() and leaf name() (skips empty). */
    void bindElement(UIElement* element);
    /** Recursively bind element and all children. */
    void bindTree(UIElement* root);

    void unbind(const std::string& name);
    void clear();

    bool contains(const std::string& name) const;
    const Entry* find(const std::string& name) const;

    wxWindow* resolve(wxWindow* root, const std::string& nameOrId) const;
    UIElement* resolveElement(const std::string& nameOrPath) const;

    const std::unordered_map<std::string, Entry>& entries() const { return m_entries; }

  private:
    static wxWindow* findById(wxWindow* root, int id);
    static wxWindow* findByNameDeep(wxWindow* root, const std::string& name);
    static bool tryParseId(const std::string& text, int& outId);

    std::unordered_map<std::string, Entry> m_entries;
};

} // namespace bas::ui::automation

#endif
