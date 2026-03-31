#ifndef UI_BUILD_VIEW_LOG_HPP
#define UI_BUILD_VIEW_LOG_HPP

#include "BuildViewContext.hpp"

#include <wx/menu.h>
#include <wx/toolbar.h>

#include <iostream>
#include <memory>
#include <vector>

class BuildViewContext;
class UIGroup;

struct BuildViewLog {
    virtual ~BuildViewLog();

    enum Kind { MENU, SUBMENU, MENU_ITEM, TOOLBAR_TOOL };
    Kind kind{MENU_ITEM};
    wxMenuBar* menuBar{nullptr};
    int menuPos{-1};

    wxMenu* menu{nullptr};
    int subMenuId{-1};
    wxMenuItem* menuItem{nullptr};

    wxToolBar* toolbar{nullptr};
    int toolId{-1};
    
    UIGroup* group{nullptr};
};

struct BuildViewLogs
    : public std::vector<std::unique_ptr<BuildViewLog>>
{
    /** Dump install records to output stream */
    void dump(std::ostream& output = std::cout) const;
};

#endif // UI_BUILD_VIEW_LOG_HPP
