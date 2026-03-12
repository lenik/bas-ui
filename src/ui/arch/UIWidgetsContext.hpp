#ifndef UI_WIDGETS_CONTEXT_H
#define UI_WIDGETS_CONTEXT_H

#include <bas/util/Path.hpp>

#include <string>
#include <unordered_map>
#include <vector>

class wxMenuBar;
class wxMenu;
class wxToolBar;

/**
 * Context that provides wx menus and toolbars by path (element.dir()).
 * Used by UIGroup::setUp() to install group/action/state into the right menus/toolbars.
 */
class UIWidgetsContext {
public:
    virtual ~UIWidgetsContext() = default;

    std::vector<wxMenuBar*> getMenubars(std::string_view path);
    std::vector<wxMenu*> getMenus(std::string_view path);
    std::vector<wxToolBar*> getToolbars(std::string_view path);

    void registerMenubar(std::string_view path, wxMenuBar* menubar);
    void registerMenu(std::string_view path, wxMenu* menu);
    void registerToolbar(std::string_view path, wxToolBar* toolbar);

    int preferredMenuIconSize() const { return 16; }
    int preferredToolIconSize() const { return 32; }
    
private:
    std::unordered_map<std::string, std::vector<wxMenuBar*>> m_menubars;
    std::unordered_map<std::string, std::vector<wxMenu*>> m_menus;
    std::unordered_map<std::string, std::vector<wxToolBar*>> m_toolbars;
};

#endif // UI_WIDGETS_CONTEXT_H
