#include "wx_login_interaction.hpp"

#include "../../module.def"

#include <bas/security/LoginUi.hpp>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <libintl.h>

#include <unordered_map>

#define _(s) dgettext(TEXT_DOMAIN, (s))

namespace bas::security {

namespace {

class LoginFormDialog : public wxDialog {
  public:
    LoginFormDialog(wxWindow* parent, const LoginFormSpec& form)
        : wxDialog(parent, wxID_ANY,
                   wxString::FromUTF8(form.title.empty() ? "Login" : form.title.c_str()),
                   wxDefaultPosition, wxDefaultSize,
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
        auto* root = new wxBoxSizer(wxVERTICAL);
        auto* grid = new wxFlexGridSizer(2, wxSize(8, 6));
        grid->AddGrowableCol(1);

        for (const LoginField& field : form.fields) {
            const wxString label =
                wxString::FromUTF8((field.label.empty() ? field.name : field.label).c_str()) +
                (field.required ? wxString(" *") : wxString());
            grid->Add(new wxStaticText(this, wxID_ANY, label), 0, wxALIGN_CENTER_VERTICAL);

            long style = wxTE_PROCESS_ENTER;
            if (field.type == "password" || field.name == "password") {
                style |= wxTE_PASSWORD;
            }
            auto* ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                        wxSize(240, -1), style);
            grid->Add(ctrl, 1, wxEXPAND);
            m_fields[field.name] = ctrl;
        }

        root->Add(grid, 1, wxALL | wxEXPAND, 12);

        auto* buttons = CreateButtonSizer(wxOK | wxCANCEL);
        root->Add(buttons, 0, wxALL | wxEXPAND, 12);

        SetSizerAndFit(root);
        CentreOnParent();

        Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
            if (event.GetId() == wxID_OK) {
                EndModal(wxID_OK);
            } else if (event.GetId() == wxID_CANCEL) {
                EndModal(wxID_CANCEL);
            }
        });
        Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) { EndModal(wxID_OK); });
    }

    std::optional<Credential> buildCredential(const LoginFormSpec& form,
                                              const CredentialRequest& request) {
        return credentialFromLoginForm(
            form, request, [this](const LoginField& field) -> std::optional<std::string> {
                const auto it = m_fields.find(field.name);
                if (it == m_fields.end()) {
                    return std::string{};
                }
                return std::string(it->second->GetValue().utf8_str());
            });
    }

  private:
    std::unordered_map<std::string, wxTextCtrl*> m_fields;
};

wxWindow* resolveParent(wxWindow* parent) {
    if (parent) {
        return parent;
    }
    if (wxWindow* active = wxGetActiveWindow()) {
        return active;
    }
    if (wxTheApp) {
        return wxTheApp->GetTopWindow();
    }
    return nullptr;
}

} // namespace

std::optional<Credential> showWxLoginDialog(wxWindow* parent, const LoginFormSpec& form,
                                            const CredentialRequest& request) {
    LoginFormDialog dlg(resolveParent(parent), form);
    if (dlg.ShowModal() != wxID_OK) {
        return std::nullopt;
    }
    return dlg.buildCredential(form, request);
}

WxLoginUi::WxLoginUi(SecurityManager& controller, wxWindow* parent)
    : m_controller(controller), m_parent(parent) {}

SecurityManager* WxLoginUi::controller() { return &m_controller; }

void WxLoginUi::setParentWindow(wxWindow* parent) { m_parent = parent; }

wxWindow* WxLoginUi::parentWindow() const { return m_parent; }

std::optional<Credential> WxLoginUi::requestCredential(const LoginFormSpec& form,
                                                       const CredentialRequest& request) {
    return showWxLoginDialog(m_parent, form, request);
}

} // namespace bas::security
