/**
 * GUI tank access-control demo — device actions gated by bas.security RBAC.
 */
#include "guitanks.hpp"

#include "bas/ui/arch/UIFragment.hpp"
#include "bas/wx/uiframe.hpp"

#include <bas/security/AccessDenied.hpp>
#include <bas/security/Subject.hpp>

#include <libintl.h>

#include <wx/aui/auibar.h>
#include <wx/button.h>
#include <wx/dcbuffer.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

#include <bas/proc/stackdump.h>

#include "module.def"

#define _(s) dgettext(TEXT_DOMAIN, (s))

namespace sec = bas::security;

namespace {

const std::string dir1 = "streamline-vectors/core/pop/interface-essential";
const std::string dirEntertainment = "streamline-vectors/core/pop/entertainment";
const std::string dirMoneyShopping = "streamline-vectors/core/pop/money-shopping";

constexpr int kLogMaxLines = 100;

const wchar_t kTankGlyph[4] = {L'\u25b2', L'\u25b6', L'\u25bc', L'\u25c0'};

class DeviceLoginDialog : public wxDialog {
  public:
    DeviceLoginDialog(wxWindow* parent, const DeviceSlot& device, const wxString& title,
                      const wxString& message)
        : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
        auto* root = new wxBoxSizer(wxVERTICAL);
        if (!message.empty()) {
            root->Add(new wxStaticText(this, wxID_ANY, message), 0, wxALL | wxEXPAND, 10);
        }

        auto* grid = new wxFlexGridSizer(2, wxSize(8, 6));
        grid->Add(new wxStaticText(this, wxID_ANY, _("Username:")), 0, wxALIGN_CENTER_VERTICAL);
        m_user = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(220, -1));
        grid->Add(m_user, 1, wxEXPAND);
        grid->Add(new wxStaticText(this, wxID_ANY, _("Password:")), 0, wxALIGN_CENTER_VERTICAL);
        m_pass = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(220, -1),
                                wxTE_PASSWORD);
        grid->Add(m_pass, 1, wxEXPAND);
        root->Add(grid, 0, wxALL | wxEXPAND, 10);

        auto* demoBox =
            new wxStaticBoxSizer(wxVERTICAL, this, _("Demo accounts (user / pass / role)"));
        for (const DemoAccount& acct : demoAccountsFor(device)) {
            wxString line = wxString::Format("%s / %s  (%s)", acct.user, acct.password, acct.role);
            auto* btn = new wxButton(demoBox->GetStaticBox(), wxID_ANY, line);
            demoBox->Add(btn, 0, wxALL | wxEXPAND, 2);
            btn->Bind(wxEVT_BUTTON, [this, acct](wxCommandEvent&) {
                m_user->SetValue(acct.user);
                m_pass->SetValue(acct.password);
            });
        }
        root->Add(demoBox, 0, wxALL | wxEXPAND, 10);
        root->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
        SetSizerAndFit(root);
        CentreOnParent();
    }

    bool runSubmitted(std::string& user, std::string& pass) {
        if (ShowModal() != wxID_OK) {
            return false;
        }
        user = trim(std::string(m_user->GetValue().utf8_str()));
        pass = trim(std::string(m_pass->GetValue().utf8_str()));
        return !user.empty();
    }

  private:
    wxTextCtrl* m_user;
    wxTextCtrl* m_pass;
};

wxButton* makeActionButton(wxWindow* parent, const wxString& label, int id) {
    auto* btn = new wxButton(parent, id, label, wxDefaultPosition, wxSize(88, 32));
    return btn;
}

} // namespace

TankCanvas::TankCanvas(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(360, 320), wxBORDER_THEME) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetMinSize(wxSize(280, 240));
    Bind(wxEVT_PAINT, &TankCanvas::onPaint, this);
}

void TankCanvas::setState(const TankState& state) {
    m_state = state;
    Refresh();
}

void TankCanvas::onPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();

    const wxSize size = GetClientSize();
    const int pad = 12;
    const int fieldW = size.x - pad * 2;
    const int fieldH = size.y - pad * 2;
    const int cellW = std::max(12, fieldW / kTankFieldW);
    const int cellH = std::max(12, fieldH / kTankFieldH);
    const int originX = (size.x - cellW * kTankFieldW) / 2;
    const int originY = (size.y - cellH * kTankFieldH) / 2;

    dc.SetPen(wxPen(wxColour(60, 70, 80)));
    dc.SetBrush(wxBrush(wxColour(28, 32, 38)));
    dc.DrawRectangle(originX - 1, originY - 1, cellW * kTankFieldW + 2, cellH * kTankFieldH + 2);

    for (int y = 0; y < kTankFieldH; ++y) {
        for (int x = 0; x < kTankFieldW; ++x) {
            dc.SetPen(wxPen(wxColour(40, 46, 52)));
            dc.SetBrush(wxBrush(wxColour(34, 38, 44)));
            dc.DrawRectangle(originX + x * cellW, originY + y * cellH, cellW - 1, cellH - 1);
        }
    }

    auto cellCenter = [&](int gx, int gy) {
        return wxPoint(originX + gx * cellW + cellW / 2, originY + gy * cellH + cellH / 2);
    };

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(wxColour(230, 70, 60)));
    for (const auto& bullet : m_state.bullets) {
        const wxPoint c = cellCenter(bullet.x, bullet.y);
        dc.DrawCircle(c, std::max(3, std::min(cellW, cellH) / 4));
    }

    if (m_state.fireFlash > 0) {
        int dx = 0;
        int dy = 0;
        switch (m_state.facing) {
        case 0:
            dy = -1;
            break;
        case 1:
            dx = 1;
            break;
        case 2:
            dy = 1;
            break;
        case 3:
            dx = -1;
            break;
        }
        const wxPoint muzzle = cellCenter(m_state.x + dx, m_state.y + dy);
        dc.SetBrush(wxBrush(wxColour(255, 200, 80)));
        dc.DrawCircle(muzzle, std::max(4, std::min(cellW, cellH) / 3));
    }

    const wxPoint tankCenter = cellCenter(m_state.x, m_state.y);
    dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.SetTextForeground(m_state.engine ? wxColour(255, 210, 60) : wxColour(180, 185, 190));
    const int facing = std::clamp(m_state.facing, 0, 3);
    dc.DrawText(wxString(kTankGlyph[facing]), tankCenter.x - cellW / 4, tankCenter.y - cellH / 3);

    dc.SetTextForeground(wxColour(120, 130, 140));
    dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.DrawText(m_state.engine ? _("Engine running") : _("Engine stopped"), pad, size.y - pad - 14);
    event.Skip();
}

GuitanksBody::GuitanksBody() { defineActions(); }

void GuitanksBody::defineActions() {
    int seq = 0;
    action(ID_DEVICE_SWITCH, "device", "switch", seq++, _("Switch &Device"), "Switch active device")
        .shortcut("Tab")
        .performFn([this](PerformContext* ctx) { doSwitchDevice(ctx); })
        .install();
    action(ID_DEVICE_LOGIN, "device", "login", seq++, _("&Login"), "Login to selected device")
        .icon(wxART_NORMAL_FILE, dir1, "clipboard-check.svg")
        .shortcut("L")
        .performFn([this](PerformContext* ctx) { doLogin(ctx); })
        .install();

    seq = 0;
    action(ID_TANK_START, "tank", "start", seq++, _("Engine &Start"), "Start engine")
        .icon(wxART_GO_FORWARD, dirEntertainment, "button-play.svg")
        .shortcut("F1")
        .performFn([this](PerformContext* ctx) { doStart(ctx); })
        .install();
    action(ID_TANK_STOP, "tank", "stop", seq++, _("Engine S&top"), "Stop engine")
        .icon(wxART_ERROR, dirEntertainment, "button-stop.svg")
        .shortcut("F2")
        .performFn([this](PerformContext* ctx) { doStop(ctx); })
        .install();
    action(ID_TANK_FIRE, "tank", "fire", seq++, _("&Fire"), "Fire")
        .icon(wxART_WARNING, dirMoneyShopping, "target.svg")
        .shortcut("Space")
        .performFn([this](PerformContext* ctx) { doFire(ctx); })
        .install();
    action(ID_TANK_FORWARD, "tank", "forward", seq++, _("Move &Forward"), "Move forward")
        .icon(wxART_GO_UP, dir1, "line-arrow-up-1.svg")
        .shortcut("Up")
        .performFn([this](PerformContext* ctx) { doForward(ctx); })
        .install();
    action(ID_TANK_BACKWARD, "tank", "backward", seq++, _("Move &Backward"), "Move backward")
        .icon(wxART_GO_DOWN, dir1, "line-arrow-move-down-1.svg")
        .shortcut("Down")
        .performFn([this](PerformContext* ctx) { doBackward(ctx); })
        .install();
    action(ID_TANK_LEFT, "tank", "left", seq++, _("Turn &Left"), "Turn left")
        .icon(wxART_GO_BACK, dir1, "line-arrow-move-left-2.svg")
        .shortcut("Left")
        .performFn([this](PerformContext* ctx) { doLeft(ctx); })
        .install();
    action(ID_TANK_RIGHT, "tank", "right", seq++, _("Turn &Right"), "Turn right")
        .icon(wxART_GO_FORWARD, dir1, "line-arrow-move-right-2.svg")
        .shortcut("Right")
        .performFn([this](PerformContext* ctx) { doRight(ctx); })
        .install();
}

void GuitanksBody::setFrameWindow(wxWindow* parent) {
    m_frame = parent;
    if (!m_ctx.sm) {
        initContext();
    }
}

void GuitanksBody::initContext() {
    m_ctx = makeContext();
    m_ctx.sm->setLoginUi(std::make_shared<sec::WxLoginUi>(*m_ctx.sm, m_frame));
    m_tanks.assign(m_ctx.devices.size(), TankState{});
    for (auto& tank : m_tanks) {
        tankReset(tank);
    }
    pushLog(_("Tab switch device  L login  F1/F2 engine  arrows move  Space fire"));
    pushLog(_("tank-a: alice drives, bob fires, admin has full access."));
    pushLog(_("tank-b: charlie trains, dana instructs."));
}

std::size_t GuitanksBody::activeIndex() const {
    if (!m_notebook) {
        return 0;
    }
    return static_cast<std::size_t>(std::max(0, m_notebook->GetSelection()));
}

wxPanel* GuitanksBody::buildDevicePage(const DeviceSlot& device, TankCanvas* canvas) {
    auto* page = new wxPanel(m_notebook);
    auto* root = new wxBoxSizer(wxHORIZONTAL);

    root->Add(canvas, 2, wxEXPAND | wxALL, 8);

    auto* controls = new wxBoxSizer(wxVERTICAL);
    auto* box = new wxStaticBoxSizer(wxVERTICAL, page, _("Device controls"));

    box->Add(makeActionButton(page, _("Start"), ID_TANK_START), 0, wxALL, 4);
    box->Add(makeActionButton(page, _("Stop"), ID_TANK_STOP), 0, wxALL, 4);
    box->AddSpacer(8);
    box->Add(makeActionButton(page, _("Forward"), ID_TANK_FORWARD), 0, wxALL, 4);
    auto* turnRow = new wxBoxSizer(wxHORIZONTAL);
    turnRow->Add(makeActionButton(page, _("Left"), ID_TANK_LEFT), 0, wxRIGHT, 4);
    turnRow->Add(makeActionButton(page, _("Right"), ID_TANK_RIGHT), 0, wxLEFT, 4);
    box->Add(turnRow, 0, wxALL, 4);
    box->Add(makeActionButton(page, _("Backward"), ID_TANK_BACKWARD), 0, wxALL, 4);
    box->AddSpacer(8);
    auto* fireBtn = makeActionButton(page, _("Fire"), ID_TANK_FIRE);
    fireBtn->SetBackgroundColour(wxColour(120, 30, 30));
    fireBtn->SetForegroundColour(*wxWHITE);
    box->Add(fireBtn, 0, wxALL | wxEXPAND, 4);

    controls->Add(box, 0, wxEXPAND | wxALL, 8);
    controls->AddStretchSpacer();
    root->Add(controls, 0, wxEXPAND | wxALL, 4);
    page->SetSizer(root);

    const struct {
        int id;
        const char* op;
    } kBindings[] = {
        {ID_TANK_START, "start"},     {ID_TANK_STOP, "stop"},         {ID_TANK_FIRE, "fire"},
        {ID_TANK_FORWARD, "forward"}, {ID_TANK_BACKWARD, "backward"}, {ID_TANK_LEFT, "left"},
        {ID_TANK_RIGHT, "right"},
    };
    for (const auto& binding : kBindings) {
        page->Bind(
            wxEVT_BUTTON, [this, op = std::string(binding.op)](wxCommandEvent&) { performOp(op); },
            binding.id);
    }
    return page;
}

wxWindow* GuitanksBody::createFragmentView(CreateViewContext* ctx) {
    auto* panel = new wxPanel(ctx->getParent());
    auto* root = new wxBoxSizer(wxVERTICAL);

    m_sessionText = new wxStaticText(panel, wxID_ANY, wxEmptyString);
    m_statusText = new wxStaticText(panel, wxID_ANY, wxEmptyString);
    auto* header = new wxBoxSizer(wxHORIZONTAL);
    header->Add(m_sessionText, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    header->Add(m_statusText, 2, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    root->Add(header, 0, wxEXPAND | wxTOP | wxBOTTOM, 6);

    m_notebook = new wxNotebook(panel, wxID_ANY);
    m_canvases.clear();
    for (const auto& device : m_ctx.devices) {
        auto* canvas = new TankCanvas(m_notebook);
        m_canvases.push_back(canvas);
        m_notebook->AddPage(buildDevicePage(device, canvas),
                            wxString::FromUTF8(device.label.c_str()));
    }
    m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &GuitanksBody::onNotebookChanged, this);

    m_policyText =
        new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(260, -1),
                       wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    m_policyText->SetMinSize(wxSize(220, 200));

    auto* middle = new wxBoxSizer(wxHORIZONTAL);
    middle->Add(m_notebook, 2, wxEXPAND | wxLEFT | wxRIGHT, 8);
    middle->Add(m_policyText, 1, wxEXPAND | wxRIGHT, 8);
    root->Add(middle, 3, wxEXPAND);

    m_logList = new wxListBox(panel, wxID_ANY);
    root->Add(new wxStaticText(panel, wxID_ANY, _("Activity log")), 0, wxLEFT | wxTOP, 8);
    root->Add(m_logList, 1, wxEXPAND | wxALL, 8);

    panel->SetSizer(root);

    m_timer = new wxTimer(panel);
    panel->Bind(wxEVT_TIMER, &GuitanksBody::onTimer, this, m_timer->GetId());
    if (m_ctx.sm) {
        m_timer->Start(60);
        refreshUi();
    }
    return panel;
}

void GuitanksBody::pushLog(const std::string& line) {
    m_log.push_back(line);
    while (m_log.size() > static_cast<std::size_t>(kLogMaxLines)) {
        m_log.pop_front();
    }
    refreshLog();
}

void GuitanksBody::refreshLog() {
    if (!m_logList) {
        return;
    }
    m_logList->Clear();
    for (const auto& line : m_log) {
        m_logList->Append(wxString::FromUTF8(line.c_str()));
    }
    if (m_logList->GetCount() > 0) {
        m_logList->SetSelection(m_logList->GetCount() - 1);
    }
}

void GuitanksBody::refreshSessionBar() {
    if (!m_sessionText || !m_statusText) {
        return;
    }
    const std::size_t idx = activeIndex();
    if (idx >= m_ctx.devices.size()) {
        return;
    }
    const auto& device = m_ctx.devices[idx];
    const std::string user = primaryUser(*m_ctx.sm, device.realm);
    const std::string role = primaryRole(*m_ctx.sm, device.realm);
    const TankState& tank = m_tanks[idx];

    m_sessionText->SetLabel(wxString::Format(
        _("Device: %s   User: %s   Role: %s   Facing: %s"),
        wxString::FromUTF8(device.label.c_str()), wxString::FromUTF8(user.c_str()),
        wxString::FromUTF8(role.c_str()), wxString::FromUTF8(tankFacingName(tank.facing))));

    wxString status = wxString::FromUTF8(tank.lastMessage.c_str());
    if (status.empty()) {
        status = _("Ready — actions are checked against device policy.");
    }
    m_statusText->SetLabel(status);
    m_statusText->SetForegroundColour(
        tank.lastMessage.rfind("DENIED", 0) == 0 ? wxColour(200, 60, 60) : wxColour(40, 40, 40));
}

void GuitanksBody::refreshPolicyPanel() {
    if (!m_policyText) {
        return;
    }
    const std::size_t idx = activeIndex();
    if (idx >= m_ctx.devices.size()) {
        return;
    }
    const auto& device = m_ctx.devices[idx];
    const auto* policy = defaultPolicyFor(m_ctx, device.realm);

    wxString text;
    text << wxString::Format(_("Realm policy: %s\n\n"), wxString::FromUTF8(device.name.c_str()));

    text << _("BINDINGS\nidentity      aclId\n");
    if (policy && !policy->bindings().empty()) {
        for (const auto& binding : policy->bindings()) {
            const bool active = identityMatchesCurrent(m_ctx, device.realm, binding.identity);
            text << wxString::Format("  %-13s %s%s\n",
                                     wxString::FromUTF8(formatIdentity(binding.identity).c_str()),
                                     wxString::FromUTF8(binding.aclId.c_str()),
                                     active ? " *" : "");
        }
    } else {
        text << "  (none)\n";
    }

    text << "\nACLS\n";
    if (policy && !policy->acls().empty()) {
        for (const auto& acl : policy->acls()) {
            const bool aclActive =
                isAclActiveForCurrent(m_ctx, device.realm, policy, acl.id);
            text << wxString::Format("  %s (%zu entries)%s\n", wxString::FromUTF8(acl.id.c_str()),
                                     acl.entries.size(), aclActive ? " *" : "");
            for (const auto& entry : acl.entries) {
                text << wxString::Format("    %-16s %c\n",
                                         wxString::FromUTF8(entry.permission.toString().c_str()),
                                         policyEffectChar(entry.effect));
            }
        }
    } else {
        text << "  (none)\n";
    }

    text << "\nGRANTS\nidentity       permission       M\n";
    if (policy && !policy->grants().empty()) {
        for (const auto& grant : policy->grants()) {
            const bool highlight = shouldHighlightGrant(m_ctx, device.realm, grant);
            text << wxString::Format("  %-14s %-16s %c%s\n",
                                     wxString::FromUTF8(formatIdentity(grant.identity).c_str()),
                                     wxString::FromUTF8(grant.permission.toString().c_str()),
                                     policyEffectChar(grant.effect), highlight ? " *" : "");
        }
    } else {
        text << "  (none)\n";
    }

    text << _("\n* marks rules matching the current session.");
    m_policyText->SetValue(text);
}

void GuitanksBody::refreshUi() {
    for (std::size_t i = 0; i < m_canvases.size() && i < m_tanks.size(); ++i) {
        m_canvases[i]->setState(m_tanks[i]);
    }
    refreshSessionBar();
    refreshPolicyPanel();
    refreshLog();
}

void GuitanksBody::onNotebookChanged(wxBookCtrlEvent& event) {
    (void)event;
    refreshSessionBar();
    refreshPolicyPanel();
}

void GuitanksBody::switchDevice() {
    if (!m_notebook || m_notebook->GetPageCount() == 0) {
        return;
    }
    const int next = (m_notebook->GetSelection() + 1) % m_notebook->GetPageCount();
    m_notebook->SetSelection(next);
    pushLog("switch -> " + m_ctx.devices[static_cast<std::size_t>(next)].name);
    refreshUi();
}

void GuitanksBody::onCharHook(wxKeyEvent& event) {
    wxWindow* focus = wxWindow::FindFocus();
    if (focus && dynamic_cast<wxTextCtrl*>(focus) != nullptr) {
        event.Skip();
        return;
    }

    const int key = event.GetKeyCode();
    switch (key) {
    case WXK_TAB:
        switchDevice();
        return;
    case 'l':
    case 'L':
        doLogin(nullptr);
        return;
    case WXK_F1:
        performOp("start");
        return;
    case WXK_F2:
        performOp("stop");
        return;
    case WXK_SPACE:
        performOp("fire");
        return;
    case WXK_UP:
        performOp("forward");
        return;
    case WXK_DOWN:
        performOp("backward");
        return;
    case WXK_LEFT:
        performOp("left");
        return;
    case WXK_RIGHT:
        performOp("right");
        return;
    default:
        break;
    }
    event.Skip();
}

void GuitanksBody::onTimer(wxTimerEvent& event) {
    (void)event;
    for (auto& tank : m_tanks) {
        tankTick(tank);
    }
    refreshUi();
}

bool GuitanksBody::showCredentialDialog(const DeviceSlot& device, const wxString& title,
                                        const wxString& message, bool /*asLogin*/,
                                        std::string& user, std::string& pass) {
    DeviceLoginDialog dlg(m_frame, device, title, message);
    return dlg.runSubmitted(user, pass);
}

bool GuitanksBody::tryElevatedOp(const DeviceSlot& device, const std::string& op,
                                 OpResult& result) {
    const auto permIt = kOpPermissions.find(op);
    if (permIt == kOpPermissions.end()) {
        return false;
    }

    const bool sessionEmpty = primaryUser(*m_ctx.sm, device.realm) == "(none)";
    sec::AccessRequestOptions opts;
    opts.realmHint = device.realm;
    opts.allowConsoleInteraction = false;
    opts.allowGuiInteraction = false;
    opts.allowAutoLogin = false;
    opts.reason = sessionEmpty ? "login device " + op : "elevated device " + op;
    const sec::Permission permission{permIt->second};

    while (true) {
        wxString title =
            sessionEmpty
                ? wxString::Format(_("Login — %s"), wxString::FromUTF8(device.label.c_str()))
                : wxString::Format(_("Authorize — %s"), wxString::FromUTF8(device.label.c_str()));
        wxString message = wxString::FromUTF8(result.message.c_str());
        std::string user;
        std::string pass;
        if (!showCredentialDialog(device, title, message, sessionEmpty, user, pass)) {
            return false;
        }

        sec::Credential cred;
        cred.meta.type = "password";
        cred.meta.subjectHint = user;
        cred.meta.serviceHint = "bas.identity.user.store";
        cred.meta.realm = device.realm;
        cred.secret = sec::SecretValue(pass);
        opts.nameHint = user;

        const auto identities = m_ctx.sm->authenticate(std::move(cred), opts);
        if (!identities.has_value()) {
            if (wxMessageBox(_("Invalid username or password."), _("Authentication failed"),
                             wxOK | wxCANCEL | wxICON_ERROR, m_frame) != wxOK) {
                return false;
            }
            continue;
        }

        const sec::Subject subject = sec::Subject::fromIdentitySet(*identities);
        if (m_ctx.sm->checkSubjectPermission(permission, subject, opts) !=
            sec::AccessEffect::Allow) {
            const wxString msg =
                wxString::Format(_("%s is not allowed to %s on %s."), user.c_str(),
                                 permission.toString().c_str(), device.name.c_str());
            if (wxMessageBox(msg, _("Access denied"), wxOK | wxCANCEL | wxICON_ERROR, m_frame) !=
                wxOK) {
                return false;
            }
            continue;
        }

        result.allowed = true;
        if (sessionEmpty) {
            m_ctx.sm->activate(*identities);
            pushLog(device.name + ": logged in as " + user);
            result.message = device.name + ": " + op + " OK";
        } else {
            result.message = device.name + ": " + op + " OK (as " + user + ")";
        }
        return true;
    }
}

void GuitanksBody::performOp(const std::string& op) {
    const std::size_t idx = activeIndex();
    if (idx >= m_ctx.devices.size()) {
        return;
    }
    const auto& device = m_ctx.devices[idx];
    OpResult res = requestOp(m_ctx, device, op);
    if (!res.allowed) {
        tryElevatedOp(device, op, res);
    }
    tankApplyAction(m_tanks[idx], op, res.allowed);
    m_tanks[idx].lastMessage = res.message;
    pushLog(res.message);
    refreshUi();
}

void GuitanksBody::doLogin(PerformContext*) {
    const std::size_t idx = activeIndex();
    if (idx >= m_ctx.devices.size()) {
        return;
    }
    const auto& device = m_ctx.devices[idx];
    while (true) {
        std::string user;
        std::string pass;
        if (!showCredentialDialog(device,
                                  wxString::Format(_("Login — %s"),
                                                   wxString::FromUTF8(device.label.c_str())),
                                  wxEmptyString, true, user, pass)) {
            pushLog(device.name + ": login cancelled");
            refreshUi();
            return;
        }
        if (loginUser(m_ctx, device, user, pass)) {
            pushLog(device.name + ": logged in as " + user);
            m_tanks[idx].lastMessage = device.name + ": session active";
            refreshUi();
            return;
        }
        pushLog("DENIED " + device.name + " login failed for " + user);
        if (wxMessageBox(wxString::Format(_("Invalid credentials for %s."),
                                          wxString::FromUTF8(device.label.c_str())),
                         _("Login failed"), wxOK | wxCANCEL | wxICON_ERROR, m_frame) != wxOK) {
            refreshUi();
            return;
        }
    }
}

GuitanksFrame::GuitanksFrame(const wxString& title) : uiFrame(title) {
    addFragment(&m_body);
    m_body.setFrameWindow(this);
    createView();
    Bind(wxEVT_CHAR_HOOK, &GuitanksBody::onCharHook, &m_body);
}

void GuitanksFrame::getDefaultAuiToolbarsSupported(std::unordered_set<std::string>& set) const {
    uiFrame::getDefaultAuiToolbarsSupported(set);
    set.insert("device");
    set.insert("tank");
}

wxAuiToolBar* GuitanksFrame::makeDefaultAuiToolbar(std::string_view path) {
    auto toolbar = uiFrame::makeDefaultAuiToolbar(path);
    if (toolbar != nullptr) {
        return toolbar;
    }
    if (path == "device") {
        if (m_deviceTools == nullptr) {
            m_deviceTools = m_buildViewContext.createAuiToolbar(this, wxID_ANY);
        }
        return m_deviceTools;
    }
    if (path == "tank") {
        if (m_tankTools == nullptr) {
            m_tankTools = m_buildViewContext.createAuiToolbar(this, wxID_ANY);
        }
        return m_tankTools;
    }
    return nullptr;
}

bool Guitanks::OnUserInit() {
    GuitanksFrame* frame = new GuitanksFrame(_("Tank Access Control"));
    frame->SetSize(920, 640);
    frame->CenterOnScreen();
    frame->Show();
    return true;
}

int main(int argc, char** argv) {
    stackdump_install_crash_handler(&stackdump_color_schema_default);
    stackdump_set_interactive(1);

    try {
        Guitanks app;
        return app.main(argc, argv);
    } catch (const sec::AccessDenied& e) {
        wxMessageBox(wxString::FromUTF8(e.what()), _("Access denied"), wxOK | wxICON_ERROR);
        return 1;
    } catch (const std::exception& e) {
        wxMessageBox(wxString::FromUTF8(e.what()), _("Error"), wxOK | wxICON_ERROR);
        return 1;
    }
}
