#include "wxLoginUi.hpp"

#include <bas/security/LoginUi.hpp>

#include <wx/app.h>
#include <wx/button.h>
#include <wx/dcbuffer.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <libintl.h>

#include <unordered_map>

#include "../../module.def"

#define _(s) dgettext(TEXT_DOMAIN, (s))

namespace bas::security {

namespace {

constexpr int kDialogMinWidth = 420;
constexpr int kFieldHeight = 34;
constexpr int kContentPad = 24;

wxString utf8(const std::string& text) { return wxString::FromUTF8(text.c_str()); }

std::string jsonOptionString(const JsonObject& object, const char* key) {
    const auto it = object.find(key);
    if (it == object.end() || !it->value().is_string()) {
        return {};
    }
    return std::string(it->value().as_string().c_str());
}

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

class LoginHeaderPanel : public wxPanel {
  public:
    LoginHeaderPanel(wxWindow* parent, const wxString& title, const wxString& subtitle)
        : wxPanel(parent, wxID_ANY), m_title(title), m_subtitle(subtitle) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetMinSize(wxSize(kDialogMinWidth, subtitle.empty() ? 72 : 92));
        Bind(wxEVT_PAINT, &LoginHeaderPanel::onPaint, this);
    }

  private:
    void onPaint(wxPaintEvent& event) {
        wxAutoBufferedPaintDC dc(this);
        const wxSize size = GetClientSize();

        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(wxColour(36, 58, 102)));
        dc.DrawRectangle(0, 0, size.x, size.y);

        dc.SetBrush(wxBrush(wxColour(88, 140, 220)));
        dc.DrawRectangle(0, 0, 5, size.y);

        dc.SetTextForeground(*wxWHITE);
        dc.SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        dc.DrawText(m_title, 22, 18);

        if (!m_subtitle.empty()) {
            dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.SetTextForeground(wxColour(196, 208, 228));
            dc.DrawLabel(m_subtitle, wxRect(22, 46, size.x - 32, 40), wxALIGN_LEFT | wxST_ELLIPSIZE_END);
        }
        event.Skip();
    }

    wxString m_title;
    wxString m_subtitle;
};

class LoginFieldPanel : public wxPanel {
  public:
    LoginFieldPanel(wxWindow* parent, const wxString& label, wxTextCtrl* input, bool required)
        : wxPanel(parent, wxID_ANY) {
        SetBackgroundColour(parent->GetBackgroundColour());

        auto* root = new wxBoxSizer(wxVERTICAL);
        wxString caption = label;
        if (required) {
            caption += " *";
        }

        auto* captionText = new wxStaticText(this, wxID_ANY, caption);
        captionText->SetForegroundColour(wxColour(55, 60, 68));
        captionText->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        root->Add(captionText, 0, wxBOTTOM, 6);

        input->Reparent(this);
        input->SetMinSize(wxSize(-1, kFieldHeight));
        root->Add(input, 0, wxEXPAND);

        SetSizer(root);
    }
};

class LoginFormDialog : public wxDialog {
  public:
    LoginFormDialog(wxWindow* parent, const LoginFormSpec& form, const CredentialRequest& request)
        : wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
          m_form(form),
          m_request(request) {
        const wxString title =
            utf8(form.title.empty() ? std::string("Sign in") : form.title);
        const std::string subtitleText = [&]() -> std::string {
            const std::string fromForm = jsonOptionString(form.options, "subtitle");
            if (!fromForm.empty()) {
                return fromForm;
            }
            const std::string description = jsonOptionString(form.options, "description");
            if (!description.empty()) {
                return description;
            }
            return request.reason;
        }();

        SetBackgroundColour(wxColour(248, 249, 251));
        SetMinSize(wxSize(kDialogMinWidth, -1));

        auto* root = new wxBoxSizer(wxVERTICAL);
        root->Add(new LoginHeaderPanel(this, title, utf8(subtitleText)), 0, wxEXPAND);

        auto* body = new wxPanel(this, wxID_ANY);
        body->SetBackgroundColour(GetBackgroundColour());
        auto* bodySizer = new wxBoxSizer(wxVERTICAL);

        const std::string hint = jsonOptionString(form.options, "hint");
        if (!hint.empty()) {
            auto* hintText = new wxStaticText(body, wxID_ANY, utf8(hint));
            hintText->SetForegroundColour(wxColour(90, 96, 108));
            hintText->Wrap(kDialogMinWidth - kContentPad * 2);
            bodySizer->Add(hintText, 0, wxEXPAND | wxBOTTOM, 14);
        }

        for (const LoginField& field : form.fields) {
            const wxString label =
                utf8(field.label.empty() ? field.name : field.label);

            long style = wxTE_PROCESS_ENTER | wxBORDER_THEME;
            if (field.type == "password" || field.name == "password") {
                style |= wxTE_PASSWORD;
            }

            auto* ctrl = new wxTextCtrl(body, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                        wxDefaultSize, style);
            if (!request.subjectHint.empty() &&
                (field.name == "username" || field.type == "text")) {
                ctrl->SetValue(utf8(request.subjectHint));
            }

            bodySizer->Add(new LoginFieldPanel(body, label, ctrl, field.required), 0,
                           wxEXPAND | wxBOTTOM, 14);
            m_fields[field.name] = ctrl;
        }

        m_errorText = new wxStaticText(body, wxID_ANY, wxEmptyString);
        m_errorText->SetForegroundColour(wxColour(180, 45, 45));
        m_errorText->Hide();
        bodySizer->Add(m_errorText, 0, wxEXPAND | wxTOP, 2);

        body->SetSizer(bodySizer);
        root->Add(body, 1, wxEXPAND | wxALL, kContentPad);

        auto* footer = new wxPanel(this, wxID_ANY);
        footer->SetBackgroundColour(wxColour(240, 242, 246));
        auto* footerSizer = new wxBoxSizer(wxHORIZONTAL);
        footerSizer->AddStretchSpacer();

        auto* cancelBtn = new wxButton(footer, wxID_CANCEL, _("Cancel"));
        cancelBtn->SetMinSize(wxSize(96, kFieldHeight));
        footerSizer->Add(cancelBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

        auto* signInBtn = new wxButton(footer, wxID_OK, _("Sign in"));
        signInBtn->SetMinSize(wxSize(112, kFieldHeight));
        signInBtn->SetBackgroundColour(wxColour(36, 58, 102));
        signInBtn->SetForegroundColour(*wxWHITE);
        signInBtn->SetDefault();
        footerSizer->Add(signInBtn, 0, wxALIGN_CENTER_VERTICAL);

        footer->SetSizer(footerSizer);
        root->Add(footer, 0, wxEXPAND | wxTOP, 0);
        root->AddSpacer(0);

        SetSizer(root);
        Layout();
        Fit();
        CentreOnParent();

        if (!m_fields.empty()) {
            m_fields.begin()->second->SetFocus();
            if (!request.subjectHint.empty()) {
                m_fields.begin()->second->SelectAll();
            }
        }

        Bind(wxEVT_BUTTON, &LoginFormDialog::onButton, this);
        Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) { trySubmit(); });
    }

    std::optional<Credential> buildCredential() {
        return credentialFromLoginForm(
            m_form, m_request, [this](const LoginField& field) -> std::optional<std::string> {
                const auto it = m_fields.find(field.name);
                if (it == m_fields.end()) {
                    return std::string{};
                }
                return std::string(it->second->GetValue().utf8_str());
            });
    }

  private:
    void onButton(wxCommandEvent& event) {
        if (event.GetId() == wxID_OK) {
            trySubmit();
            return;
        }
        if (event.GetId() == wxID_CANCEL) {
            EndModal(wxID_CANCEL);
        }
    }

    void showError(const wxString& message) {
        m_errorText->SetLabel(message);
        m_errorText->Wrap(kDialogMinWidth - kContentPad * 2);
        m_errorText->Show();
        Layout();
    }

    void clearError() {
        m_errorText->Hide();
        m_errorText->SetLabel(wxEmptyString);
    }

    void trySubmit() {
        clearError();
        for (const LoginField& field : m_form.fields) {
            if (!field.required) {
                continue;
            }
            const auto it = m_fields.find(field.name);
            if (it == m_fields.end() || it->second->GetValue().empty()) {
                const wxString label = utf8(field.label.empty() ? field.name : field.label);
                showError(_("Please enter ") + label + ".");
                if (it != m_fields.end()) {
                    it->second->SetFocus();
                }
                return;
            }
        }
        EndModal(wxID_OK);
    }

    LoginFormSpec m_form;
    CredentialRequest m_request;
    std::unordered_map<std::string, wxTextCtrl*> m_fields;
    wxStaticText* m_errorText{nullptr};
};

} // namespace

std::optional<Credential> showWxLoginDialog(wxWindow* parent, const LoginFormSpec& form,
                                            const CredentialRequest& request) {
    LoginFormDialog dlg(resolveParent(parent), form, request);
    if (dlg.ShowModal() != wxID_OK) {
        return std::nullopt;
    }
    return dlg.buildCredential();
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
