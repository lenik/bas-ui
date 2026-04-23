#pragma once

#include "text.hpp"

#include "bas/wx/app.hpp"
#include "bas/wx/uiframe.hpp"

enum {
    ID_ZOOM_IN = uiFrame::ID_APP_HIGHEST + 1,
    ID_ZOOM_OUT,
    ID_ZOOM_RESET,
    ID_EDIT_EOL_MODE,
    ID_EDIT_INDENT_MODE,
    ID_EDIT_TAB_SIZE,
};

class NotepadBody : public UIFragment {
  public:
    explicit NotepadBody();

    void defineActions();
    void defineStates();

    void createFragmentView(CreateViewContext* ctx) override;

    wxEvtHandler* getEventHandler() override { return m_text->GetEventHandler(); }

  private:
    wxTextCtrl* m_text;
    wxString m_filePath;
    bool m_loaded{false};
    EolMode m_eolMode{EOL_AUTO};
    EolMode m_detectedEolMode{EOL_LINUX};
    IndentMode m_indentMode{INDENT_AUTO_SPACE};
    int m_tabSize{4};

    void doNew(PerformContext*);
    void doOpen(PerformContext*);
    void doSave(PerformContext*);
    void doSaveAs(PerformContext*);
    void saveTo(const wxString& path);
    void doUndo(PerformContext*) { m_text->Undo(); }
    void doRedo(PerformContext*) { m_text->Redo(); }
    void doSelectAll(PerformContext*) { m_text->SelectAll(); }
    void doClear(PerformContext*) { m_text->Clear(); }
    void doCut(PerformContext*) { m_text->Cut(); }
    void doCopy(PerformContext*) { m_text->Copy(); }
    void doPaste(PerformContext*) { m_text->Paste(); }
    void doZoomIn(PerformContext*);
    void doZoomOut(PerformContext*);
    void doZoomReset(PerformContext*);

  private:
    void onTextChar(wxKeyEvent& event);
};

class NotepadFrame : public uiFrame {

  public:
    explicit NotepadFrame(const wxString& title) : uiFrame(title) {
        addFragment(&m_body);
        createView();
    }

    void getDefaultToolbarsSupported(std::unordered_set<std::string>& set) const override {
        uiFrame::getDefaultToolbarsSupported(set);
        // Don't add edit toolbar for legacy mode toolbar.
    }

    void getDefaultAuiToolbarsSupported(std::unordered_set<std::string>& set) const override {
        uiFrame::getDefaultAuiToolbarsSupported(set);
        set.insert("edit");
        set.insert("view");
    }

    wxAuiToolBar* makeDefaultAuiToolbar(std::string_view path) override;

  private:
    NotepadBody m_body;
    wxAuiToolBar* m_fileTools{nullptr};
    wxAuiToolBar* m_editTools{nullptr};
    wxAuiToolBar* m_viewTools{nullptr};
};

class Notepad : public uiApp {
  public:
    Notepad() : uiApp() {}

    bool OnUserInit() override {
        NotepadFrame* frame = new NotepadFrame(_("Notepad"));
        frame->CenterOnScreen();
        frame->Show();
        return true;
    }
};
