#include "UIGroup.hpp"

#include "BuildViewContext.hpp"
#include "BuildViewLog.hpp"
#include "UIAction.hpp"
#include "UIState.hpp"

#include "ui/arch/ImageSet.hpp"
#include "wx/menus.hpp"
#include "wx/toolbars.hpp"

#include <bas/log/uselog.h>

#include <wx/bitmap.h>
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/toolbar.h>

#include <memory>
#include <string>
#include <vector>

void buildGroupView(UIGroup* group, wxMenuBar* menubar, BuildViewContext* context,
                    BuildViewLogs* logs) {
    wxString label(group->label.get().empty() ? group->name() : group->label.get());
    wxString help(group->description.get());

    // ImageSet icon = group->icon.get();
    // int iconSize = context->preferredMenuIconSize();

    int menuPos = menubar->GetMenuCount();
    wxMenu* menu = new wxMenu();
    menubar->Append(menu, label);

    // menu icon not useful
    // if (icon.isSet())
    //     menu->SetBitmap(icon.loadBitmap(24, 24));

    std::string menuPath = group->path ? group->path->str() : group->dir();
    context->registerMenu(menuPath, menu);

    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::MENU;
    log->menuBar = menubar;
    log->menuPos = menuPos;
    log->group = group;
    logs->push_back(std::move(log));
}

void buildGroupView(UIGroup* group, wxMenu* menu, BuildViewContext* context, BuildViewLogs* logs) {
    wxString label(group->label.get().empty() ? group->name() : group->label.get());
    wxString help(group->description.get());

    ImageSet icon = group->icon.get();
    int iconSize = context->preferredMenuIconSize();

    wxMenu* submenu = new wxMenu();
    wxMenuItem* item = menu->AppendSubMenu(submenu, label, help);

    if (icon.isSet()) {
        wxBitmap bmp = icon.loadBitmap(iconSize, iconSize);
        if (bmp.IsOk())
            item->SetBitmap(bmp);
    }

    std::string menuPath = group->path ? group->path->str() : group->dir();
    context->registerMenu(menuPath, submenu);

    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::SUBMENU;
    log->menu = menu;
    log->subMenuId = item->GetId();
    log->menuItem = item;
    log->group = group;
    logs->push_back(std::move(log));
}

void buildGroupView(UIGroup* group, wxToolBar* toolbar, BuildViewContext* context,
                    BuildViewLogs* logs) {
    wxString label(group->label.get().empty() ? group->name() : group->label.get());
    wxString help(group->description.get());

    // ImageSet icon = group->icon.get();
    // int iconSize = context->preferredToolIconSize();

    // add necessary separators

    if (group->flattenActionCount() == 0)
        return;

    wxToolBarToolBase* sep = wx::addNecessarySeparator(toolbar);
    if (!sep)
        return;

    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::TOOLBAR_TOOL;
    log->toolbar = toolbar;
    log->toolId = sep->GetId();
    logs->push_back(std::move(log));
}

void buildActionView(UIAction* action, wxMenu* menu, BuildViewContext* context,
                     BuildViewLogs* logs) {
    wxString label(action->label.get().empty() ? action->name() : action->label.get());
    wxString help(action->description.get());

    ImageSet icon = action->icon.get();
    int iconSize = context->preferredMenuIconSize();

    wxMenuItem* item = menu->Append(action->id, label, help);
    if (icon.isSet()) {
        wxBitmap bmp = icon.loadBitmap(iconSize, iconSize);
        if (bmp.IsOk())
            item->SetBitmap(bmp);
    }

    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::MENU_ITEM;
    log->menu = menu;
    log->menuItem = item;
    logs->push_back(std::move(log));
}

void buildActionView(UIAction* action, wxToolBar* toolbar, BuildViewContext* context,
                     BuildViewLogs* logs) {
    wxString label(action->label.get().empty() ? action->name() : action->label.get());
    wxString help(action->description.get());

    ImageSet icon = action->icon.get();
    int iconSize = context->preferredToolIconSize();

    wxBitmap bmp = icon.loadBitmap(iconSize, iconSize);
    wxString toolLabel = label;
    toolLabel.Replace("&", "");

    // auto tool =
    toolbar->AddTool(action->id, toolLabel, bmp, help, //
                     wxITEM_NORMAL);

    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::TOOLBAR_TOOL;
    log->toolbar = toolbar;
    log->toolId = action->id;
    logs->push_back(std::move(log));
}

void buildStateView(UIState* state, wxMenu* menu, BuildViewContext* context, BuildViewLogs* logs) {
    wxString label(state->label.get().empty() ? state->name() : state->label.get());
    wxString help(state->description.get());

    if (state->getType() == UIStateType::BOOL) {
        bool checked = false;
        if (auto* p = std::get_if<bool>(&state->value.get()))
            checked = *p;

        wxMenuItem* item = menu->AppendCheckItem(state->id, label, help);
        // check item can't have bitmap
        menu->Check(state->id, checked);
        auto log = std::make_unique<BuildViewLog>();
        log->kind = BuildViewLog::MENU_ITEM;
        log->menu = menu;
        log->menuItem = item;
        logs->push_back(std::move(log));
    }

    else if (state->getType() == UIStateType::ENUM) {
        const int enumCount = state->getEnumCount();

        wxMenu* submenu = new wxMenu();
        for (int v = 0; v < enumCount; ++v) {
            UIStateValueDescriptor d = state->getValueDescriptor(v);
            if (d.label.empty())
                break;
            submenu->AppendRadioItem(wxID_ANY, d.label, d.description);
        }
        wxMenuItem* item = menu->Append(state->id, label, submenu, help);
        auto log = std::make_unique<BuildViewLog>();
        log->kind = BuildViewLog::SUBMENU;
        log->menu = menu;
        log->menuItem = item;
        logs->push_back(std::move(log));
    }
}

void buildStateView(UIState* state, wxToolBar* toolbar, BuildViewContext* context,
                    BuildViewLogs* logs) {
    wxString label(state->label.get().empty() ? state->name() : state->label.get());
    wxString help(state->description.get());

    ImageSet icon = state->icon.get();
    int toolIconSize = context->preferredToolIconSize();

    if (state->getType() == UIStateType::BOOL) {
        bool checked = false;
        if (auto* p = std::get_if<bool>(&state->value.get()))
            checked = *p;

        wxBitmap bmp;
        if (icon.isSet()) {
            bmp = icon.loadBitmap(toolIconSize, toolIconSize);
        } else {
            wxSize size(toolIconSize, toolIconSize);
            bmp = wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, //
                                           wxART_TOOLBAR, size);
        }

        toolbar->AddTool(state->id, label, bmp, help, wxITEM_CHECK);
        auto log = std::make_unique<BuildViewLog>();
        log->kind = BuildViewLog::TOOLBAR_TOOL;
        log->toolbar = toolbar;
        log->toolId = state->id;
        logs->push_back(std::move(log));
    }

    else if (state->getType() == UIStateType::ENUM) {
        const int enumCount = state->getEnumCount();

        for (int v = 0; v < enumCount; ++v) {
            int toolId = state->id * 1000 + v;
            UIStateValueDescriptor d = state->getValueDescriptor(v);
            if (d.label.empty())
                break;
            toolbar->AddTool(toolId + v, d.label, wxBitmap(), d.description, wxITEM_RADIO);
            auto log = std::make_unique<BuildViewLog>();
            log->kind = BuildViewLog::TOOLBAR_TOOL;
            log->toolbar = toolbar;
            log->toolId = toolId;
            logs->push_back(std::move(log));
        }
    }
}

void UIGroup::buildView(BuildViewContext* context, BuildViewLogs* logs) {
    int menuIconSize = context->preferredMenuIconSize();
    int toolIconSize = context->preferredToolIconSize();

    int klast = -1;

    for (UIElement* child : children) {
        if (!child->visible.get())
            continue;
        std::string dir = child->dir();

        wxString label(child->label.get().empty() ? child->name() : child->label.get());
        wxString help(child->description.get());

        ImageSet icon = child->icon.get();

        std::vector<wxMenuBar*> parentMenubars = context->getMenubars(dir);
        std::vector<wxMenu*> parentMenus = context->getMenus(dir);
        std::vector<wxToolBar*> parentToolbars = context->getToolbars(dir);

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
                if (child->toolWanted())
                    for (wxToolBar* parentToolbar : parentToolbars) {
                        wx::addNecessarySeparator(parentToolbar);
                    }
            }
            klast = kgroup;
        }

        if (child->isGroup()) {
            UIGroup* gchild = dynamic_cast<UIGroup*>(child);
            if (!gchild)
                continue;

            if (gchild->menuWanted())
                for (wxMenuBar* parentMenubar : parentMenubars)
                    buildGroupView(gchild, parentMenubar, context, logs);

            if (gchild->menuWanted())
                for (wxMenu* parentMenu : parentMenus)
                    buildGroupView(gchild, parentMenu, context, logs);

            if (gchild->toolWanted() &&
                gchild->flattenActionCount() > 0) // add necessary separators
                for (wxToolBar* parentToolbar : parentToolbars)
                    buildGroupView(gchild, parentToolbar, context, logs);

            // recursively set up the children
            gchild->buildView(context, logs);
        }

        else if (child->isAction()) {
            UIAction* achild = dynamic_cast<UIAction*>(child);
            if (!achild)
                continue;

            if (achild->menuWanted())
                for (wxMenu* m : parentMenus)
                    buildActionView(achild, m, context, logs);

            if (achild->toolWanted())
                for (wxToolBar* tb : parentToolbars)
                    buildActionView(achild, tb, context, logs);
        }

        else if (child->isState()) {
            UIState* stchild = dynamic_cast<UIState*>(child);
            if (!stchild)
                continue;

            if (stchild->menuWanted())
                for (wxMenu* m : parentMenus)
                    buildStateView(stchild, m, context, logs);

            if (stchild->toolWanted())
                for (wxToolBar* tb : parentToolbars)
                    buildStateView(stchild, tb, context, logs);
        }
    }
}
