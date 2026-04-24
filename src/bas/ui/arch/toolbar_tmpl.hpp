
template <typename T> class ToolBar {
  public:
    ToolBar(T* impl) : tb(impl) {}
    void AddTool(int id, const wxString& label, const wxBitmap& bitmap, const wxString& help,
                 wxItemKind kind) {
        tb->AddTool(id, label, bitmap, help, kind);
    }
    void Bind(int type, std::function<void(wxCommandEvent&)> fn, int id) { tb->Bind(type, fn, id); }
    void SetToolLabel(int id, const wxString& label) { tb->SetToolLabel(id, label); }
    void SetToolBitmap(int id, const wxBitmap& bitmap) { tb->SetToolBitmap(id, bitmap); }
    void ToggleTool(int id, bool state) { tb->ToggleTool(id, state); }
    bool GetToolToggled(int id) { return tb->GetToolToggled(id); }

    operator T*() const { return tb; }
    T* getWrapped() const { return tb; }
    // T* operator->() const { return tb; }

  private:
    T* tb{nullptr};
};

template <> void ToolBar<wxToolBar>::SetToolLabel(int id, const wxString& label) {
    tb->FindById(id)->SetLabel(label);
}

using SysToolBar = ToolBar<wxToolBar>;
using AuiToolBar = ToolBar<wxAuiToolBar>;

