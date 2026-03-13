/**
 * Simple notepad application using ui/arch action/group models.
 * Menubar and toolbar are built from a UIFragment and BuildViewContext.
 */
#include "appframe.hpp"
#include "ui/arch/CreateViewContext.hpp"

#include <bas/log/uselog.h>

AppFrame::AppFrame(const wxString& title,              //
                   std::vector<UIFragment*> fragments, //
                   wxWindow* parent,                   //
                   wxWindowID id,                      //
                   const wxPoint& pos,                 //
                   const wxSize& size,                 //
                   long style,                         //
                   const wxString& name                //
                   )
    : wxFrame(parent, id, title, pos, size, style, name) //
{
    m_fragments.push_back(this);
    m_fragments.insert(m_fragments.end(), fragments.begin(), fragments.end());

    CreateViewContext createViewContext(wxID_ANY, this, "");
    for (auto& fragment : m_fragments) {
        fragment->createView(&createViewContext);
    }

    group(1, "", "file", 10, "&File").install();
    group(2, "", "edit", 20, "&Edit").install();
    group(3, "", "view", 30, "&View").install();

    int seq = 100000;
    action(wxID_EXIT, "file", "exit", seq++, "E&xit", "Exit")
        .icon(wxART_QUIT)
        .performFn([this](PerformContext* ctx) { onExit(ctx); })
        .no_tool()
        .install();

    seq = 1000;

    // action(ID_TOOLBAR_SMALL, "view", "toolbar_small", seq++, "Toolbar &Small", "Toolbar
    // small")
    //     .icon(wxART_LIST)
    //     .performFn([this](PerformContext* ctx) { onToolbarSmall(ctx); })
    //     .install();

    state(ID_TOOLBAR_SHOW_LABEL, "view", "toolbar_show_label", seq++, "Toolbar &Show Label",
          "Toolbar show label")
        .icon(wxART_LIST_VIEW)
        .stateType(UIStateType::BOOL)
        .valueDescriptorFn([this](int value) {
            return UIStateValueDescriptor{
                .label = value ? "Show Label" : "Hide Label",       //
                .description = value ? "Show label" : "Hide label", //
            };
        })
        .initValue(false)
        .valueRef(&m_showLabel)
        .connect([this](UIStateVariant const value, UIStateVariant const old_value) {
            bool showLabel = std::get<bool>(value);
            onToolbarShowLabel(showLabel);
        })
        .install();

    BuildViewContext buildViewContext;

    m_menubar = new wxMenuBar();
    m_toolbar = CreateToolBar(wxTB_FLAT);

    buildViewContext.registerMenubar("", m_menubar);
    buildViewContext.registerToolbar("", m_toolbar);

    std::vector<UIElement*> elements;
    for (auto& fragment : m_fragments) {
        auto part = fragment->loadElements();
        elements.insert(elements.end(), part.begin(), part.end());
    }

    m_root = UIGroup(0, "", "", 0, "<root>", "", "", //
                     ImageSet(), true, true);
    m_root.buildTree(elements);
    m_root.buildView(&buildViewContext, &m_buildViewLogs);

    SetMenuBar(m_menubar);
    m_toolbar->Realize();

    // Connect menu and toolbar events for each action/state ID where supported
    for (auto& el : elements) {
        if (el->isAction()) {
            UIAction* action = dynamic_cast<UIAction*>(el);
            Bind(
                wxEVT_MENU,
                [this, action](wxCommandEvent& event) { //
                    onCommand(event, action);
                },
                el->id);
            Bind(
                wxEVT_TOOL,
                [this, action](wxCommandEvent& event) { //
                    onCommand(event, action);
                },
                el->id);
        }
        if (el->isState()) {
            UIState* state = dynamic_cast<UIState*>(el);
            Bind(
                wxEVT_MENU,
                [this, state](wxCommandEvent& event) { //
                    onStateChange(event, state);
                },
                el->id);
            Bind(
                wxEVT_TOOL,
                [this, state](wxCommandEvent& event) { //
                    onStateChange(event, state);
                },
                el->id);
        }
    }
}

AppFrame::~AppFrame() {}

void AppFrame::exitOnShow(bool exit) {
    m_exitOnShow = exit;
    if (exit) {
        Bind(wxEVT_SHOW, &AppFrame::onShowExit, this);
    } else {
        Unbind(wxEVT_SHOW, &AppFrame::onShowExit, this);
    }
}

void AppFrame::onShowExit(wxShowEvent& event) {
    // Simulate exit command after window is shown (for testing)
    std::cout << "Window shown, simulating exit..." << std::endl;
    wxCommandEvent exitEvent(wxEVT_MENU, wxID_EXIT);
    GetEventHandler()->AddPendingEvent(exitEvent);
}

void AppFrame::onCommand(wxCommandEvent& event, UIAction* action) {
    PerformContext ctx(action, 0, nullptr, &event, getEventHandler());
    action->perform(&ctx);
}

void AppFrame::onExit(PerformContext* ctx) { Close(); }

void AppFrame::onStateChange(wxCommandEvent& event, UIState* state) {
    bool checked = event.IsChecked();
    state->value.set(checked);
}

void AppFrame::onToolbarSize(int size) {
    // m_toolbar->SetToolBarStyle(wxTB_TEXT | wxTB_HORIZONTAL);
}

void AppFrame::onToolbarShowLabel(bool value) {
    long style = m_toolbar->GetWindowStyle();
    if (value) {
        style |= wxTB_TEXT;     // Add text
        style &= ~wxTB_NOICONS; // Ensure icons are NOT hidden
    } else {
        style &= ~wxTB_TEXT; // Remove text
    }
    m_toolbar->SetWindowStyle(style);
    m_toolbar->Realize();
}
