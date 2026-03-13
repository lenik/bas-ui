#ifndef UI_BUILD_VIEW_CONTEXT_H
#define UI_BUILD_VIEW_CONTEXT_H

#include "wx/gdicmn.h"
#include "wx/gtk/window.h"
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
class BuildViewContext {
  public:
    virtual ~BuildViewContext() = default;

    std::vector<wxMenuBar*> getMenubars(std::string_view path);
    std::vector<wxMenu*> getMenus(std::string_view path);
    std::vector<wxToolBar*> getToolbars(std::string_view path);

    void registerMenubar(std::string_view path, wxMenuBar* menubar);
    void registerMenu(std::string_view path, wxMenu* menu);
    void registerToolbar(std::string_view path, wxToolBar* toolbar);

    int preferredMenuIconSize() const { return 16; }
    int preferredToolIconSize() const { return 32; }
    bool toolbarSmallSize() const { return false; }
    bool toolbarShowLabel() const { return false; }

  private:

    std::unordered_map<std::string, std::vector<wxMenuBar*>> m_menubars;
    std::unordered_map<std::string, std::vector<wxMenu*>> m_menus;
    std::unordered_map<std::string, std::vector<wxToolBar*>> m_toolbars;
};

#endif // UI_BUILD_VIEW_CONTEXT_H
