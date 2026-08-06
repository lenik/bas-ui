#ifndef BAS_UI_TESTS_DEMO_DIALOG_HPP
#define BAS_UI_TESTS_DEMO_DIALOG_HPP

#include <bas/ui/automation/Automatable.hpp>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>

/**
 * Self-contained dialog used by automation e2e tests.
 * Demonstrates widget bindings + Automatable mixin.
 */
class DemoDialog : public wxDialog, public bas::ui::automation::Automatable {
  public:
    enum {
        ID_GO = wxID_HIGHEST + 1,
        ID_NAME,
        ID_ROLE,
        ID_AGREE,
    };

    explicit DemoDialog(wxWindow* parent = nullptr);

    bool goClicked() const { return m_goClicked; }
    wxTextCtrl* nameCtrl() const { return m_name; }

  private:
    wxTextCtrl* m_name = nullptr;
    wxChoice* m_role = nullptr;
    wxCheckBox* m_agree = nullptr;
    wxButton* m_go = nullptr;
    bool m_goClicked = false;
};

#endif
