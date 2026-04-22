#ifndef WX_TOOLBARS_HPP
#define WX_TOOLBARS_HPP

#include <wx/aui/auibar.h>
#include <wx/toolbar.h>

#include <iostream>

namespace wx {

void dumpAuiToolbar(wxAuiToolBar* toolbar, std::string prefix, std::ostream& out = std::cout);
void dumpToolbar(wxToolBar* toolbar, std::string prefix, std::ostream& out = std::cout);

wxAuiToolBarItem* addNecessarySeparator(wxAuiToolBar* toolbar);
wxToolBarToolBase* addNecessarySeparator(wxToolBar* toolbar);

} // namespace wx

#endif // WX_TOOLBARS_HPP