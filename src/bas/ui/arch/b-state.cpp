#include "b-state.hpp"

#include "../../wx/toolbar.hpp"
#include "../../wx/wx_compat.hpp"

#include <wx/tbarbase.h>

#include <utility>
#include <vector>

void StateVB::build(wxMenu* menu) {
    auto& shortcuts = state->getShortcuts();
    if (!shortcuts.empty()) {
        basWxAppendMenuShortcut(label, shortcuts[0]);
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
    switch (type) {
    case UIStateType::BOOL:
        bool2Tool(wx::ToolBar(toolbar), val<bool>(false));
        break;
    case UIStateType::ENUM:
        if (cycled)
            enum2ToolCycled(wx::ToolBar(toolbar), val<int>(0));
        else
            enum2RadioTools(wx::ToolBar(toolbar), val<int>(0));
        break;
    default:
        std::cout << "StateVB::build: not supported type: " << static_cast<int>(type) << std::endl;
        break;
    }
}

void StateVB::build(wxAuiToolBar* toolbar) {
    switch (type) {
    case UIStateType::BOOL:
        bool2Tool(wx::ToolBar(toolbar), val<bool>(false));
        break;
    case UIStateType::ENUM:
        if (cycled)
            enum2ToolCycled(wx::ToolBar(toolbar), val<int>(0));
        else
            enum2RadioTools(wx::ToolBar(toolbar), val<int>(0));
        break;
    default:
        std::cout << "StateVB::build: not supported type: " << static_cast<int>(type) << std::endl;
        break;
    }
}

void StateVB::bool2Menu(wxMenu* menu, bool val) {
    auto id = state->id;
    wxMenuItem* item = menu->AppendCheckItem(state->id, label, shortHelp);
    menu->Check(state->id, val);

    menu->Bind(
        wxEVT_MENU,
        [state = this->state](wxCommandEvent& event) {
            bool checked = event.IsChecked();
            state->value.set(checked);
        },
        id);

    log(menu, item);
}

void StateVB::bool2Tool(wx::ToolBar toolbar, bool val) {
    int id = state->id;
    wxItemKind kind = wxITEM_CHECK;
    wxBitmap bitmap = state->icon->toBitmap1(toolIconSize, toolIconSize, wxART_TOOLBAR);
    toolbar.AddTool(id, label, bitmap, shortHelp, kind);

    toolbar.Bind(
        wxEVT_TOOL,
        [state = this->state](wxCommandEvent& event) { //
            bool checked = event.IsChecked();
            state->value.set(checked);
        },
        id);

    state->value.connect(
        [tb = toolbar, id](UIStateVariant const value, UIStateVariant const) mutable {
            auto b = std::get<bool>(value);
            bool state = tb.GetToolToggled(id);
            if (state != b)
                tb.ToggleTool(id, b);
        });

    log(toolbar, id);
}

void StateVB::enum2Menu(wxMenu* menu, int val) {
    const std::vector<int> enumValues = state->getEnumValues();
    int iconSize = context->preferredMenuIconSize();

    wxMenu* submenu = new wxMenu();
    for (int v : enumValues) {
        auto d = state->getValueDescriptor(v);
        if (!d) {
            printf("val/menu: no descriptor for value %d\n", v);
            continue;
        }
        int itemId = d->id(context, state->id, v);
        wxMenuItem* item = new wxMenuItem(submenu, itemId, d->label, d->description, wxITEM_RADIO);
        if (d->icon.isSet()) {
            auto bmp = d->icon.toBitmap(iconSize, iconSize, wxART_MENU);
            if (bmp.isOk())
                basWxSetMenuItemBitmap(item, bmp);
        }
        submenu->Append(item);
        if (v == val) {
            submenu->Check(itemId, true);
        }

        auto st = this->state;
        submenu->Bind(
            wxEVT_MENU,
            [st, v](wxCommandEvent&) { st->value.set(v); },
            itemId);
    }

    // Add as submenu
    wxMenuItem* item = menu->Append(state->id, label, submenu, shortHelp);

    log(menu, item);
}

void StateVB::enum2RadioTools(wx::ToolBar toolbar, int val) {
    const std::vector<int> enumValues = state->getEnumValues();
    std::vector<std::pair<int, int>> tools; // value -> tool id

    toolbar.addNecessarySeparator();

    for (int v : enumValues) {
        auto d = state->getValueDescriptor(v);
        if (!d) {
            printf("val/radio: no descriptor for value %d\n", v);
            continue;
        }

        int id = d->id(context, state->id, v);
        auto kind = wxITEM_RADIO;
        wxBitmap bmp = d->icon.toBitmap1(toolIconSize, toolIconSize, wxART_TOOLBAR);
        wxString toolLabel = d->label;
        toolLabel.Replace("&", "");
        toolbar.AddTool(id, toolLabel, bmp, d->description, kind);

        auto st = this->state;
        toolbar.Bind(
            wxEVT_TOOL,
            [st, v](wxCommandEvent&) { st->value.set(v); },
            id);

        tools.emplace_back(v, id);

        log(toolbar, id);
    }

    toolbar.addNecessarySeparator();

    // toggle radio
    for (const auto& [v, id] : tools) {
        if (v == val)
            toolbar.ToggleTool(id, true);
    }
    state->value.connect(
        [toolbar, tools](UIStateVariant const value, UIStateVariant const) mutable {
            int n = std::get<int>(value);
            for (const auto& [v, id] : tools) {
                if (v == n) {
                    toolbar.ToggleTool(id, true);
                    return;
                }
            }
        });
}

void StateVB::enum2ToolCycled(wx::ToolBar toolbar, int val) {
    auto d = state->getValueDescriptor(val);
    if (!d) {
        printf("val/cycle: no descriptor for value %d\n", val);
        return;
    }

    int id = state->id;

    auto kind = wxITEM_NORMAL;
    wxBitmap bmp = d->icon.toBitmap1(toolIconSize, toolIconSize, wxART_TOOLBAR);
    toolbar.AddTool //
        (id, d->label, bmp, d->description, kind);

    auto st = this->state;
    toolbar.Bind(
        wxEVT_TOOL,
        [st](wxCommandEvent& event) {
            int currentValue = 0;
            if (auto* p = std::get_if<int>(&st->value.get()))
                currentValue = *p;

            // find the next value in the enum values
            const std::vector<int> enumValues = st->getEnumValues();
            int matched_index = -1;
            int i = 0;
            for (int v : enumValues) {
                if (v == currentValue) {
                    matched_index = i;
                    break;
                }
                i++;
            }
            if (matched_index == -1) {
                std::cout << "Enum state change: current value not found in enum values"
                          << std::endl;
                return;
            }
            int next_index = (matched_index + 1) % enumValues.size();
            int next_value = enumValues[next_index];
            st->value.set(next_value);
        },
        id);

    // change tool bitmap & label
    state->value.connect([st, toolbar, id, size = toolIconSize](UIStateVariant const value,
                                                                UIStateVariant const) mutable {
        int n = std::get<int>(value);
        auto d = st->getValueDescriptor(n);
        if (!d) {
            printf("val/cycle: no descriptor for value %d\n", n);
            return;
        }

        wxBitmap bmp = d->icon.toBitmap1(size, size, wxART_TOOLBAR);

        printf("val/cycle\n");
        toolbar.SetToolLabel(id, d->label);
        toolbar.SetToolBitmap(id, bmp);
    });

    log(toolbar, id);
}

void StateVB::log(wxMenu* menu, wxMenuItem* item) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::MENU_ITEM;
    log->menu = menu;
    log->menuItem = item;
    logs->push_back(std::move(log));
}

void StateVB::log(wx::ToolBar toolbar, int toolId) {
    auto log = std::make_unique<BuildViewLog>();
    log->kind = BuildViewLog::TOOLBAR_TOOL;
    if (toolbar.isWx()) {
        log->toolbar = toolbar.getWx();
    } else {
        log->auiToolbar = toolbar.getAui();
    }
    log->toolId = toolId;
    logs->push_back(std::move(log));
}
