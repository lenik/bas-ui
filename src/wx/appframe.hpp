#ifndef WX_APPFRAME_HPP
#define WX_APPFRAME_HPP

#include "ui/arch/BuildViewContext.hpp"
#include "ui/arch/UIFragment.hpp"
#include "ui/arch/UIGroup.hpp"
#include "ui/arch/UIState.hpp"

#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>

#include <vector>

class AppFrame : public wxFrame, public UIFragment {
  public:
    enum {
        ID_TOOLBAR_SMALL = wxID_HIGHEST + 1,
        ID_TOOLBAR_SHOW_LABEL,
        ID_APP_HIGHEST = ID_TOOLBAR_SHOW_LABEL,
    };

    AppFrame(const wxString& title,                  //
             std::vector<UIFragment*> fragments,     //
             wxWindow* parent = nullptr,             //
             wxWindowID id = wxID_ANY,               //
             const wxPoint& pos = wxDefaultPosition, //
             const wxSize& size = wxDefaultSize,     //
             long style = wxDEFAULT,                 //
             const wxString& name = wxFrameNameStr   //
    );

    void exitOnShow(bool exit = true) { m_exitOnShow = exit; }

    wxEvtHandler* getEventHandler() override { return this->getEventHandler(); }

  private:
    std::vector<UIFragment*> m_fragments;
    UIGroup m_root;
    BuildViewLogs m_buildViewLogs;

    wxMenuBar* m_menubar;
    wxToolBar* m_toolbar;

    bool m_exitOnShow{false};

  private:
    void onShowExit(wxShowEvent& event);

    void onCommand(wxCommandEvent& event, UIAction* action);
    void onExit(PerformContext* ctx);

    void onStateChange(wxCommandEvent& event, UIState* state);
    void onToolbarSize(int size);
    void onToolbarShowLabel(bool value);
};

#endif // WX_APPFRAME_HPP