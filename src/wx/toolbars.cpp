#include "toolbars.hpp"

#include <wx/toolbar.h>

namespace wx {

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
