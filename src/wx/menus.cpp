#include "menus.hpp"

#include <wx/menu.h>

namespace wx {

void dumpMenubar(wxMenuBar* menubar, std::string prefix, std::ostream& out) {
    for (int i = 0; i < menubar->GetMenuCount(); i++) {
        wxMenu* menu = menubar->GetMenu(i);
        wxString menuLabel = menubar->GetMenuLabel(i);
        out << prefix << "Menu: " << menuLabel << std::endl;
        for (int j = 0; j < menu->GetMenuItemCount(); j++) {
            wxMenuItem* item = menu->FindItemByPosition(j);
            if (item) {
                out << prefix << "  Item: " << item->GetLabel() << std::endl;
                if (item->IsSubMenu()) {
                    dumpMenu(item->GetSubMenu(), prefix + "    ", out);
                }
            }
        }
    }
}

void dumpMenu(wxMenu* menu, std::string prefix, std::ostream& out) {
    for (int j = 0; j < menu->GetMenuItemCount(); j++) {
        wxMenuItem* item = menu->FindItemByPosition(j);
        if (item) {
            out << prefix << "Item: " << item->GetLabel() << std::endl;
            if (item->GetSubMenu()) {
                dumpMenu(item->GetSubMenu(), prefix + "  ", out);
            }
        }
    }
}

bool isSeparator(wxMenuBar* menubar, int pos) {
    auto label = menubar->GetMenuLabelText(pos);
    return label == "|";
}

wxMenu* addNecessarySeparator(wxMenuBar* menuBar) {
    int menuCount = menuBar->GetMenuCount();
    if (menuCount == 0)
        return nullptr;

    if (isSeparator(menuBar, menuCount - 1))
        return nullptr;

    wxMenu* menu = new wxMenu();

    menuBar->Append(menu, "|");
    return menu;
}

wxMenuItem* addNecessarySeparator(wxMenu* menu) {
    int menuCount = menu->GetMenuItemCount();
    if (menuCount == 0)
        return nullptr;

    auto last = menu->FindItemByPosition(menuCount - 1);
    if (last->IsSeparator())
        return nullptr;

    auto sep = new wxMenuItem(menu, wxID_SEPARATOR, "");
    menu->Append(sep);
    return sep;
}

} // namespace wx
