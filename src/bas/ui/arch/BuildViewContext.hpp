#ifndef UI_BUILD_VIEW_CONTEXT_H
#define UI_BUILD_VIEW_CONTEXT_H

#include "IdManager.hpp"

#include <bas/util/Path.hpp>

#include <wx/aui/auibar.h>
#include <wx/gdicmn.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/window.h>

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class LegacyToolBarArt : public wxAuiDefaultToolBarArt {
  public:
    void DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override {
        // Use a solid system color for that classic legacy look
        dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(rect);
    }
};

/**
 * Context that provides wx menus and toolbars by path (element.dir()).
 * Used by UIGroup::setUp() to install group/action/state into the right menus/toolbars.
 */
class BuildViewContext : public IdManager<int> {
  public:
    BuildViewContext();
    virtual ~BuildViewContext() = default;

    std::vector<wxMenuBar*> getMenubars(std::string_view path);
    std::vector<wxMenu*> getMenus(std::string_view path);
    std::vector<wxToolBar*> getToolbars(std::string_view path);
    std::vector<wxAuiToolBar*> getAuiToolbars(std::string_view path);

    void registerMenubar(std::string_view path, wxMenuBar* menubar);
    void registerMenu(std::string_view path, wxMenu* menu);
    void registerToolbar(std::string_view path, wxToolBar* toolbar);
    void registerAuiToolbar(std::string_view path, wxAuiToolBar* toolbar);

    bool isAuiPreferred() const { return m_auiPreferred; }
    void setAuiPreferred(bool v) { m_auiPreferred = v; }

    int preferredMenuIconSize() const { return 16; }
    int preferredToolIconSize() const { return 32; }
    bool toolbarSmallSize() const { return false; }
    bool toolbarShowLabel() const { return false; }

    void forMenubars(std::function<void(wxMenuBar*)> fn,
                     std::optional<std::string_view> path = std::nullopt);
    void forMenus(std::function<void(wxMenu*)> fn,
                  std::optional<std::string_view> path = std::nullopt);
    void forToolbars(std::function<void(wxToolBar*)> fn,
                     std::optional<std::string_view> path = std::nullopt);
    void forAuiToolbars(std::function<void(wxAuiToolBar*)> fn,
                        std::optional<std::string_view> path = std::nullopt);

    wxAuiToolBarArt* getAuiToolBarArt() { return &m_auiToolBarArt; }
    wxAuiToolBar* createAuiToolbar(wxWindow* parent, wxWindowID id,
                                   const wxPoint& pos = wxDefaultPosition,
                                   const wxSize& size = wxDefaultSize,
                                   long style = wxAUI_TB_DEFAULT_STYLE);

    int getNextId(std::string_view name = "", std::string_view description = "") {
        return alloc(name, description).id;
    }

    int getNextId(int parent, int index, std::string_view name = "", std::string_view description = "") {
        return alloc(parent, index, name, description).id;
    }

  private:
    std::unordered_map<std::string, std::vector<wxMenuBar*>> m_menubars;
    std::unordered_map<std::string, std::vector<wxMenu*>> m_menus;
    std::unordered_map<std::string, std::vector<wxToolBar*>> m_toolbars;
    std::unordered_map<std::string, std::vector<wxAuiToolBar*>> m_auiToolbars;

    bool m_auiPreferred;
    LegacyToolBarArt m_auiToolBarArt;
};

#endif // UI_BUILD_VIEW_CONTEXT_H
