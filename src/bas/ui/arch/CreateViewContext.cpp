#include "CreateViewContext.hpp"

#include "../../wx/uiframe.hpp"

#include <wx/sizer.h>

CreateViewContext::CreateViewContext(wxWindowID id, wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : BuildViewContext()
    , m_id(id)
    , m_parent(parent)
    , m_title(title)
    , m_pos(pos)
    , m_size(size)
    , m_style(style)
    , m_name(name) {}

uiFrame* CreateViewContext::findParentFrame(bool error) const {
    wxWindow* parent = m_parent;
    while (parent != nullptr) {
        uiFrame* frame = dynamic_cast<uiFrame*>(parent);
        if (frame != nullptr) {
            return frame;
        }
        parent = parent->GetParent();
    }
    
    if (error) {
        wxMessageBox("Not in a uiFrame", "Error", wxOK | wxICON_ERROR);
        exit(1);
    }
    return nullptr;
}

wxSizer* CreateViewContext::sizer(wxOrientation defaultOrient) {
    wxSizer* sizer = m_parent->GetSizer();
    if (sizer == nullptr) {
        auto* boxSizer = new wxBoxSizer(defaultOrient);
        m_parent->SetSizer(boxSizer);
        sizer = boxSizer;
    }
    return sizer;
}

void CreateViewContext::addContent(wxWindow* content, bool layout, wxOrientation orient) {
    assert(content != nullptr);
    wxSizer* rootSizer = sizer(orient);
    rootSizer->Add(content, 1, wxEXPAND);
    if (layout) {
        m_parent->Layout();
    }
}
