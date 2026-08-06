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
#include <wx/listctrl.h>
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

constexpr int kLogMaxLines = 80;

wxString utf8(const std::string& text) { return wxString::FromUTF8(text.c_str()); }

wxString badgeLabel(const std::string& text) { return wxString(" ") + utf8(text) + wxString(" "); }

wxString loginDialogTitle(const std::string& deviceLabel) {
    return wxString(_("Login — ")) + utf8(deviceLabel);
}

wxString authorizeDialogTitle(const std::string& deviceLabel) {
    return wxString(_("Authorize — ")) + utf8(deviceLabel);
}

wxString accessDeniedMessage(const std::string& user, const std::string& permission,
                             const std::string& deviceName) {
    return utf8(user) + _(" is not allowed to ") + utf8(permission) + _(" on ") +
           utf8(deviceName) + ".";
}

class DeviceLoginDialog : public wxDialog, public bas::ui::automation::Automatable {
  public:
    DeviceLoginDialog(wxWindow* parent, const DeviceSlot& device, const wxString& title,
                      const wxString& message)
        : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
          Automatable(this) {
        auto* root = new wxBoxSizer(wxVERTICAL);
        if (!message.empty()) {
            root->Add(new wxStaticText(this, wxID_ANY, message), 0, wxALL | wxEXPAND, 10);
        }

        auto* grid = new wxFlexGridSizer(2, wxSize(8, 6));
        grid->Add(new wxStaticText(this, wxID_ANY, _("Username:")), 0, wxALIGN_CENTER_VERTICAL);
        m_user = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(220, -1));
        m_user->SetName("user");
        grid->Add(m_user, 1, wxEXPAND);
        grid->Add(new wxStaticText(this, wxID_ANY, _("Password:")), 0, wxALIGN_CENTER_VERTICAL);
        m_pass = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(220, -1),
                                wxTE_PASSWORD);
        m_pass->SetName("pass");
        grid->Add(m_pass, 1, wxEXPAND);
        root->Add(grid, 0, wxALL | wxEXPAND, 10);

        auto* demoBox =
            new wxStaticBoxSizer(wxVERTICAL, this, _("Demo accounts (user / pass / role)"));
        for (const DemoAccount& acct : demoAccountsFor(device)) {
            wxString line = wxString(acct.user) + " / " + wxString(acct.password) + "  (" +
                            wxString(acct.role) + ")";
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

        automationMap().bind("user", m_user);
        automationMap().bind("pass", m_pass);
        automationMap().bind("ok", wxID_OK);
        automationMap().bind("cancel", wxID_CANCEL);
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

wxStaticText* makeBadge(wxWindow* parent, const wxString& label, const wxColour& bg,
                        const wxColour& fg) {
    auto* badge = new wxStaticText(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize,
                                   wxST_NO_AUTORESIZE | wxALIGN_CENTER);
    badge->SetBackgroundColour(bg);
    badge->SetForegroundColour(fg);
    badge->SetMinSize(wxSize(72, 22));
    return badge;
}

} // namespace

TankCanvas::TankCanvas(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetMinSize(wxSize(320, 280));
    Bind(wxEVT_PAINT, &TankCanvas::onPaint, this);
    Bind(wxEVT_SIZE, &TankCanvas::onSize, this);
}

void TankCanvas::setState(const TankState& state) {
    m_state = state;
    Refresh(false);
}

void TankCanvas::onSize(wxSizeEvent& event) {
    Refresh(false);
    event.Skip();
}

void TankCanvas::drawTankShape(wxDC& dc, const wxPoint& center, int cellSize, int facing,
                               bool engine) {
    const int r = std::max(8, cellSize / 2);
    wxPoint tip = center;
    wxPoint left = center;
    wxPoint right = center;
    switch (facing) {
    case 0:
        tip.y -= r;
        left.x -= r / 2;
        left.y += r / 2;
        right.x += r / 2;
        right.y += r / 2;
        break;
    case 1:
        tip.x += r;
        left.x -= r / 2;
        left.y -= r / 2;
        right.x -= r / 2;
        right.y += r / 2;
        break;
    case 2:
        tip.y += r;
        left.x -= r / 2;
        left.y -= r / 2;
        right.x += r / 2;
        right.y -= r / 2;
        break;
    default:
        tip.x -= r;
        left.x += r / 2;
        left.y -= r / 2;
        right.x += r / 2;
        right.y += r / 2;
        break;
    }

    wxPoint tri[3] = {tip, left, right};
    dc.SetPen(wxPen(engine ? wxColour(255, 210, 80) : wxColour(150, 155, 165), 2));
    dc.SetBrush(wxBrush(engine ? wxColour(210, 170, 40) : wxColour(110, 115, 125)));
    dc.DrawPolygon(3, tri);
}

void TankCanvas::onPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    const wxSize size = GetClientSize();
    if (size.x < 8 || size.y < 8) {
        event.Skip();
        return;
    }

    dc.SetBackground(wxBrush(wxColour(24, 28, 34)));
    dc.Clear();

    const int pad = 14;
    const int footerH = 28;
    const int fieldW = size.x - pad * 2;
    const int fieldH = size.y - pad * 2 - footerH;
    const int cellW = std::max(14, fieldW / kTankFieldW);
    const int cellH = std::max(14, fieldH / kTankFieldH);
    const int originX = (size.x - cellW * kTankFieldW) / 2;
    const int originY = pad + (fieldH - cellH * kTankFieldH) / 2;

    dc.SetPen(wxPen(wxColour(70, 80, 95)));
    dc.SetBrush(wxBrush(wxColour(32, 38, 46)));
    dc.DrawRectangle(originX - 2, originY - 2, cellW * kTankFieldW + 4, cellH * kTankFieldH + 4);

    for (int y = 0; y < kTankFieldH; ++y) {
        for (int x = 0; x < kTankFieldW; ++x) {
            const bool alt = (x + y) % 2 == 0;
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(alt ? wxColour(38, 44, 52) : wxColour(34, 40, 48)));
            dc.DrawRectangle(originX + x * cellW, originY + y * cellH, cellW, cellH);
        }
    }

    auto cellCenter = [&](int gx, int gy) {
        return wxPoint(originX + gx * cellW + cellW / 2, originY + gy * cellH + cellH / 2);
    };

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(wxColour(230, 70, 60)));
    for (const auto& bullet : m_state.bullets) {
        const wxPoint c = cellCenter(bullet.x, bullet.y);
        dc.DrawCircle(c, std::max(4, std::min(cellW, cellH) / 4));
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
        dc.DrawCircle(muzzle, std::max(5, std::min(cellW, cellH) / 3));
    }

    const wxPoint tankCenter = cellCenter(m_state.x, m_state.y);
    drawTankShape(dc, tankCenter, std::min(cellW, cellH), m_state.facing, m_state.engine);

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(wxColour(18, 22, 28)));
    dc.DrawRectangle(0, size.y - footerH, size.x, footerH);
    dc.SetTextForeground(wxColour(170, 180, 195));
    dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.DrawText(_("Tab device  L login  F1/F2 engine  arrows move  Space fire"), pad,
                size.y - footerH + 7);
    event.Skip();
}

GuitanksBody::GuitanksBody() { defineActions(); }

void GuitanksBody::defineActions() {
    // Explicit menu groups so titles are translated (internal auto-groups capitalize the name only).
    group(ID_GROUP_DEVICE, "", "device", 10, _("&Device")).install();
    group(ID_GROUP_TANK, "", "tank", 20, _("&Tank")).install();

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
    pushLog(_("tank-a: alice drives, bob fires, admin has full access."));
    pushLog(_("tank-b: charlie trains, dana instructs."));
}

std::size_t GuitanksBody::activeIndex() const {
    if (!m_notebook) {
        return 0;
    }
    return static_cast<std::size_t>(std::max(0, m_notebook->GetSelection()));
}

std::string GuitanksBody::policyCacheKey() const {
    const std::size_t idx = activeIndex();
    if (idx >= m_ctx.devices.size()) {
        return {};
    }
    const auto& device = m_ctx.devices[idx];
    return device.name + "|" + primaryUser(*m_ctx.sm, device.realm) + "|" +
           primaryRole(*m_ctx.sm, device.realm);
}

wxListCtrl* GuitanksBody::makePolicyList(wxWindow* parent,
                                         const std::vector<wxString>& columns) {
    auto* list =
        new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
    for (std::size_t i = 0; i < columns.size(); ++i) {
        list->AppendColumn(columns[i], i == 0 ? wxLIST_FORMAT_LEFT : wxLIST_FORMAT_LEFT,
                           i == 0 ? 140 : 120);
    }
    list->SetMinSize(wxSize(240, 120));
    return list;
}

wxPanel* GuitanksBody::buildDevicePage(const DeviceSlot& /*device*/) {
    auto* page = new wxPanel(m_notebook);
    auto* canvas = new TankCanvas(page);
    m_canvases.push_back(canvas);

    auto* root = new wxBoxSizer(wxVERTICAL);
    root->Add(canvas, 1, wxEXPAND | wxALL, 6);
    page->SetSizer(root);
    return page;
}

wxWindow* GuitanksBody::createFragmentView(CreateViewContext* ctx) {
    auto* panel = new wxPanel(ctx->getParent());
    auto* root = new wxBoxSizer(wxVERTICAL);

    m_sessionPanel = new wxPanel(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
    m_sessionPanel->SetBackgroundColour(wxColour(245, 247, 250));
    auto* sessionRow = new wxBoxSizer(wxHORIZONTAL);
    sessionRow->Add(new wxStaticText(m_sessionPanel, wxID_ANY, _("Session")), 0,
                    wxALIGN_CENTER_VERTICAL | wxLEFT, 10);
    m_deviceBadge = makeBadge(m_sessionPanel, wxEmptyString, wxColour(55, 90, 150), *wxWHITE);
    m_userBadge = makeBadge(m_sessionPanel, wxEmptyString, wxColour(70, 120, 80), *wxWHITE);
    m_roleBadge = makeBadge(m_sessionPanel, wxEmptyString, wxColour(110, 90, 150), *wxWHITE);
    m_facingBadge = makeBadge(m_sessionPanel, wxEmptyString, wxColour(90, 90, 95), *wxWHITE);
    m_engineBadge = makeBadge(m_sessionPanel, wxEmptyString, wxColour(130, 90, 40), *wxWHITE);
    sessionRow->Add(m_deviceBadge, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
    sessionRow->Add(m_userBadge, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    sessionRow->Add(m_roleBadge, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    sessionRow->Add(m_facingBadge, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    sessionRow->Add(m_engineBadge, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
    sessionRow->AddStretchSpacer();
    m_statusText = new wxStaticText(m_sessionPanel, wxID_ANY, wxEmptyString);
    sessionRow->Add(m_statusText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    m_sessionPanel->SetSizer(sessionRow);
    root->Add(m_sessionPanel, 0, wxEXPAND | wxALL, 8);

    m_notebook = new wxNotebook(panel, wxID_ANY);
    m_canvases.clear();
    for (const auto& device : m_ctx.devices) {
        m_notebook->AddPage(buildDevicePage(device), wxString::FromUTF8(device.label.c_str()));
    }
    m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &GuitanksBody::onNotebookChanged, this);

    m_policyBook = new wxNotebook(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                  wxNB_LEFT | wxNB_NOPAGETHEME);
    m_bindingsList = makePolicyList(m_policyBook, {_("Identity"), _("ACL"), _("Active")});
    m_aclsList = makePolicyList(m_policyBook, {_("ACL"), _("Permission"), _("Effect")});
    m_grantsList = makePolicyList(m_policyBook, {_("Identity"), _("Permission"), _("Effect")});
    m_policyBook->AddPage(m_bindingsList, _("Bindings"));
    m_policyBook->AddPage(m_aclsList, _("ACLs"));
    m_policyBook->AddPage(m_grantsList, _("Grants"));
    m_policyBook->SetMinSize(wxSize(280, 260));

    auto* middle = new wxBoxSizer(wxHORIZONTAL);
    middle->Add(m_notebook, 3, wxEXPAND | wxRIGHT, 8);
    middle->Add(m_policyBook, 2, wxEXPAND);
    root->Add(middle, 4, wxEXPAND | wxLEFT | wxRIGHT, 8);

    auto* logBox = new wxStaticBoxSizer(wxVERTICAL, panel, _("Recent activity"));
    m_logList = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(-1, 96),
                               wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
    m_logList->AppendColumn(_("Message"), wxLIST_FORMAT_LEFT, 640);
    logBox->Add(m_logList, 1, wxEXPAND | wxALL, 6);
    root->Add(logBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

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
    const long existing = m_logList->GetItemCount();
    const long target = static_cast<long>(m_log.size());
    if (existing == target) {
        return;
    }
    if (existing > target) {
        m_logList->DeleteAllItems();
    }
    long row = existing > target ? 0 : existing;
    for (std::size_t i = static_cast<std::size_t>(row); i < m_log.size(); ++i) {
        m_logList->InsertItem(row, wxString::FromUTF8(m_log[i].c_str()));
        ++row;
    }
    if (m_logList->GetItemCount() > 0) {
        m_logList->EnsureVisible(m_logList->GetItemCount() - 1);
    }
}

void GuitanksBody::refreshSessionBar() {
    if (!m_deviceBadge || !m_statusText) {
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

    m_deviceBadge->SetLabel(badgeLabel(device.label));
    m_userBadge->SetLabel(badgeLabel(user));
    m_roleBadge->SetLabel(badgeLabel(role));
    m_facingBadge->SetLabel(badgeLabel(tankFacingName(tank.facing)));
    m_engineBadge->SetLabel(tank.engine ? _(" ON ") : _(" OFF "));
    m_engineBadge->SetBackgroundColour(tank.engine ? wxColour(40, 120, 70) : wxColour(120, 70, 40));

    wxString status = wxString::FromUTF8(tank.lastMessage.c_str());
    if (status.empty()) {
        status = _("Use toolbar or keyboard shortcuts to operate the tank.");
    }
    m_statusText->SetLabel(status);
    m_statusText->SetForegroundColour(
        tank.lastMessage.rfind("DENIED", 0) == 0 ? wxColour(180, 50, 50) : wxColour(50, 50, 55));
}

void GuitanksBody::refreshPolicyPanel() {
    if (!m_bindingsList || !m_aclsList || !m_grantsList) {
        return;
    }
    const std::string key = policyCacheKey();
    if (key == m_policyCacheKey) {
        return;
    }
    m_policyCacheKey = key;

    const std::size_t idx = activeIndex();
    if (idx >= m_ctx.devices.size()) {
        return;
    }
    const auto& device = m_ctx.devices[idx];
    const auto* policy = defaultPolicyFor(m_ctx, device.realm);

    m_bindingsList->DeleteAllItems();
    if (policy && !policy->bindings().empty()) {
        long row = 0;
        for (const auto& binding : policy->bindings()) {
            const bool active = identityMatchesCurrent(m_ctx, device.realm, binding.identity);
            m_bindingsList->InsertItem(row, wxString::FromUTF8(formatIdentity(binding.identity).c_str()));
            m_bindingsList->SetItem(row, 1, wxString::FromUTF8(binding.aclId.c_str()));
            if (active) {
                m_bindingsList->SetItem(row, 2, _("yes"));
            }
            ++row;
        }
    }

    m_aclsList->DeleteAllItems();
    if (policy && !policy->acls().empty()) {
        long row = 0;
        for (const auto& acl : policy->acls()) {
            const bool aclActive =
                isAclActiveForCurrent(m_ctx, device.realm, policy, acl.id);
            for (const auto& entry : acl.entries) {
                wxString aclLabel = wxString::FromUTF8(acl.id.c_str());
                if (aclActive) {
                    aclLabel += " *";
                }
                m_aclsList->InsertItem(row, aclLabel);
                m_aclsList->SetItem(row, 1, wxString::FromUTF8(entry.permission.toString().c_str()));
                wxString effect = policyEffectChar(entry.effect) == 'A' ? _("Allow") : _("Deny");
                m_aclsList->SetItem(row, 2, effect);
                ++row;
            }
        }
    }

    m_grantsList->DeleteAllItems();
    if (policy && !policy->grants().empty()) {
        long row = 0;
        for (const auto& grant : policy->grants()) {
            const bool highlight = shouldHighlightGrant(m_ctx, device.realm, grant);
            wxString identity = wxString::FromUTF8(formatIdentity(grant.identity).c_str());
            if (highlight) {
                identity += " *";
            }
            m_grantsList->InsertItem(row, identity);
            m_grantsList->SetItem(row, 1, wxString::FromUTF8(grant.permission.toString().c_str()));
            wxString effect = policyEffectChar(grant.effect) == 'A' ? _("Allow") : _("Deny");
            m_grantsList->SetItem(row, 2, effect);
            ++row;
        }
    }
}

void GuitanksBody::refreshCanvases() {
    for (std::size_t i = 0; i < m_canvases.size() && i < m_tanks.size(); ++i) {
        m_canvases[i]->setState(m_tanks[i]);
    }
}

void GuitanksBody::refreshUi() {
    refreshCanvases();
    refreshSessionBar();
    m_policyCacheKey.clear();
    refreshPolicyPanel();
}

void GuitanksBody::onNotebookChanged(wxBookCtrlEvent& event) {
    (void)event;
    refreshSessionBar();
    m_policyCacheKey.clear();
    refreshPolicyPanel();
}

void GuitanksBody::switchDevice() {
    if (!m_notebook || m_notebook->GetPageCount() == 0) {
        return;
    }
    const int next = (m_notebook->GetSelection() + 1) % m_notebook->GetPageCount();
    m_notebook->SetSelection(next);
    pushLog("switch -> " + m_ctx.devices[static_cast<std::size_t>(next)].name);
    refreshSessionBar();
    m_policyCacheKey.clear();
    refreshPolicyPanel();
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
    refreshCanvases();
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
            sessionEmpty ? loginDialogTitle(device.label) : authorizeDialogTitle(device.label);
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
                accessDeniedMessage(user, permission.toString(), device.name);
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
    if (m_opInProgress) {
        return;
    }
    m_opInProgress = true;
    struct Guard {
        bool& flag;
        ~Guard() { flag = false; }
    } guard{m_opInProgress};

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
    refreshCanvases();
    refreshSessionBar();
    m_policyCacheKey.clear();
    refreshPolicyPanel();
}

void GuitanksBody::doLogin(PerformContext*) {
    if (m_opInProgress) {
        return;
    }
    m_opInProgress = true;
    struct Guard {
        bool& flag;
        ~Guard() { flag = false; }
    } guard{m_opInProgress};

    const std::size_t idx = activeIndex();
    if (idx >= m_ctx.devices.size()) {
        return;
    }
    const auto& device = m_ctx.devices[idx];
    while (true) {
        std::string user;
        std::string pass;
        if (!showCredentialDialog(device, loginDialogTitle(device.label), wxEmptyString, true,
                                  user, pass)) {
            pushLog(device.name + ": login cancelled");
            refreshSessionBar();
            return;
        }
        if (loginUser(m_ctx, device, user, pass)) {
            pushLog(device.name + ": logged in as " + user);
            m_tanks[idx].lastMessage = device.name + ": session active";
            refreshUi();
            return;
        }
        pushLog("DENIED " + device.name + " login failed for " + user);
        if (wxMessageBox(utf8(device.label) + _(": invalid credentials."), _("Login failed"),
                         wxOK | wxCANCEL | wxICON_ERROR, m_frame) != wxOK) {
            refreshSessionBar();
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
    frame->SetSize(980, 680);
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
