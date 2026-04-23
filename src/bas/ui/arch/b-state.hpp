#ifndef STATE_VB_HPP
#define STATE_VB_HPP

#include "BuildViewContext.hpp"
#include "BuildViewLog.hpp"
#include "ImageSet.hpp"
#include "UIState.hpp"

#include "wx/aui/auibar.h"
#include "wx/bitmap.h"
#include "wx/menu.h"

class StateVB {
  public:
    StateVB(UIState* state, BuildViewContext* context, BuildViewLogs* logs)
        : state(state), context(context), logs(logs) {
        type = state->getType();
        label = state->label.get().empty() ? state->name() : state->label.get();
        shortHelp = state->description.get();
        icon = state->icon.get();

        cycled = state->behavior & static_cast<int>(UIStateBehavior::CYCLED);

        toolIconSize = context->preferredToolIconSize();
    }

    template <typename T> T val(T defaultValue = T()) {
        if (auto* p = std::get_if<T>(&state->value.get()))
            return *p;
        return defaultValue;
    }

    void build(wxMenu* menu);
    void build(wxToolBar* toolbar);
    void build(wxAuiToolBar* toolbar);

    void bool2Menu(wxMenu* menu, bool val);
    void bool2Tool(wxToolBar* toolbar, bool val);
    void bool2Tool2(wxAuiToolBar* toolbar, bool val);

    void enum2Menu(wxMenu* menu, int val);
    void enum2Tool(wxToolBar* toolbar, int val);
    void enum2Tool2(wxAuiToolBar* toolbar, int val);

    void log(wxMenu* menu, wxMenuItem* item);
    void log(wxToolBar* toolbar, int toolId);
    void log(wxAuiToolBar* toolbar, int toolId);

  private:
    UIState* state{nullptr};
    BuildViewContext* context{nullptr};
    BuildViewLogs* logs{nullptr};

    UIStateType type{UIStateType::BOOL};
    wxString label;
    wxString shortHelp;
    ImageSet icon;
    wxBitmap bitmap;

    bool cycled{false};
    int toolIconSize{0};
};

#endif // STATE_VB_HPP