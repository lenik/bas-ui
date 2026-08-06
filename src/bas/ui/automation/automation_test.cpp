/**
 * Smoke test for bas::ui::automation.
 *
 * Creates a tiny dialog, drives it via emulate()/ScriptRunner, and asserts
 * state through AutomationQuery. No ShowModal — suitable for headless CI when
 * a display (or xvfb) is available for wx init.
 */

#include <bas/ui/automation/Automatable.hpp>
#include <bas/ui/automation/AutomationError.hpp>
#include <bas/ui/automation/AutomationQuery.hpp>
#include <bas/ui/automation/ScriptRunner.hpp>

#include <wx/app.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

enum {
    ID_OK_BTN = wxID_HIGHEST + 1,
    ID_USER,
    ID_ROLE,
    ID_AGREE,
};

class SmokeDialog : public wxDialog, public bas::ui::automation::Automatable {
  public:
    SmokeDialog()
        : wxDialog(nullptr, wxID_ANY, "automation smoke", wxDefaultPosition, wxDefaultSize,
                   wxDEFAULT_DIALOG_STYLE),
          Automatable(this) {
        auto* root = new wxBoxSizer(wxVERTICAL);

        m_user = new wxTextCtrl(this, ID_USER);
        m_user->SetName("user");
        root->Add(m_user, 0, wxEXPAND | wxALL, 4);

        m_role = new wxChoice(this, ID_ROLE);
        m_role->Append("guest");
        m_role->Append("admin");
        m_role->Append("operator");
        m_role->SetSelection(0);
        root->Add(m_role, 0, wxEXPAND | wxALL, 4);

        m_agree = new wxCheckBox(this, ID_AGREE, "I agree");
        root->Add(m_agree, 0, wxALL, 4);

        m_ok = new wxButton(this, ID_OK_BTN, "OK");
        root->Add(m_ok, 0, wxALL, 4);

        SetSizerAndFit(root);

        automationMap().bind("user", ID_USER);
        automationMap().bind("role", ID_ROLE);
        automationMap().bind("agree", ID_AGREE);
        automationMap().bind("ok", ID_OK_BTN);

        m_ok->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { m_clicked = true; });
    }

    bool clicked() const { return m_clicked; }

  private:
    wxTextCtrl* m_user = nullptr;
    wxChoice* m_role = nullptr;
    wxCheckBox* m_agree = nullptr;
    wxButton* m_ok = nullptr;
    bool m_clicked = false;
};

class SmokeApp : public wxApp {
  public:
    bool OnInit() override {
        // Avoid needing a visible frame for ProcessEvent-based tests.
        return true;
    }
};

void expect(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "FAIL: " << msg << "\n";
        std::exit(1);
    }
}

int runSmoke() {
    using namespace bas::ui::automation;

    SmokeDialog dlg;

    expect(dlg.query().exists("user"), "user exists");
    expect(dlg.query().getValue("user").empty(), "user empty initially");

    dlg.emulate("setValue", "user", {{"text", "alice"}});
    expect(dlg.query().getValue("user") == "alice", "setValue user");

    dlg.emulate("type", "user", {{"text", "!"}});
    expect(dlg.query().getValue("user") == "alice!", "type append");

    dlg.emulate("select", "role", {{"value", "admin"}});
    expect(dlg.query().getSelection("role") == 1, "select role by value");
    expect(dlg.query().getValue("role") == "admin", "role value");

    dlg.emulate("check", "agree");
    expect(dlg.query().isChecked("agree"), "check agree");

    dlg.emulate("click", "ok", {{"x", 5}, {"y", 5}});
    expect(dlg.clicked(), "ok clicked via emulate");

    // Name discovery fallback (GetName on text ctrl).
    expect(dlg.query().exists("user"), "name map still works");

    // ScriptRunner
    dlg.emulate("setValue", "user", {{"text", ""}});
    ScriptRunner runner(dlg);
    runner.runJson(R"([
      {"do": "setValue", "on": "user", "data": {"text": "bob"}},
      {"do": "select", "on": "role", "data": {"index": 2}},
      {"do": "uncheck", "on": "agree"},
      {"do": "wait", "data": {"ms": 1}},
      ["click", "ok"]
    ])");
    expect(dlg.query().getValue("user") == "bob", "script setValue");
    expect(dlg.query().getSelection("role") == 2, "script select");
    expect(!dlg.query().isChecked("agree"), "script uncheck");

    // Soft fail
    expect(!dlg.tryEmulate("click", "missing-control"), "tryEmulate missing");

    bool threw = false;
    try {
        dlg.emulate("nope", "user");
    } catch (const AutomationError&) {
        threw = true;
    }
    expect(threw, "unknown event throws");

    std::cout << "automation_test: OK\n";
    return 0;
}

} // namespace

wxIMPLEMENT_APP_NO_MAIN(SmokeApp);

int main(int argc, char** argv) {
    wxApp::SetInstance(new SmokeApp());
    if (!wxEntryStart(argc, argv)) {
        std::cerr << "wxEntryStart failed (display available?)\n";
        return 2;
    }
    if (!wxTheApp->CallOnInit()) {
        wxEntryCleanup();
        return 2;
    }
    const int rc = runSmoke();
    wxTheApp->OnExit();
    wxEntryCleanup();
    return rc;
}
