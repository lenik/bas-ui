#include "b-group.hpp"

#include "../../wx/toolbar.hpp"

#include <wx/aui/auibar.h>
#include <wx/toolbar.h>

void GroupVB::build(wxMenuBar* menubar) {
    int iconSize = context->preferredMenuIconSize();

    int menuPos = menubar->GetMenuCount();
    wxMenu* menu = new wxMenu();
    menubar->Append(menu, label);

    // menu icon not useful
    // if (icon.isSet())
    //     menu->SetBitmap(icon.loadBitmap(24, 24, wxART_MENU));

    std::string menuPath = group->path ? group->path->str() : group->dir();
    context->registerMenu(menuPath, menu);

    log(menubar, menuPos);
}

void GroupVB::build(wxMenu* menu) {
    int iconSize = context->preferredMenuIconSize();

    wxMenu* submenu = new wxMenu();
    wxMenuItem* item = menu->AppendSubMenu(submenu, label, help);

    if (icon.isSet()) {
        auto bmp = icon.toBitmap(iconSize, iconSize, wxART_MENU);
        if (bmp && bmp->IsOk())
            item->SetBitmap(*bmp);
    }

    std::string menuPath = group->path ? group->path->str() : group->dir();
    context->registerMenu(menuPath, submenu);

    log(menu, item->GetId(), item);
}

void GroupVB::build(wxToolBar* toolbar) {
    if (group->flattenActionCount() == 0)
        return;
    wxToolBarToolBase* sep = wx::ToolBar(toolbar).addNecessarySeparator_wx();
    if (!sep)
        return;
    log(toolbar, sep->GetId());
}

void GroupVB::build(wxAuiToolBar* toolbar) {
    int iconSize = context->preferredToolIconSize();

    // add necessary separators

    if (group->flattenActionCount() == 0)
        return;

    wxAuiToolBarItem* sep = wx::ToolBar(toolbar).addNecessarySeparator_aui();
    if (!sep)
        return;

    log(toolbar, sep->GetId());
}

void GroupVB::log(wxMenuBar* menubar, int menuPos) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::MENU;
    log->menuBar = menubar;
    log->menuPos = menuPos;
    log->group = group;
    logs->push_back(std::move(log));
}

void GroupVB::log(wxMenu* menu, int subMenuId, wxMenuItem* menuItem) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::SUBMENU;
    log->menu = menu;
    log->subMenuId = subMenuId;
    log->menuItem = menuItem;
    log->group = group;
    logs->push_back(std::move(log));
}

void GroupVB::log(wxToolBar* toolbar, int toolId) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::TOOLBAR_TOOL;
    log->toolbar = toolbar;
    log->toolId = toolId;
    logs->push_back(std::move(log));
}

void GroupVB::log(wxAuiToolBar* toolbar, int toolId) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::TOOLBAR_TOOL;
    log->auiToolbar = toolbar;
    log->toolId = toolId;
    logs->push_back(std::move(log));
}
