#include "toolbars.hpp"

#include <wx/aui/auibar.h>
#include <wx/toolbar.h>

namespace wx {

void dumpAuiToolbar(wxAuiToolBar* toolbar, std::string prefix, std::ostream& out) {
    for (size_t i = 0; i < toolbar->GetToolCount(); ++i) {
        const wxAuiToolBarItem* tool = toolbar->FindToolByIndex(i);
        if (tool) {
            out << prefix << tool->GetLabel() << std::endl;
        }
    }
}

void dumpToolbar(wxToolBar* toolbar, std::string prefix, std::ostream& out) {
    for (size_t i = 0; i < toolbar->GetToolsCount(); ++i) {
        const wxToolBarToolBase* tool = toolbar->GetToolByPos(i);
        if (tool) {
            out << prefix << tool->GetLabel() << std::endl;
        }
    }
}

wxAuiToolBarItem* addNecessarySeparator(wxAuiToolBar* toolbar) {
    // if the last tool isn't separator
    int toolCount = static_cast<int>(toolbar->GetToolCount());
    if (toolCount == 0)
        return nullptr;

    auto last = toolbar->FindToolByIndex(toolCount - 1);
    if (last->GetKind() == wxITEM_SEPARATOR)
        return nullptr;

    auto sep = toolbar->AddSeparator();
    return sep;
}

wxToolBarToolBase* addNecessarySeparator(wxToolBar* toolbar) {
    int toolCount = toolbar->GetToolsCount();
    if (toolCount == 0)
        return nullptr;
    auto last = toolbar->GetToolByPos(toolCount - 1);
    if (last->IsSeparator())
        return nullptr;
    return toolbar->AddSeparator();
}

} // namespace wx
