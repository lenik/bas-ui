#ifndef WX_UIFRAME_HPP
#define WX_UIFRAME_HPP

#include "../ui/arch/BuildViewContext.hpp"
#include "../ui/arch/UIFragment.hpp"
#include "../ui/arch/UIGroup.hpp"
#include "../ui/arch/UIState.hpp"

#include <wx/aui/auibar.h>
#include <wx/aui/framemanager.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>

#include <vector>

class uiFrame : public wxFrame, public UIFragment {
  public:
    enum {
        ID_TOOLBAR_SMALL = wxID_HIGHEST + 1000,
        ID_TOOLBAR_SHOW_LABEL,
        ID_APP_HIGHEST = ID_TOOLBAR_SHOW_LABEL,
    };

    uiFrame(const wxString& title,                                            //
            std::optional<std::vector<UIFragment*>> fragments = std::nullopt, //
            wxWindow* parent = nullptr,                                       //
            wxWindowID id = wxID_ANY,                                         //
            const wxPoint& pos = wxDefaultPosition,                           //
            const wxSize& size = wxSize(800, 600),                            //
            long style = wxDEFAULT_FRAME_STYLE,                               //
            const wxString& name = wxFrameNameStr                             //
    );
    virtual ~uiFrame();

    void addFragment(UIFragment* fragment);
    void removeFragment(UIFragment* fragment);

    void createView();

    void getDefaultMenubarsSupported(std::unordered_set<std::string>& set) const override;
    void getDefaultToolbarsSupported(std::unordered_set<std::string>& set) const override;
    void getDefaultAuiToolbarsSupported(std::unordered_set<std::string>& set) const override;
    wxMenuBar* makeDefaultMenubar(std::string_view path) override;
    wxToolBar* makeDefaultToolbar(std::string_view path) override;
    wxAuiToolBar* makeDefaultAuiToolbar(std::string_view path) override;

    void createFragmentView(CreateViewContext* ctx) override;

    void addFragmentView(UIFragment* fragment, CreateViewContext* ctx);
    void removeFragmentView(UIFragment* fragment, CreateViewContext* ctx);

    void exitOnShow(bool exit = true);

    wxEvtHandler* getEventHandler() override {
        wxFrame* frame = dynamic_cast<wxFrame*>(this);
        return frame->GetEventHandler();
    }

  private:
    void create();

  protected:
    BuildViewContext m_buildViewContext;
    BuildViewLogs m_buildViewLogs;

  private:
    std::vector<UIFragment*> m_fragments;
    UIGroup m_root;
    wxAuiManager m_auiManager;
    int m_nextAuiPaneId{0};

    wxMenuBar* m_menubar{nullptr};
    wxToolBar* m_toolbar{nullptr};
    wxAuiToolBar* m_auiToolbar{nullptr};
    wxPanel* m_contentPanel{nullptr};

    bool m_exitOnShow{false};
    observable<UIStateVariant>* m_showLabel;

  private:
    void onShowExit(wxShowEvent& event);

    void onCommand(wxCommandEvent& event, UIAction* action);
    void onExit(PerformContext* ctx);

    void onBoolStateChange(wxCommandEvent& event, UIState* state);
    void onEnumStateChange(wxCommandEvent& event, UIState* state);

    void setToolbarSize(int size);
    void setToolbarLabel(bool value);

    void updateAuiPaneInfo();
};

#endif // WX_UIFRAME_HPP