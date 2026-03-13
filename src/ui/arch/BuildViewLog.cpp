#include "BuildViewLog.hpp"

#include <wx/menu.h>
#include <wx/toolbar.h>

#include <iostream>
#include <vector>

BuildViewLog::~BuildViewLog() {}

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
