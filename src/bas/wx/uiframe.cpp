/**
 * Simple notepad application using ui/arch action/group models.
 * Menubar and toolbar are built from a UIFragment and BuildViewContext.
 */
#include "uiframe.hpp"

#include "../../module.def"

#include "../ui/arch/BuildViewContext.hpp"
#include "../ui/arch/CreateViewContext.hpp"

#include <wx/aui/auibar.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/toolbar.h>

#include <bas/log/uselog.h>

#define _(s) dgettext(TEXT_DOMAIN, (s))

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
    m_auiManager.SetManagedWindow(this);
    create();

    m_fragments.push_back(this);
    if (fragments) {
        m_fragments.insert(m_fragments.end(), fragments->begin(), fragments->end());
        createView();
    }
}

uiFrame::~uiFrame() { m_auiManager.UnInit(); }

void uiFrame::create() {
    std::string dir = "streamline-vectors/core/pop/interface-essential";
    std::string dir2 = "streamline-vectors/core/pop/map-travel";

    group(1, "", "file", 10, _("&File")).install();
    group(2, "", "edit", 20, _("&Edit")).install();
    group(3, "", "view", 30, _("&View")).install();

    int seq = 100000;
    action(wxID_EXIT, "file", "exit", seq++, _("E&xit"), _("Exit"))
        .icon(wxART_QUIT, dir2, "emergency-exit.svg")
        .shortcut("Ctrl+Q")
        .performFn([this](PerformContext* ctx) { onExit(ctx); })
        .no_tool()
        .install();

    seq = 1000;

    // action(ID_TOOLBAR_SMALL, "view", "toolbar_small", seq++, "AuiToolbar &Small", "AuiToolbar
    // small")
    //     .icon(wxART_LIST)
    //     .performFn([this](PerformContext* ctx) { onAuiToolbarSmall(ctx); })
    //     .install();

    state(ID_TOOLBAR_SHOW_LABEL, "view", _("toolbar_show_label"), seq++, //
          _("AuiToolbar &Show Label"), _("AuiToolbar show label"))
        .icon(wxART_LIST_VIEW, dir, "text-square.svg")
        .shortcut("Ctrl+L")
        .stateType(UIStateType::BOOL)
        .valueDescriptorFn([this](int value) {
            UIStateValueDescriptor d;
            d.label = value ? _("Show Label") : _("Hide Label");
            d.description = value ? _("Show label") : _("Hide label");
            return d;
        })
        .initValue(false)
        .valueRef(&m_showLabel)
        .connect([this](UIStateVariant const value, UIStateVariant const old_value) {
            bool showLabel = std::get<bool>(value);
            setToolbarLabel(showLabel);
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
    if (!m_contentPanel) {
        m_contentPanel = new wxPanel(this, wxID_ANY);
        m_contentPanel->SetSizer(new wxBoxSizer(wxVERTICAL));
        if (m_buildViewContext.isAuiPreferred()) {
            m_auiManager.AddPane(m_contentPanel,
                                 wxAuiPaneInfo().Name("content").CenterPane().PaneBorder(false));
        } else {
            wxSizer* frameSizer = GetSizer();
            if (frameSizer == nullptr) {
                frameSizer = new wxBoxSizer(wxVERTICAL);
                SetSizer(frameSizer);
            }
            frameSizer->Add(m_contentPanel, 1, wxEXPAND);
        }
    }

    CreateViewContext ctx(wxID_ANY, m_contentPanel, "");

    wxSizer* rootSizer = m_contentPanel->GetSizer();
    if (rootSizer == nullptr) {
        auto* box = new wxBoxSizer(wxVERTICAL);
        m_contentPanel->SetSizer(box);
        rootSizer = box;
    }

    for (auto& fragment : m_fragments) {
        auto content = fragment->createFragmentView(&ctx);
        if (content) {
            rootSizer->Add(content, 1, wxEXPAND);
        }

        for (auto path : fragment->getDefaultMenubarsSupported()) {
            auto menubars = ctx.getMenubars(path);
            if (menubars.empty()) {
                auto menubar = fragment->makeDefaultMenubar(path);
                m_buildViewContext.registerMenubar(path, menubar);
            }
        }

        for (auto path : fragment->getDefaultMenusSupported()) {
            auto menubars = ctx.getMenus(path);
            if (menubars.empty()) {
                auto menu = fragment->makeDefaultMenu(path);
                m_buildViewContext.registerMenu(path, menu);
            }
        }

        if (m_buildViewContext.isAuiPreferred()) {
            for (auto path : fragment->getDefaultAuiToolbarsSupported()) {
                auto toolbars = ctx.getAuiToolbars(path);
                if (toolbars.empty()) {
                    auto toolbar = fragment->makeDefaultAuiToolbar(path);
                    if (!toolbar)
                        continue;
                    m_buildViewContext.registerAuiToolbar(path, toolbar);
                }
            }
        } else {
            for (auto path : fragment->getDefaultToolbarsSupported()) {
                auto toolbars = ctx.getToolbars(path);
                if (toolbars.empty()) {
                    auto toolbar = fragment->makeDefaultToolbar(path);
                    m_buildViewContext.registerToolbar(path, toolbar);
                }
            }
        }
    }

    std::vector<UIElement*> all;
    for (auto& fragment : m_fragments) {
        auto part = fragment->elements();
        all.insert(all.end(), part.begin(), part.end());
    }

    m_root = UIGroup(0, "", "", 0, "<root>", //
                     "", "",                 //
                     ImageSet(), true, true);
    m_root.addToTree(all, &ctx);
    m_root.buildView(&m_buildViewContext, &m_buildViewLogs);

    if (m_buildViewContext.isAuiPreferred()) {
        m_buildViewContext.forAuiToolbars([this](wxAuiToolBar* toolbar) {
            int position = m_nextAuiPaneId++;
            wxString paneName = wxString::Format("aui-toolbar-%d", position);
            wxAuiPaneInfo paneInfo;
            paneInfo.Name(paneName)
                .ToolbarPane()
                .Layer(10)
                .Top()
                .Row(0)
                .Position(position)
                .Dockable(true)
                // .Fixed()
                .Movable(true)
                .Floatable(true)
                .Gripper(true)
                .PaneBorder(false)
                .CaptionVisible(false);
            m_auiManager.AddPane(toolbar, paneInfo);
        });
        m_auiManager.Update();
    } else if (m_toolbar) {
        m_buildViewContext.forToolbars([](wxToolBar* toolbar) { toolbar->Realize(); });
    }
}

void uiFrame::getDefaultMenubarsSupported(std::unordered_set<std::string>& set) const {
    set.insert("");
}
void uiFrame::getDefaultToolbarsSupported(std::unordered_set<std::string>& set) const {
    set.insert("");
}
void uiFrame::getDefaultAuiToolbarsSupported(std::unordered_set<std::string>& set) const {
    set.insert("");
}
wxMenuBar* uiFrame::makeDefaultMenubar(std::string_view path) {
    if (path == "") {
        if (m_menubar == nullptr) {
            m_menubar = new wxMenuBar();
            SetMenuBar(m_menubar);
        }
        return m_menubar;
    }
    return nullptr;
}
wxToolBar* uiFrame::makeDefaultToolbar(std::string_view path) {
    if (path == "") {
        if (m_toolbar == nullptr) {
            m_toolbar = CreateToolBar(wxTB_FLAT);
        }
        return m_toolbar;
    }
    return nullptr;
}
wxAuiToolBar* uiFrame::makeDefaultAuiToolbar(std::string_view path) {
    if (path == "") {
        if (m_auiToolbar == nullptr) {
            m_auiToolbar = m_buildViewContext.createAuiToolbar(this, wxID_ANY);
        }
        return m_auiToolbar;
    }
    return nullptr;
}

wxWindow* uiFrame::createFragmentView(CreateViewContext* ctx) { return nullptr; }

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
    PerformContext ctx(action, 0, nullptr, &event);
    action->perform(&ctx);
}

void uiFrame::onExit(PerformContext* ctx) { Close(); }

void uiFrame::setToolbarSize(int size) {
    // m_toolbar->SetToolBarStyle(wxTB_TEXT | wxTB_HORIZONTAL);
}

void uiFrame::setToolbarLabel(bool value) {
    if (m_buildViewContext.isAuiPreferred()) {
        m_buildViewContext.forAuiToolbars([this, value](wxAuiToolBar* toolbar) {
            long style = toolbar->GetWindowStyle();
            long oldStyle = style;
            if (value) {
                style |= wxAUI_TB_TEXT;
            } else {
                style &= ~wxAUI_TB_TEXT;
            }
            if (style == oldStyle)
                return;
            toolbar->SetWindowStyle(style);
        });
        updateAuiPaneInfo();
    } else if (m_toolbar) {
        long style = m_toolbar->GetWindowStyle();
        if (value) {
            style |= wxTB_TEXT;
            style &= ~wxTB_NOICONS;
        } else {
            style &= ~wxTB_TEXT;
        }
        m_toolbar->SetWindowStyle(style);
        m_toolbar->Realize();
    }
}

void uiFrame::updateAuiPaneInfo() {
    wxAuiPaneInfoArray& allPanes = m_auiManager.GetAllPanes();

    for (size_t i = 0; i < allPanes.GetCount(); ++i) {
        wxAuiPaneInfo& pane = allPanes.Item(i);

        if (pane.IsToolbar() && pane.window) {
            wxAuiToolBar* tb = wxDynamicCast(pane.window, wxAuiToolBar);
            if (tb) {
                tb->Realize();
                wxSize size = tb->GetBestSize();
                pane.BestSize(size);
            }
        }
    }
    m_auiManager.Update();
}