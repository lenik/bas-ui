#include "toolbars.hpp"

#include <wx/toolbar.h>

namespace wx {

void dumpToolbar(wxToolBar* toolbar, std::string prefix, std::ostream& out) {
    for (size_t i = 0; i < toolbar->GetToolsCount(); ++i) {
        const wxToolBarToolBase* tool = toolbar->GetToolByPos(i);
        if (tool) {
            out << prefix << tool->GetLabel() << std::endl;
        }
    }
}

wxToolBarToolBase* addNecessarySeparator(wxToolBar* toolbar) {
    // if the last tool isn't separator
    int toolCount = toolbar->GetToolsCount();
    if (toolCount == 0)
        return nullptr;

    auto last = toolbar->GetToolByPos(toolCount - 1);
    if (last->IsSeparator())
        return nullptr;

    auto sep = toolbar->AddSeparator();
    return sep;
}

} // namespace wx
