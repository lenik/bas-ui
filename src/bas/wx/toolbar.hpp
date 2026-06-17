#ifndef TOOLBAR_HPP
#define TOOLBAR_HPP

#include "wx_compat.hpp"

#include <wx/aui/auibar.h>
#include <wx/toolbar.h>

#include <functional>

namespace wx {

class ToolBar {
  public:
    ToolBar(wxToolBar* toolbar) : wx(toolbar) {}
    ToolBar(wxAuiToolBar* toolbar) : aui(toolbar) {}

    ToolBar(const ToolBar& other) : wx(other.wx), aui(other.aui) {}
    ToolBar(ToolBar&& other) : wx(other.wx), aui(other.aui) {}

    ToolBar& operator=(const ToolBar& other) {
        wx = other.wx;
        aui = other.aui;
        return *this;
    }
    ToolBar& operator=(ToolBar&& other) {
        wx = other.wx;
        aui = other.aui;
        return *this;
    }

    ~ToolBar() {
        wx = nullptr;
        aui = nullptr;
    }

    bool operator==(const ToolBar& other) const { return wx == other.wx && aui == other.aui; }
    bool operator!=(const ToolBar& other) const { return !(*this == other); }

    // operator wxToolBar*() const { return wx; }
    // operator wxAuiToolBar*() const { return aui; }

    bool isWx() const { return wx != nullptr; }
    bool isAui() const { return aui != nullptr; }

    const wxToolBar* getWx() const { return wx; }
    const wxAuiToolBar* getAui() const { return aui; }

    wxToolBar* getWx() { return wx; }
    wxAuiToolBar* getAui() { return aui; }

    void AddTool(int id, const wxString& label, const wxBitmap& bitmap, const wxString& help,
                 wxItemKind kind) {
        if (wx)
            basWxToolBarAddTool(wx, id, label, bitmap, help, kind);
        else
            basWxAuiToolBarAddTool(aui, id, label, bitmap, help, kind);
    }

    template <typename EventTag, typename Functor>
    void Bind(const EventTag& type, const Functor& fn, int id) {
        if (wx)
            wx->Bind(type, fn, id);
        else
            aui->Bind(type, fn, id);
    }

    void SetToolLabel(int id, const wxString& label) {
        if (wx)
            wx->FindById(id)->SetLabel(label);
        else
            aui->SetToolLabel(id, label);
    }
    void SetToolBitmap(int id, const wxBitmap& bitmap) {
        if (wx)
            basWxToolBarSetNormalBitmap(wx, id, bitmap);
        else
            basWxAuiToolBarSetBitmap(aui, id, bitmap);
    }
    void ToggleTool(int id, bool state) {
        if (wx)
            wx->ToggleTool(id, state);
        else
            aui->ToggleTool(id, state);
    }
    bool GetToolToggled(int id) {
        if (wx)
            return wx->GetToolState(id);
        else
            return aui->GetToolToggled(id);
    }

    wxToolBarToolBase* addNecessarySeparator_wx();
    wxAuiToolBarItem* addNecessarySeparator_aui();
    void* addNecessarySeparator() {
        if (wx)
            return addNecessarySeparator_wx();
        else
            return addNecessarySeparator_aui();
    }

    void dump(std::string prefix, std::ostream& out = std::cout);

  private:
    wxToolBar* wx{nullptr};
    wxAuiToolBar* aui{nullptr};
};

} // namespace wx

#endif // TOOLBAR_HPP