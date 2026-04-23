#include "b-state.hpp"

void StateVB::build(wxMenu* menu) {
    auto& shortcuts = state->getShortcuts();
    if (!shortcuts.empty()) {
        label += "\t";
        label += shortcuts[0];
    }

    switch (type) {
    case UIStateType::BOOL:
        bool2Menu(menu, val<bool>(false));
        break;
    case UIStateType::ENUM:
        enum2Menu(menu, val<int>());
        break;
    default:
        std::cout << "StateVB::build: not supported type: " << static_cast<int>(type) << std::endl;
        break;
    }
}

void StateVB::build(wxToolBar* toolbar) {
    int toolIconSize = context->preferredToolIconSize();
    if (icon.isSet()) {
        bitmap = *icon.toBitmap(toolIconSize, toolIconSize, wxART_TOOLBAR);
    } else {
        wxSize size(toolIconSize, toolIconSize);
        bitmap = wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_TOOLBAR, size);
    }

    switch (type) {
    case UIStateType::BOOL:
        bool2Tool(toolbar, val<bool>(false));
        break;
    case UIStateType::ENUM:
        enum2Tool(toolbar, val<int>(0));
        break;
    default:
        std::cout << "StateVB::build: not supported type: " << static_cast<int>(type) << std::endl;
        break;
    }
}

void StateVB::build(wxAuiToolBar* toolbar) {
    int toolIconSize = context->preferredToolIconSize();
    if (icon.isSet()) {
        bitmap = *icon.toBitmap(toolIconSize, toolIconSize, wxART_TOOLBAR);
    } else {
        wxSize size(toolIconSize, toolIconSize);
        bitmap = wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_TOOLBAR, size);
    }

    switch (type) {
    case UIStateType::BOOL:
        bool2Tool2(toolbar, val<bool>(false));
        break;
    case UIStateType::ENUM:
        enum2Tool2(toolbar, val<int>(0));
        break;
    default:
        std::cout << "StateVB::build: not supported type: " << static_cast<int>(type) << std::endl;
        break;
    }
}

void StateVB::bool2Menu(wxMenu* menu, bool val) {
    wxMenuItem* item = menu->AppendCheckItem(state->id, label, shortHelp);
    menu->Check(state->id, val);
    log(menu, item);
}

void StateVB::bool2Tool(wxToolBar* toolbar, bool val) {
    int toolid = state->id;
    wxItemKind kind = wxITEM_CHECK;
    auto tool = toolbar->AddTool(toolid, label, bitmap, shortHelp, kind);

    state->value.connect([this, toolbar](UIStateVariant const value, UIStateVariant const) {
        auto b = std::get<bool>(value);
        // toolbar->ToggleTool(toolid, b);
    });

    log(toolbar, toolid);
}

void StateVB::bool2Tool2(wxAuiToolBar* toolbar, bool val) {
    int toolId = state->id;
    wxItemKind kind = wxITEM_CHECK;
    auto tool = toolbar->AddTool(toolId, label, bitmap, shortHelp, kind);

    state->value.connect([this, toolbar, toolId](UIStateVariant const value, UIStateVariant const) {
        auto b = std::get<bool>(value);
        // toolbar->ToggleTool(toolId, b);
    });

    log(toolbar, toolId);
}

void StateVB::enum2Menu(wxMenu* menu, int val) {
    const std::vector<int> enumValues = state->getEnumValues();

    wxMenu* submenu = new wxMenu();
    for (int v : enumValues) {
        UIStateValueDescriptor d = state->getValueDescriptor(v);
        if (d.label.empty())
            continue;
        int itemId = d.id(context);
        wxMenuItem* item = submenu->AppendRadioItem(itemId, d.label, d.description);
        if (v == val) {
            submenu->Check(itemId, true);
        }
    }
    wxMenuItem* item = menu->Append(state->id, label, submenu, shortHelp);

    log(menu, item);
}

void StateVB::enum2Tool(wxToolBar* toolbar, int val) {
    const std::vector<int> enumValues = state->getEnumValues();
    for (int v : enumValues) {
        UIStateValueDescriptor d = state->getValueDescriptor(v);
        if (d.label.empty())
            break;
        if (cycled && v != val)
            continue;

        wxBitmap bmp = d.icon.toBitmap1(toolIconSize, toolIconSize, wxART_TOOLBAR);

        int toolId = d.id(context);
        auto kind = cycled ? wxITEM_NORMAL : wxITEM_RADIO;
        auto tool = toolbar->AddTool(toolId, d.label, bmp, d.description, kind);

        if (cycled) {
            state->value.connect([this, tool](UIStateVariant const value, UIStateVariant const) {
                int n = std::get<int>(value);
                auto d = state->getValueDescriptor(n);
                wxBitmap bmp = d.icon.toBitmap1(toolIconSize, toolIconSize, wxART_TOOLBAR);

                printf("cycle\n");
                tool->SetLabel(d.label);
                tool->SetNormalBitmap(bmp);
            });
        }

        log(toolbar, toolId);
    }
}

void StateVB::enum2Tool2(wxAuiToolBar* toolbar, int val) {
    const std::vector<int> enumValues = state->getEnumValues();
    for (int v : enumValues) {
        UIStateValueDescriptor d = state->getValueDescriptor(v);
        if (d.label.empty())
            break;
        if (cycled && v != val)
            continue;
        wxBitmap bmp = d.icon.toBitmap1(toolIconSize, toolIconSize, wxART_TOOLBAR);

        int toolId = d.id(context);
        auto kind = cycled ? wxITEM_RADIO : wxITEM_NORMAL;
        auto tool = toolbar->AddTool(toolId, d.label, bmp, wxString(d.description.c_str()), kind);

        if (cycled) {
            state->value.connect(
                [this, toolbar, toolId](UIStateVariant const value, UIStateVariant const) {
                    int n = std::get<int>(value);
                    auto d = state->getValueDescriptor(n);
                    wxBitmap bmp = d.icon.toBitmap1(toolIconSize, toolIconSize, wxART_TOOLBAR);

                    printf("onchange cycle\n");
                    toolbar->SetToolLabel(toolId, d.label);
                    toolbar->SetToolBitmap(toolId, bmp);
                });
        }

        log(toolbar, toolId);
    }
}

void StateVB::log(wxMenu* menu, wxMenuItem* item) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::MENU_ITEM;
    log->menu = menu;
    log->menuItem = item;
    logs->push_back(std::move(log));
}

void StateVB::log(wxToolBar* toolbar, int toolId) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::TOOLBAR_TOOL;
    log->toolbar = toolbar;
    log->toolId = toolId;
    logs->push_back(std::move(log));
}

void StateVB::log(wxAuiToolBar* toolbar, int toolId) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::AUI_TOOLBAR_TOOL;
    log->auiToolbar = toolbar;
    log->toolId = toolId;
    logs->push_back(std::move(log));
}
