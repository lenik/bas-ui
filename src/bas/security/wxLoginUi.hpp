#ifndef BAS_SECURITY_WX_LOGIN_UI_HPP
#define BAS_SECURITY_WX_LOGIN_UI_HPP

#include <bas/security/LoginUi.hpp>

class wxWindow;

namespace bas::security {

/** wxWidgets implementation of LoginUi for SecurityManager interactive login. */
class WxLoginUi : public LoginUi {
  public:
    WxLoginUi(SecurityManager& controller, wxWindow* parent = nullptr);

    SecurityManager* controller() override;

    std::optional<Credential> requestCredential(const LoginFormSpec& form,
                                                const CredentialRequest& request) override;

    void setParentWindow(wxWindow* parent);
    wxWindow* parentWindow() const;

  private:
    SecurityManager& m_controller;
    wxWindow* m_parent;
};

/** Show a modal wx login dialog built from LoginFormSpec. Returns nullopt on cancel. */
std::optional<Credential> showWxLoginDialog(wxWindow* parent, const LoginFormSpec& form,
                                            const CredentialRequest& request);

} // namespace bas::security

#endif // BAS_SECURITY_WX_LOGIN_UI_HPP
