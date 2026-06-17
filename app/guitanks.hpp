#pragma once

#include "tanks_rbac.hpp"
#include "tanks_sim.hpp"

#include "bas/security/wxLoginUi.hpp"
#include "bas/wx/app.hpp"
#include "bas/wx/uiframe.hpp"

#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/timer.h>

#include <deque>
#include <string>
#include <vector>

enum {
    ID_DEVICE_SWITCH = uiFrame::ID_APP_HIGHEST + 1,
    ID_DEVICE_LOGIN,
    ID_TANK_START,
    ID_TANK_STOP,
    ID_TANK_FIRE,
    ID_TANK_FORWARD,
    ID_TANK_BACKWARD,
    ID_TANK_LEFT,
    ID_TANK_RIGHT,
};

class TankCanvas : public wxPanel {
  public:
    explicit TankCanvas(wxWindow* parent);

    void setState(const TankState& state);

  private:
    void onPaint(wxPaintEvent& event);
    void onSize(wxSizeEvent& event);
    void drawTankShape(wxDC& dc, const wxPoint& center, int cellSize, int facing, bool engine);

    TankState m_state;
};

class GuitanksBody : public UIFragment {
  public:
    explicit GuitanksBody();

    void defineActions();
    wxWindow* createFragmentView(CreateViewContext* ctx) override;

    void setFrameWindow(wxWindow* parent);
    void onCharHook(wxKeyEvent& event);

  private:
    DemoContext m_ctx;
    std::vector<TankState> m_tanks;
    std::deque<std::string> m_log;

    wxNotebook* m_notebook{nullptr};
    wxPanel* m_sessionPanel{nullptr};
    wxStaticText* m_deviceBadge{nullptr};
    wxStaticText* m_userBadge{nullptr};
    wxStaticText* m_roleBadge{nullptr};
    wxStaticText* m_facingBadge{nullptr};
    wxStaticText* m_engineBadge{nullptr};
    wxStaticText* m_statusText{nullptr};
    wxNotebook* m_policyBook{nullptr};
    wxListCtrl* m_bindingsList{nullptr};
    wxListCtrl* m_aclsList{nullptr};
    wxListCtrl* m_grantsList{nullptr};
    wxListCtrl* m_logList{nullptr};
    std::vector<TankCanvas*> m_canvases;
    wxTimer* m_timer{nullptr};
    wxWindow* m_frame{nullptr};

    std::string m_policyCacheKey;
    bool m_opInProgress{false};

    void initContext();
    std::size_t activeIndex() const;
    std::string policyCacheKey() const;
    void pushLog(const std::string& line);
    void refreshCanvases();
    void refreshUi();
    void refreshSessionBar();
    void refreshPolicyPanel();
    void refreshLog();
    void onNotebookChanged(wxBookCtrlEvent& event);
    void onTimer(wxTimerEvent& event);

    bool showCredentialDialog(const DeviceSlot& device, const wxString& title,
                              const wxString& message, bool asLogin, std::string& user,
                              std::string& pass);
    bool tryElevatedOp(const DeviceSlot& device, const std::string& op, OpResult& result);
    void performOp(const std::string& op);
    void switchDevice();

    void doSwitchDevice(PerformContext*) { switchDevice(); }
    void doLogin(PerformContext*);
    void doStart(PerformContext*) { performOp("start"); }
    void doStop(PerformContext*) { performOp("stop"); }
    void doFire(PerformContext*) { performOp("fire"); }
    void doForward(PerformContext*) { performOp("forward"); }
    void doBackward(PerformContext*) { performOp("backward"); }
    void doLeft(PerformContext*) { performOp("left"); }
    void doRight(PerformContext*) { performOp("right"); }

    wxPanel* buildDevicePage(const DeviceSlot& device);
    wxListCtrl* makePolicyList(wxWindow* parent, const std::vector<wxString>& columns);
};

class GuitanksFrame : public uiFrame {
  public:
    explicit GuitanksFrame(const wxString& title);

    void getDefaultAuiToolbarsSupported(std::unordered_set<std::string>& set) const override;
    wxAuiToolBar* makeDefaultAuiToolbar(std::string_view path) override;

  private:
    GuitanksBody m_body;
    wxAuiToolBar* m_deviceTools{nullptr};
    wxAuiToolBar* m_tankTools{nullptr};
};

class Guitanks : public uiApp {
  public:
    Guitanks() : uiApp() {}

    bool OnUserInit() override;
};
