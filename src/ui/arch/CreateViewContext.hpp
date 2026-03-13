#ifndef UI_CREATE_VIEW_CONTEXT_HPP
#define UI_CREATE_VIEW_CONTEXT_HPP

#include "BuildViewContext.hpp"

class CreateViewContext : public BuildViewContext {
  public:
    CreateViewContext() = default;
    CreateViewContext(wxWindowID id,                          //
                      wxWindow* parent,                       //
                      const wxString& title,                  //
                      const wxPoint& pos = wxDefaultPosition, //
                      const wxSize& size = wxDefaultSize,     //
                      long style = wxDEFAULT,                 //
                      const wxString& name = wxFrameNameStr   //
                      )
        : m_id(id), m_parent(parent), m_title(title), m_pos(pos), m_size(size), m_style(style),
          m_name(name) {}
    virtual ~CreateViewContext() = default;

    wxWindow* getParent() const { return m_parent; }
    wxWindowID getId() const { return m_id; }
    const wxString& getTitle() const { return m_title; }
    const wxPoint& getPos() const { return m_pos; }
    const wxSize& getSize() const { return m_size; }
    long getStyle() const { return m_style; }
    const wxString& getName() const { return m_name; }

    CreateViewContext& id(wxWindowID id) {
        m_id = id;
        return *this;
    }
    CreateViewContext& parent(wxWindow* parent) {
        m_parent = parent;
        return *this;
    }
    CreateViewContext& title(const wxString& title) {
        m_title = title;
        return *this;
    }
    CreateViewContext& pos(const wxPoint& pos) {
        m_pos = pos;
        return *this;
    }
    CreateViewContext& size(const wxSize& size) {
        m_size = size;
        return *this;
    }
    CreateViewContext& style(long style) {
        m_style = style;
        return *this;
    }
    CreateViewContext& name(const wxString& name) {
        m_name = name;
        return *this;
    }

  private:
    wxWindowID m_id{wxID_ANY};
    wxWindow* m_parent{nullptr};
    wxString m_title{""};
    wxPoint m_pos{wxDefaultPosition};
    wxSize m_size{wxDefaultSize};
    long m_style{wxDEFAULT};
    wxString m_name{wxFrameNameStr};
};

#endif // UI_CREATE_VIEW_CONTEXT_HPP
