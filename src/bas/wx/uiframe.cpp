/**
 * Simple notepad application using ui/arch action/group models.
 * Menubar and toolbar are built from a UIFragment and BuildViewContext.
 */
#include "uiframe.hpp"

#include "i18n.hpp"
#include "../ui/arch/BuildViewContext.hpp"
#include "../ui/arch/CreateViewContext.hpp"

#include <bas/log/uselog.h>

uiFrame::uiFrame(const wxString& title,                             //
                 std::optional<std::vector<UIFragment*>> fragments, //
                 wxWindow* parent,                                  //
                 wxWindowID id,                                     //
                 const wxPoint& pos,                                //
                 const wxSize& size,                                //
                 long style,                                        //
                 const wxString& name                               //
                 )
    : wxFrame(parent, id, title, pos, size, style, name) //
{
    create();

    m_fragments.push_back(this);
    if (fragments) {
        m_fragments.insert(m_fragments.end(), fragments->begin(), fragments->end());
        createView();
    }
}

uiFrame::~uiFrame() {}

void uiFrame::create() {
    std::string dir = "streamline-vectors/core/pop/interface-essential";
    std::string dir2 = "streamline-vectors/core/pop/map-travel";

    group(1, "", "file", 10, basUiMsg("&File")).install();
    group(2, "", "edit", 20, basUiMsg("&Edit")).install();
    group(3, "", "view", 30, basUiMsg("&View")).install();

    int seq = 100000;
    action(wxID_EXIT, "file", "exit", seq++, basUiMsg("E&xit"), basUiMsg("Exit"))
        .icon(wxART_QUIT, dir2, "emergency-exit.svg")
        .performFn([this](PerformContext* ctx) { onExit(ctx); })
        .no_tool()
        .install();

    seq = 1000;

    // action(ID_TOOLBAR_SMALL, "view", "toolbar_small", seq++, "Toolbar &Small", "Toolbar
    // small")
    //     .icon(wxART_LIST)
    //     .performFn([this](PerformContext* ctx) { onToolbarSmall(ctx); })
    //     .install();

    state(ID_TOOLBAR_SHOW_LABEL, "view", "toolbar_show_label", seq++, basUiMsg("Toolbar &Show Label"),
          basUiMsg("Toolbar show label"))
        .icon(wxART_LIST_VIEW, dir, "text-square.svg")
        .stateType(UIStateType::BOOL)
        .valueDescriptorFn([this](int value) {
            UIStateValueDescriptor d;
            d.label = value ? basUiMsg("Show Label") : basUiMsg("Hide Label");
            d.description = value ? basUiMsg("Show label") : basUiMsg("Hide label");
            return d;
        })
        .initValue(false)
        .valueRef(&m_showLabel)
        .connect([this](UIStateVariant const value, UIStateVariant const old_value) {
            bool showLabel = std::get<bool>(value);
            onToolbarShowLabel(showLabel);
        })
        .install();
}

void uiFrame::addFragment(UIFragment* fragment) { //
    m_fragments.push_back(fragment);
}

void uiFrame::removeFragment(UIFragment* fragment) {
    m_fragments.erase(std::remove(m_fragments.begin(), m_fragments.end(), fragment),
                      m_fragments.end());
}

void uiFrame::createView() {
    CreateViewContext ctx(wxID_ANY, this, "");
    for (auto& fragment : m_fragments) {
        fragment->createFragmentView(&ctx);
    }

    m_buildViewContext.registerMenubar("", m_menubar);
    m_buildViewContext.registerToolbar("", m_toolbar);

    std::vector<UIElement*> all;
    for (auto& fragment : m_fragments) {
        auto part = fragment->elements();
        all.insert(all.end(), part.begin(), part.end());
    }

    m_root = UIGroup(0, "", "", 0, "<root>", "", "", //
                     ImageSet(), true, true);
    m_root.addToTree(all, &ctx);
    m_root.buildView(&m_buildViewContext, &m_buildViewLogs);

    m_toolbar->Realize();

    // Connect menu and toolbar events for each action/state ID where supported
    for (auto& el : all) {
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
            UIStateType type = state->getType();
            switch (type) {
            case UIStateType::BOOL:
                Bind(
                    wxEVT_MENU,
                    [this, state](wxCommandEvent& event) { //
                        onBoolStateChange(event, state);
                    },
                    el->id);
                Bind(
                    wxEVT_TOOL,
                    [this, state](wxCommandEvent& event) { //
                        onBoolStateChange(event, state);
                    },
                    el->id);
                break;

            case UIStateType::ENUM: {
                const std::vector<int> enumValues = state->getEnumValues();
                for (int v : enumValues) {
                    UIStateValueDescriptor d = state->getValueDescriptor(v);
                    int itemId = d.id(&m_buildViewContext);
                    Bind(
                        wxEVT_MENU,
                        [this, state](wxCommandEvent& event) { //
                            onEnumStateChange(event, state);
                        },
                        itemId);
                    Bind(
                        wxEVT_TOOL,
                        [this, state](wxCommandEvent& event) { //
                            onEnumStateChange(event, state);
                        },
                        itemId);
                }
                break;
            }

            default:
                // not supported yet.
                break;
            }
        }
    }
}

void uiFrame::createFragmentView(CreateViewContext* ctx) {
    m_menubar = new wxMenuBar();
    SetMenuBar(m_menubar);

    m_toolbar = CreateToolBar(wxTB_FLAT);
}

void uiFrame::addFragmentView(UIFragment* fragment, CreateViewContext* ctx) {
    addFragment(fragment);

    fragment->createFragmentView(ctx);

    std::vector<UIElement*> elements = fragment->elements();
    m_root.addToTree(elements, ctx);

    std::unordered_set<UIElement*> white_set{elements.begin(), elements.end()};
    m_root.buildView(&m_buildViewContext, &m_buildViewLogs, //
                     white_set);
}

void uiFrame::removeFragmentView(UIFragment* fragment, CreateViewContext* ctx) {
    std::vector<UIElement*> elements = fragment->elements();

    std::unordered_set<UIElement*> white_set{elements.begin(), elements.end()};
    m_root.removeBuild(&m_buildViewContext, white_set);

    m_root.removeFromTree(elements);

    fragment->destroyFragmentView(ctx);

    removeFragment(fragment);
}

void uiFrame::exitOnShow(bool exit) {
    m_exitOnShow = exit;
    if (exit) {
        Bind(wxEVT_SHOW, &uiFrame::onShowExit, this);
    } else {
        Unbind(wxEVT_SHOW, &uiFrame::onShowExit, this);
    }
}

void uiFrame::onShowExit(wxShowEvent& event) {
    // Simulate exit command after window is shown (for testing)
    std::cout << "Window shown, simulating exit..." << std::endl;
    wxCommandEvent exitEvent(wxEVT_MENU, wxID_EXIT);
    GetEventHandler()->AddPendingEvent(exitEvent);
}

void uiFrame::onCommand(wxCommandEvent& event, UIAction* action) {
    PerformContext ctx(action, 0, nullptr, &event, getEventHandler());
    action->perform(&ctx);
}

void uiFrame::onExit(PerformContext* ctx) { Close(); }

void uiFrame::onBoolStateChange(wxCommandEvent& event, UIState* state) {
    bool checked = event.IsChecked();
    state->value.set(checked);
}

void uiFrame::onEnumStateChange(wxCommandEvent& event, UIState* state) {
    int id = event.GetId();

    std::optional<int> value = state->findValueById(id);
    if (!value) {
        std::cout << "Enum state change: unknown id " << id << std::endl;
        return;
    }
    state->value.set(*value);
}

void uiFrame::onToolbarSize(int size) {
    // m_toolbar->SetToolBarStyle(wxTB_TEXT | wxTB_HORIZONTAL);
}

void uiFrame::onToolbarShowLabel(bool value) {
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
