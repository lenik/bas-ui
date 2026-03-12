#ifndef WX_MENUS_HPP
#define WX_MENUS_HPP

#include <wx/menu.h>

namespace wx {

wxMenu* addNecessarySeparator(wxMenuBar* menuBar);
wxMenuItem* addNecessarySeparator(wxMenu* menu);

} // namespace wx

#endif // WX_MENUS_HPP
