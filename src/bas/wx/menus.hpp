#ifndef WX_MENUS_HPP
#define WX_MENUS_HPP

#include <wx/menu.h>

#include <iostream>

namespace wx {

void dumpMenubar(wxMenuBar* menubar, std::string prefix, std::ostream& out = std::cout);
void dumpMenu(wxMenu* menu, std::string prefix, std::ostream& out = std::cout);

wxMenu* addNecessarySeparator(wxMenuBar* menuBar);
wxMenuItem* addNecessarySeparator(wxMenu* menu);

} // namespace wx

#endif // WX_MENUS_HPP
