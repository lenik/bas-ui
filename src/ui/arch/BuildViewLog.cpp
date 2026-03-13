#include "BuildViewLog.hpp"

#include <wx/menu.h>
#include <wx/toolbar.h>

#include <iostream>
#include <vector>

BuildViewLog::~BuildViewLog() {
    switch (kind) {
    case MENU:
        if (menuBar && menuPos >= 0 && menu) {
            // Only remove if the menu is still at the expected position
            // Menu might have been already removed by tearDown() or wxWidgets cleanup
            int currentMenuCount = menuBar->GetMenuCount();
            if (menuPos < currentMenuCount) {
                wxMenu* _menu = menuBar->Remove(menuPos);
                // Only delete if we got the expected menu back
                if (_menu == menu) {
                    delete _menu;
                }
            }
            menu = nullptr;
            menuPos = -1;
        }
        break;

    case SUBMENU:
        if (menu && subMenuId >= 0 && menuItem) {
            // Only remove if the menu item still exists
            wxMenuItem* subMenuItem = menu->Remove(subMenuId);
            // Only delete if we got the expected item back
            if (subMenuItem == menuItem) {
                delete subMenuItem; // auto delete submenu
            }
            menuItem = nullptr;
            subMenuId = -1;
        }
        break;

    case MENU_ITEM:
        if (menu && menuItem) {
            // Menu item might have been already removed by tearDown()
            // Check if it still exists in the menu before removing
            bool found = false;
            for (int i = 0; i < menu->GetMenuItemCount(); ++i) {
                if (menu->FindItemByPosition(i) == menuItem) {
                    found = true;
                    break;
                }
            }
            if (found) {
                menu->Remove(menuItem);
                delete menuItem;
            }
            menuItem = nullptr;
        }
        break;

    case TOOLBAR_TOOL:
        if (toolbar && toolId >= 0) {
            // Tool might have been already removed
            // Check if it still exists before removing
            wxToolBarToolBase* tool = toolbar->FindById(toolId);
            if (tool) {
                toolbar->RemoveTool(toolId);
            }
            toolId = -1;
        }
        break;
    default:
        break;
    }
}

void BuildViewLogs::dump(std::ostream& output) const {
    output << "BuildViewLogs (" << size() << " items):\n";
    for (size_t i = 0; i < size(); ++i) {
        const BuildViewLog* rec = (*this)[i].get();
        output << "  [" << i << "] ";

        switch (rec->kind) {
        case BuildViewLog::MENU:
            output << "MENU";
            if (rec->menuBar && rec->menuPos >= 0) {
                output << " (menuBar=" << rec->menuBar << ", pos=" << rec->menuPos << ")";
            }
            break;
        case BuildViewLog::SUBMENU:
            output << "SUBMENU";
            if (rec->menu && rec->subMenuId >= 0) {
                output << " (menu=" << rec->menu << ", id=" << rec->subMenuId << ")";
            }
            break;
        case BuildViewLog::MENU_ITEM:
            output << "MENU_ITEM";
            if (rec->menu && rec->menuItem) {
                output << " (menu=" << rec->menu << ", item=" << rec->menuItem << ")";
            }
            break;
        case BuildViewLog::TOOLBAR_TOOL:
            output << "TOOLBAR_TOOL";
            if (rec->toolbar && rec->toolId >= 0) {
                output << " (toolbar=" << rec->toolbar << ", id=" << rec->toolId << ")";
            }
            break;
        default:
            output << "UNKNOWN";
            break;
        }

        if (rec->group) {
            output << " -> group=" << rec->group;
        }

        output << "\n";
    }
}
