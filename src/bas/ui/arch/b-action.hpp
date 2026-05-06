#ifndef ACTION_VB_HPP
#define ACTION_VB_HPP

#include "BuildViewContext.hpp"
#include "BuildViewLog.hpp"
#include "ImageSet.hpp"
#include "UIAction.hpp"

#include <wx/aui/auibar.h>
#include <wx/bitmap.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/string.h>
#include <wx/toolbar.h>

class ActionVB {
  public:
    ActionVB(UIAction* action, BuildViewContext* context, BuildViewLogs* logs)
        : action(action), context(context), logs(logs) {
        label = action->label.get().empty() ? action->name() : action->label.get();
        shortHelp = action->description.get();
        icon = action->icon.get();
    }

    void build(wxMenu* menu);
    void build(wxToolBar* toolbar);
    void build(wxAuiToolBar* toolbar);

    void log(wxMenu* menu, wxMenuItem* item);
    void log(wxToolBar* toolbar, int toolId);
    void log(wxAuiToolBar* toolbar, int toolId);

  private:
    UIAction* action{nullptr};
    BuildViewContext* context{nullptr};
    BuildViewLogs* logs{nullptr};

    wxString label;
    wxString shortHelp;
    ImageSet icon;
};

#endif // ACTION_VB_HPP