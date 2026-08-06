#include "b-action.hpp"

#include "../../wx/wx_compat.hpp"
#include "UIAction.hpp"

void ActionVB::build(wxMenu* menu) {
    auto& shortcuts = action->getShortcuts();
    if (!shortcuts.empty()) {
        basWxAppendMenuShortcut(label, shortcuts[0]);
    }

    auto id = action->id;

    if (id < 0 || id > SHRT_MAX || id >= wxID_AUTO_LOWEST && id <= wxID_AUTO_HIGHEST) {
        std::cerr << "SHRT_MAX: " << SHRT_MAX << std::endl;
        std::cerr << "wxID_AUTO_LOWEST: " << wxID_AUTO_LOWEST << std::endl;
        std::cerr << "wxID_AUTO_HIGHEST: " << wxID_AUTO_HIGHEST << std::endl;
        std::cerr << "action->id: " << action->id << std::endl;
        std::cerr << "action->name: " << action->name() << std::endl;
        std::cerr << "action->label: " << action->label.get() << std::endl;
        std::cerr << "action->description: " << action->description.get() << std::endl;
        exit(1);
    }

    wxMenuItem* item = new wxMenuItem(menu, action->id, label, shortHelp);
    if (icon.isSet()) {
        int iconSize = context->preferredMenuIconSize();
        auto bmp = icon.toBitmap(iconSize, iconSize, wxART_MENU);
        if (bmp.isOk())
            basWxSetMenuItemBitmap(item, bmp);
    }
    menu->Append(item);

    menu->Bind(
        wxEVT_MENU,
        [act = this->action](wxCommandEvent& event) {
            PerformContext ctx(act, 0, nullptr, &event);
            act->perform(&ctx);
        },
        id);

    log(menu, item);
}

void ActionVB::build(wxToolBar* toolbar) {
    int iconSize = context->preferredToolIconSize();
    wxBitmap bmp;
    if (icon.isSet()) {
        BitmapResult result = icon.toBitmap(iconSize, iconSize, wxART_TOOLBAR);
        if (result.isOk()) {
            bmp = result;
        }
    }
    if (!bmp.IsOk()) {
        wxSize size(iconSize, iconSize);
        bmp = wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_TOOLBAR, size);
    }

    wxString toolLabel = label;
    toolLabel.Replace("&", "");

    auto id = action->id;
    basWxToolBarAddTool(toolbar, id, toolLabel, bmp, shortHelp, wxITEM_NORMAL);

    toolbar->Bind(
        wxEVT_TOOL,
        [act = this->action](wxCommandEvent& event) {
            PerformContext ctx(act, 0, nullptr, &event);
            act->perform(&ctx);
        },
        id);

    log(toolbar, id);
}

void ActionVB::build(wxAuiToolBar* toolbar) {
    int iconSize = context->preferredToolIconSize();
    wxBitmap bmp;
    if (icon.isSet()) {
        BitmapResult result = icon.toBitmap(iconSize, iconSize, wxART_TOOLBAR);
        if (result.isOk()) {
            bmp = result;
        }
    }
    if (!bmp.IsOk()) {
        wxSize size(iconSize, iconSize);
        bmp = wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_TOOLBAR, size);
    }

    wxString toolLabel = label;
    toolLabel.Replace("&", "");

    auto id = action->id;
    basWxAuiToolBarAddTool(toolbar, id, toolLabel, bmp, shortHelp, wxITEM_NORMAL);

    toolbar->Bind(
        wxEVT_TOOL,
        [act = this->action](wxCommandEvent& event) {
            PerformContext ctx(act, 0, nullptr, &event);
            act->perform(&ctx);
        },
        id);

    log(toolbar, id);
}

void ActionVB::log(wxMenu* menu, wxMenuItem* item) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::MENU_ITEM;
    log->menu = menu;
    log->menuItem = item;
    logs->push_back(std::move(log));
}

void ActionVB::log(wxToolBar* toolbar, int toolId) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::TOOLBAR_TOOL;
    log->toolbar = toolbar;
    log->toolId = toolId;
    logs->push_back(std::move(log));
}

void ActionVB::log(wxAuiToolBar* toolbar, int toolId) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::TOOLBAR_TOOL;
    log->auiToolbar = toolbar;
    log->toolId = toolId;
    logs->push_back(std::move(log));
}