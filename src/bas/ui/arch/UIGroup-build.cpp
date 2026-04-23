
#include "BuildViewContext.hpp"
#include "UIAction.hpp"
#include "UIElement.hpp"
#include "UIGroup.hpp"
#include "UIState.hpp"

#include "b-action.hpp"
#include "b-group.hpp"
#include "b-state.hpp"

#include "../../wx/menus.hpp"
#include "../../wx/toolbars.hpp"

#include <wx/aui/auibar.h>
#include <wx/menu.h>
#include <wx/toolbar.h>

void UIGroup::buildView(BuildViewContext* context, BuildViewLogs* logs, //
                        std::optional<std::unordered_set<UIElement*>> white_set) {
    int menuIconSize = context->preferredMenuIconSize();
    int toolIconSize = context->preferredToolIconSize();

    int klast = -1;

    int child_index = -1;
    for (UIElement* child : children) {
        child_index++;
        if (white_set && white_set->find(child) == white_set->end()) {
            // ignore, but recursive into the group
            if (child->isGroup()) {
                UIGroup* gchild = dynamic_cast<UIGroup*>(child);
                if (!gchild)
                    continue;
                gchild->buildView(context, logs, white_set);
            }
            continue;
        }

        if (!child->visible.get())
            continue;
        std::string dir = child->dir();

        wxString label = child->label.get().empty() ? child->name() : child->label.get();
        wxString help = child->description.get();

        ImageSet icon = child->icon.get();

        std::vector<wxMenuBar*> parentMenubars = context->getMenubars(dir);
        std::vector<wxMenu*> parentMenus = context->getMenus(dir);
        std::vector<wxAuiToolBar*> parentAuiToolbars;
        std::vector<wxToolBar*> parentToolbars;
        if (context->isAuiPreferred())
            parentAuiToolbars = context->getAuiToolbars(dir);
        else
            parentToolbars = context->getToolbars(dir);

        int priority = child->priority.get();
        int kgroup = priority / 1000;
        if (kgroup != klast) {
            if (klast != -1) {
                // add separator between k-groups
                // if (child->menuWanted())
                //     for (wxMenuBar* parentMenubar : parentMenubars) {
                //         wx::addNecessarySeparator(parentMenubar);
                //     }
                if (child->menuWanted())
                    for (wxMenu* parentMenu : parentMenus) {
                        wx::addNecessarySeparator(parentMenu);
                    }
                if (child->toolWanted() && context->isAuiPreferred())
                    for (wxAuiToolBar* parentAuiToolbar : parentAuiToolbars) {
                        wx::addNecessarySeparator(parentAuiToolbar);
                    }
                if (child->toolWanted() && !context->isAuiPreferred())
                    for (wxToolBar* parentToolbar : parentToolbars) {
                        wx::addNecessarySeparator(parentToolbar);
                    }
            }
            klast = kgroup;
        }

        if (child == nullptr) {
            logerror_fmt("child become nullptr in group %s[%d]", str().c_str(), child_index);
            continue;
        }
        if (child->isGroup()) {
            UIGroup* gchild = dynamic_cast<UIGroup*>(child);
            if (!gchild)
                continue;

            if (gchild->menuWanted())
                for (wxMenuBar* parentMenubar : parentMenubars)
                    GroupVB(gchild, context, logs).build(parentMenubar);

            if (gchild->menuWanted())
                for (wxMenu* parentMenu : parentMenus)
                    GroupVB(gchild, context, logs).build(parentMenu);

            if (gchild->toolWanted() && gchild->flattenActionCount() > 0) {
                if (context->isAuiPreferred()) {
                    for (wxAuiToolBar* parentAuiToolbar : parentAuiToolbars)
                        GroupVB(gchild, context, logs).build(parentAuiToolbar);
                } else {
                    for (wxToolBar* parentToolbar : parentToolbars)
                        GroupVB(gchild, context, logs).build(parentToolbar);
                }
            }

            // recursively set up the children
            gchild->buildView(context, logs);
        }

        else if (child->isAction()) {
            UIAction* achild = dynamic_cast<UIAction*>(child);
            if (!achild)
                continue;

            if (achild->menuWanted())
                for (wxMenu* m : parentMenus)
                    ActionVB(achild, context, logs).build(m);

            if (achild->toolWanted()) {
                if (context->isAuiPreferred()) {
                    for (wxAuiToolBar* tb : parentAuiToolbars)
                        ActionVB(achild, context, logs).build(tb);
                } else {
                    for (wxToolBar* tb : parentToolbars)
                        ActionVB(achild, context, logs).build(tb);
                }
            }
        }

        else if (child->isState()) {
            UIState* stchild = dynamic_cast<UIState*>(child);
            if (!stchild)
                continue;

            if (stchild->menuWanted())
                for (wxMenu* m : parentMenus)
                    StateVB(stchild, context, logs).build(m);

            if (stchild->toolWanted()) {
                if (context->isAuiPreferred()) {
                    for (wxAuiToolBar* tb : parentAuiToolbars)
                        StateVB(stchild, context, logs).build(tb);
                } else {
                    for (wxToolBar* tb : parentToolbars)
                        StateVB(stchild, context, logs).build(tb);
                }
            }
        }
    }
}
