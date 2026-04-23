#ifndef GROUP_VB_HPP
#define GROUP_VB_HPP

#include "BuildViewContext.hpp"
#include "BuildViewLog.hpp"
#include "ImageSet.hpp"
#include "UIGroup.hpp"

#include <wx/aui/auibar.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/string.h>
#include <wx/toolbar.h>

class GroupVB {
  public:
    GroupVB(UIGroup* group, BuildViewContext* context, BuildViewLogs* logs)
        : group(group), context(context), logs(logs) {
        label = group->label.get().empty() ? group->name() : group->label.get();
        help = group->description.get();
        icon = group->icon.get();
    }

    void build(wxMenuBar* menubar);
    void build(wxMenu* menu);
    void build(wxToolBar* toolbar);
    void build(wxAuiToolBar* toolbar);

    void log(wxMenuBar* menubar, int menuPos);
    void log(wxMenu* menu, int subMenuId, wxMenuItem* menuItem);
    void log(wxToolBar* toolbar, int toolId);
    void log(wxAuiToolBar* toolbar, int toolId);

  private:
    UIGroup* group{nullptr};
    BuildViewContext* context{nullptr};
    BuildViewLogs* logs{nullptr};

    wxString label;
    wxString help;
    ImageSet icon;
};

#endif // GROUP_VB_HPP