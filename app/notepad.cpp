/**
 * Simple notepad application using ui/arch action/group models.
 * Menubar and toolbar are built from a UIFragment and UIWidgetsContext.
 */
#include "bas/proc/MyStackWalker.hpp"
#include "bas/ui/arch/UIFragment.hpp"
#include "bas/wx/app.hpp"
#include "bas/wx/i18n.hpp"
#include "bas/wx/uiframe.hpp"

#include <wx/app.h>
#include <wx/filedlg.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>
#include <wx/wfstream.h>
#include <wx/wx.h>

#include <bas/log/uselog.h>
#include <bas/proc/stackdump.h>
#include <bas/util/Path.hpp>

enum {
    ID_ZOOM_IN = uiFrame::ID_APP_HIGHEST + 1,
    ID_ZOOM_OUT,
    ID_ZOOM_RESET,
};

class NotepadBody : public UIFragment {
  public:
    explicit NotepadBody() {
        std::string dir = "streamline-vectors/core/pop/interface-essential";
        std::string dir2 = "streamline-vectors/core/pop/artificial-intelligence";

        int seq = 0;
        action(wxID_NEW, "file", "new", seq++, basUiMsg("&New"), basUiMsg("New document"))
            .icon(wxART_NEW, dir, "new-file.svg")
            .performFn([this](PerformContext* ctx) { onNew(ctx); })
            .install();
        action(wxID_OPEN, "file", "open", seq++, basUiMsg("&Open..."), basUiMsg("Open file"))
            .icon(wxART_FILE_OPEN, dir, "open-book.svg")
            .performFn([this](PerformContext* ctx) { onOpen(ctx); })
            .install();
        action(wxID_SAVE, "file", "save", seq++, basUiMsg("&Save"), basUiMsg("Save file"))
            .icon(wxART_FILE_SAVE, dir, "file-add-alternate.svg")
            .performFn([this](PerformContext* ctx) { onSave(ctx); })
            .install();
        action(wxID_SAVEAS, "file", "saveas", seq++, basUiMsg("Save &As..."), basUiMsg("Save as"))
            .icon(wxART_FILE_SAVE_AS, dir, "multiple-file-2.svg")
            .performFn([this](PerformContext* ctx) { onSaveAs(ctx); })
            .install();

        seq = 0;
        action(wxID_UNDO, "edit", "undo", seq++, basUiMsg("Undo"), basUiMsg("Undo"))
            .icon(wxART_UNDO, dir,
                  "line-arrow-reload-horizontal-1."
                  "svg")
            .performFn([this](PerformContext* ctx) { onUndo(ctx); })
            .install();
        action(wxID_REDO, "edit", "redo", seq++, basUiMsg("Redo"), basUiMsg("Redo"))
            .icon(wxART_REDO, dir2, "ai-redo-spark.svg")
            .performFn([this](PerformContext* ctx) { onRedo(ctx); })
            .install();

        seq = 1000;
        action(wxID_SELECTALL, "edit", "select_all", seq++, basUiMsg("Select &All"),
               basUiMsg("Select all"))
            .icon(wxART_REPORT_VIEW, dir, "clipboard-check.svg")
            .performFn([this](PerformContext* ctx) { onSelectAll(ctx); })
            .install();
        action(wxID_CLEAR, "edit", "clear", seq++, basUiMsg("Clear"), basUiMsg("Clear"))
            .icon(wxART_DELETE, dir, "clipboard-remove.svg")
            .performFn([this](PerformContext* ctx) { onClear(ctx); })
            .install();

        action(wxID_CUT, "edit", "cut", seq++, basUiMsg("Cu&t"), basUiMsg("Cut"))
            .icon(wxART_CUT, dir, "cut.svg")
            .performFn([this](PerformContext* ctx) { onCut(ctx); })
            .install();
        action(wxID_COPY, "edit", "copy", seq++, basUiMsg("&Copy"), basUiMsg("Copy"))
            .icon(wxART_COPY, dir, "clipboard-add.svg")
            .performFn([this](PerformContext* ctx) { onCopy(ctx); })
            .install();
        action(wxID_PASTE, "edit", "paste", seq++, basUiMsg("&Paste"), basUiMsg("Paste"))
            .icon(wxART_PASTE, dir, "empty-clipboard.svg")
            .performFn([this](PerformContext* ctx) { onPaste(ctx); })
            .install();

        seq = 2000;
        action(ID_ZOOM_IN, "view", "zoom_in", seq++, basUiMsg("Zoom &In"), basUiMsg("Zoom in"))
            .icon(wxART_PLUS, dir, "magnifying-glass-circle.svg")
            .performFn([this](PerformContext* ctx) { onZoomIn(ctx); })
            .install();
        action(ID_ZOOM_OUT, "view", "zoom_out", seq++, basUiMsg("Zoom &Out"), basUiMsg("Zoom out"))
            .icon(wxART_MINUS, dir, "magnifying-glass.svg")
            .performFn([this](PerformContext* ctx) { onZoomOut(ctx); })
            .install();
        action(ID_ZOOM_RESET, "view", "zoom_reset", seq++, basUiMsg("Zoom &Reset"),
               basUiMsg("Zoom reset"))
            .icon(wxART_CROSS_MARK, dir, "search-visual.svg")
            .performFn([this](PerformContext* ctx) { onZoomReset(ctx); })
            .install();
    }

    void createFragmentView(CreateViewContext* ctx) override {
        wxWindow* parent = ctx->getParent();
        const wxPoint& pos = ctx->getPos();
        const wxSize& size = ctx->getSize();
        m_text = new wxTextCtrl(parent, wxID_ANY, "", pos, size, wxTE_MULTILINE | wxTE_WORDWRAP);
    }

    wxEvtHandler* getEventHandler() override { return m_text->GetEventHandler(); }

  private:
    wxTextCtrl* m_text;
    wxString m_filePath;
    bool m_loaded{false};

    void onNew(PerformContext*) {
        m_text->Clear();
        m_filePath.clear();
    }

    void onOpen(PerformContext*) {
        wxString path = wxFileSelector(basUiTr("Open"), wxEmptyString, wxEmptyString, //
                                       wxEmptyString, basUiTr("All files (*.*)|*.*"), //
                                       wxFD_OPEN);
        if (path.empty())
            return;
        wxFileInputStream is(path);
        if (!is.IsOk()) {
            wxMessageBox(basUiTr("Cannot open file."), basUiTr("Error"), wxOK | wxICON_ERROR);
            return;
        }
        wxString content;
        char buf[4096];
        for (;;) {
            is.Read(buf, sizeof(buf));
            size_t n = is.LastRead();
            if (n == 0)
                break;
            std::string chunk(buf, n);
            content += wxString::FromUTF8(chunk.data(), chunk.size());
        }
        m_text->SetValue(content);
        m_filePath = path;
    }

    void onSave(PerformContext*) {
        if (m_filePath.empty()) {
            onSaveAs(nullptr);
            return;
        }
        saveTo(m_filePath);
    }

    void onSaveAs(PerformContext*) {
        wxString path = wxFileSelector(basUiTr("Save As"), wxEmptyString, wxEmptyString, //
                                       wxEmptyString, basUiTr("All files (*.*)|*.*"),    //
                                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (path.empty())
            return;
        saveTo(path);
        m_filePath = path;
    }

    void saveTo(const wxString& path) {
        wxFileOutputStream os(path);
        if (!os.IsOk()) {
            wxMessageBox(basUiTr("Cannot save file."), basUiTr("Error"), wxOK | wxICON_ERROR);
            return;
        }
        std::string utf8(m_text->GetValue().ToUTF8());
        os.Write(utf8.data(), utf8.size());
        if (!os.IsOk()) {
            wxMessageBox(basUiTr("Write failed."), basUiTr("Error"), wxOK | wxICON_ERROR);
        }
    }

    void onUndo(PerformContext*) { m_text->Undo(); }
    void onRedo(PerformContext*) { m_text->Redo(); }

    void onSelectAll(PerformContext*) { m_text->SelectAll(); }
    void onClear(PerformContext*) { m_text->Clear(); }
    void onCut(PerformContext*) { m_text->Cut(); }
    void onCopy(PerformContext*) { m_text->Copy(); }
    void onPaste(PerformContext*) { m_text->Paste(); }

    // zoom by adjusting font size
    void onZoomIn(PerformContext*) {
        int fontSize = m_text->GetFont().GetPointSize();
        m_text->SetFont(
            wxFont(fontSize + 1, wxFONTFAMILY_DEFAULT, wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL));
    }
    void onZoomOut(PerformContext*) {
        int fontSize = m_text->GetFont().GetPointSize();
        m_text->SetFont(
            wxFont(fontSize - 1, wxFONTFAMILY_DEFAULT, wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL));
    }
    void onZoomReset(PerformContext*) {
        m_text->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL));
    }
};

class NodepadFrame : public uiFrame {

  public:
    explicit NodepadFrame(const wxString& title) : uiFrame(title) {
        addFragment(&m_body);
        createView();
    }

  private:
    NotepadBody m_body;
};

class Notepad : public uiApp {
  public:
    Notepad() : uiApp() {}

    bool OnUserInit() override {
        NodepadFrame* frame = new NodepadFrame(basUiTr("Notepad"));
        frame->CenterOnScreen();
        frame->Show();
        return true;
    }
};

int main(int argc, char** argv) {
    stackdump_install_crash_handler(&stackdump_color_schema_default);
    stackdump_set_interactive(1);

    Notepad app;
    return app.main(argc, argv);
}