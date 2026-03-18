#include "UIGroup.hpp"

#include "BuildViewContext.hpp"
#include "BuildViewLog.hpp"
#include "UIAction.hpp"
#include "UIState.hpp"

#include <bas/log/uselog.h>

#include <wx/bitmap.h>
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/toolbar.h>

#include <memory>
#include <string>
#include <vector>

void removeGroup(UIGroup* group, wxMenuBar* menubar) {
    int menuPos = menubar->FindMenu(group->label.get());
    if (menuPos >= 0) {
        wxMenu* menu = menubar->Remove(menuPos);
        delete menu;
    }
}

void removeGroup(UIGroup* group, wxMenu* menu) {
    wxMenuItem* subMenuItem = menu->FindChildItem(group->id);
    if (subMenuItem) {
        menu->Remove(subMenuItem);
        delete subMenuItem;
    }
}

void removeGroup(UIGroup* group, wxToolBar* toolbar) {
    wxToolBarToolBase* tool = toolbar->FindById(group->id);
    if (tool) {
        toolbar->RemoveTool(tool->GetId());
        delete tool;
    }
}

void removeAction(UIElement* action, wxMenu* menu) {
    wxMenuItem* item = menu->FindChildItem(action->id);
    if (item) {
        menu->Remove(item);
        delete item;
    }
}

void removeAction(UIElement* action, wxToolBar* toolbar) {
    wxToolBarToolBase* tool = toolbar->FindById(action->id);
    if (tool) {
        toolbar->RemoveTool(tool->GetId());
        delete tool;
    }
}

void removeState(UIState* state, wxMenu* menu) {
    if (state->getType() == UIStateType::BOOL) {
        // find menuitem matching el
        wxMenuItem* item;
        for (int i = 0; i < menu->GetMenuItemCount(); ++i) {
            size_t pos = -1;
            wxMenuItem* item = menu->FindChildItem(state->id, &pos);
            if (item) {
                menu->Remove(item);
                delete item;
                break;
            }
        }

    } else if (state->getType() == UIStateType::ENUM) {
        size_t pos = -1;
        wxMenuItem* item = menu->FindChildItem(state->id, &pos);
        if (item) {
            // wxMenu* submenu = item->GetSubMenu();
            menu->Remove(item);
            delete item;
        }
    }
}

void removeState(UIState* state, wxToolBar* toolbar) {
    if (state->getType() == UIStateType::BOOL) {
        wxToolBarToolBase* tool = toolbar->FindById(state->id);
        if (tool) {
            toolbar->RemoveTool(tool->GetId());
            delete tool;
        }
    }

    else if (state->getType() == UIStateType::ENUM) {
        const std::vector<int> enumValues = state->getEnumValues();
        for (int v : enumValues) {
            UIStateValueDescriptor d = state->getValueDescriptor(v);
            int toolId = d.id(nullptr);
            toolbar->RemoveTool(toolId);
        }
    }
}

void UIGroup::removeBuild(BuildViewContext* context, //
                          std::optional<std::unordered_set<UIElement*>> white_set) {
    for (UIElement* child : children) {

        if (white_set && white_set->find(child) == white_set->end()) {
            // ignore, but recursive into the group
            if (child->isGroup()) {
                UIGroup* gchild = dynamic_cast<UIGroup*>(child);
                if (!gchild)
                    continue;
                gchild->removeBuild(context, white_set);
            }
            continue;
        }

        if (!child->visible.get())
            continue;
        std::string dir = child->dir();

        wxString label(child->label.get().empty() ? child->name() : child->label.get());
        wxString help(child->description.get());

        ImageSet icon = child->icon.get();

        std::vector<wxMenuBar*> menubars = context->getMenubars(dir);
        std::vector<wxMenu*> menus = context->getMenus(dir);
        std::vector<wxToolBar*> toolbars = context->getToolbars(dir);

        if (child->isGroup()) {
            UIGroup* gchild = dynamic_cast<UIGroup*>(child);
            if (!gchild)
                continue;
            gchild->removeBuild(context);

            if (gchild->menuWanted())
                for (wxMenuBar* mb : menubars)
                    removeGroup(gchild, mb);

            if (gchild->menuWanted())
                for (wxMenu* m : menus)
                    removeGroup(gchild, m);

            if (gchild->toolWanted())
                for (wxToolBar* tb : toolbars)
                    removeGroup(gchild, tb);
        }

        else if (child->isAction()) {
            UIAction* achild = dynamic_cast<UIAction*>(child);
            if (!achild)
                continue;

            if (child->menuWanted())
                for (wxMenu* m : menus)
                    removeAction(achild, m);

            if (child->toolWanted())
                for (wxToolBar* tb : toolbars)
                    removeAction(achild, tb);
        }

        else if (child->isState()) {
            UIState* stchild = dynamic_cast<UIState*>(child);
            if (!stchild)
                continue;

            if (stchild->menuWanted())
                for (wxMenu* m : menus)
                    removeState(stchild, m);

            if (stchild->toolWanted())
                for (wxToolBar* tb : toolbars)
                    removeState(stchild, tb);
        }
    }
}

void UIGroup::removeBuild(BuildViewLogs* logs) {
    for (auto it = logs->rbegin(); it != logs->rend(); ++it) {
        BuildViewLog* log = it->get();

        switch (log->kind) {
        case BuildViewLog::MENU:
            if (log->menuBar && log->menuPos >= 0) {
                wxMenu* _menu = log->menuBar->Remove(log->menuPos);
                assert(_menu && _menu == log->menu);
                delete _menu;
                log->menu = nullptr;
                log->menuPos = -1;
            }
            break;
        case BuildViewLog::SUBMENU:
            if (log->menu && log->subMenuId >= 0) {
                wxMenuItem* subMenuItem = log->menu->Remove(log->subMenuId);
                assert(subMenuItem && subMenuItem == log->menuItem);
                delete subMenuItem; // auto delete submenu
                log->menuItem = nullptr;
                log->subMenuId = -1;
            }
            break;
        case BuildViewLog::MENU_ITEM:
            if (log->menu && log->menuItem) {
                log->menu->Remove(log->menuItem);
                delete log->menuItem;
                log->menuItem = nullptr;
            }
            break;
        case BuildViewLog::TOOLBAR_TOOL:
            if (log->toolbar && log->toolId >= 0) {
                log->toolbar->RemoveTool(log->toolId);
                log->toolId = -1;
            }
            break;
        default:
            break;
        }
    }
    logs->clear();
}
