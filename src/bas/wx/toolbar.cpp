#include "toolbar.hpp"

namespace wx {

wxAuiToolBarItem* ToolBar::addNecessarySeparator_aui() {
    // if the last tool isn't separator
    int toolCount = static_cast<int>(aui->GetToolCount());
    if (toolCount == 0)
        return nullptr;

    auto last = aui->FindToolByIndex(toolCount - 1);
    if (last->GetKind() == wxITEM_SEPARATOR)
        return nullptr;

    auto sep = aui->AddSeparator();
    return sep;
}

wxToolBarToolBase* ToolBar::addNecessarySeparator_wx() {
    int toolCount = wx->GetToolsCount();
    if (toolCount == 0)
        return nullptr;

    auto last = wx->GetToolByPos(toolCount - 1);
    if (last->IsSeparator())
        return nullptr;

    return wx->AddSeparator();
}

void ToolBar::dump(std::string prefix, std::ostream& out) {
    if (isAui()) {
        for (size_t i = 0; i < aui->GetToolCount(); ++i) {
            const wxAuiToolBarItem* tool = aui->FindToolByIndex(i);
            if (tool) {
                out << prefix << tool->GetLabel() << std::endl;
            }
        }
    } else {
        for (size_t i = 0; i < wx->GetToolsCount(); ++i) {
            const wxToolBarToolBase* tool = wx->GetToolByPos(i);
            if (tool) {
                out << prefix << tool->GetLabel() << std::endl;
            }
        }
    }
}

} // namespace wx