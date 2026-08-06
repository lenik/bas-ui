#ifndef BAS_UI_TESTS_DEMO_FRAME_HPP
#define BAS_UI_TESTS_DEMO_FRAME_HPP

#include <bas/wx/uiframe.hpp>

#include <wx/textctrl.h>

/**
 * Minimal uiFrame + UIFragment demo for arch automation
 * (perform / setstate / path bindings).
 */
class DemoBody : public UIFragment {
  public:
    enum {
        ID_SAY_HELLO = uiFrame::ID_APP_HIGHEST + 1,
        ID_CLEAR,
        ID_BOLD,
    };

    DemoBody();

    void defineActions();
    void defineStates();
    wxWindow* createFragmentView(CreateViewContext* ctx) override;

    int helloCount() const { return m_helloCount; }
    wxTextCtrl* editor() const { return m_editor; }

  private:
    void doHello(PerformContext*);
    void doClear(PerformContext*);

    wxTextCtrl* m_editor = nullptr;
    int m_helloCount = 0;
};

class DemoFrame : public uiFrame {
  public:
    explicit DemoFrame(const wxString& title = "Automation Demo Frame");

    DemoBody& body() { return m_body; }

  private:
    DemoBody m_body;
};

#endif
