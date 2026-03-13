#ifndef WX_TOOLBARS_HPP
#define WX_TOOLBARS_HPP

#include <wx/toolbar.h>

#include <iostream>

namespace wx {

void dumpToolbar(wxToolBar* toolbar, std::string prefix, std::ostream& out = std::cout);

wxToolBarToolBase* addNecessarySeparator(wxToolBar* toolbar);

} // namespace wx

#endif // WX_TOOLBARS_HPP