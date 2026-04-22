/**
 * Simple notepad application using ui/arch action/group models.
 * Menubar and toolbar are built from a UIFragment and UIWidgetsContext.
 */
#include "bas/proc/MyStackWalker.hpp"
#include "bas/ui/arch/UIFragment.hpp"
#include "bas/wx/app.hpp"
#include "bas/wx/uiframe.hpp"

#include <libintl.h>

#include <wx/app.h>
#include <wx/aui/auibar.h>
#include <wx/filedlg.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>
#include <wx/wfstream.h>
#include <wx/wx.h>

#include <bas/log/uselog.h>
#include <bas/proc/stackdump.h>
#include <bas/util/Path.hpp>

#include "module.def"

#define _(s) dgettext(TEXT_DOMAIN, (s))

enum {
    ID_ZOOM_IN = uiFrame::ID_APP_HIGHEST + 1,
    ID_ZOOM_OUT,
    ID_ZOOM_RESET,
};

class NotepadBody : public UIFragment {
  public:
    explicit NotepadBody() {
        std::string dir1 = "streamline-vectors/core/pop/interface-essential";
        std::string dir2 = "streamline-vectors/core/pop/artificial-intelligence";

        int seq = 0;
        action(wxID_NEW, "file", "new", seq++, _("&New"), "New document")
            .icon(wxART_NEW, dir1, "new-file.svg")
            .shortcut("Ctrl+N")
            .performFn([this](PerformContext* ctx) { onNew(ctx); })
            .install();
        action(wxID_OPEN, "file", "open", seq++, _("&Open..."), "Open file")
            .icon(wxART_FILE_OPEN, dir1, "open-book.svg")
            .shortcut("Ctrl+O")
            .performFn([this](PerformContext* ctx) { onOpen(ctx); })
            .install();
        action(wxID_SAVE, "file", "save", seq++, _("&Save"), "Save file")
            .icon(wxART_FILE_SAVE, dir1, "file-add-alternate.svg")
            .shortcut("Ctrl+S")
            .performFn([this](PerformContext* ctx) { onSave(ctx); })
            .install();
        action(wxID_SAVEAS, "file", "saveas", seq++, _("Save &As..."), "Save as")
            .icon(wxART_FILE_SAVE_AS, dir1, "multiple-file-2.svg")
            .shortcut("Ctrl+Shift+S")
            .performFn([this](PerformContext* ctx) { onSaveAs(ctx); })
            .install();

        seq = 0;
        action(wxID_UNDO, "edit", "undo", seq++, _("Undo"), "Undo")
            .icon(wxART_UNDO, dir1, "line-arrow-reload-horizontal-1.svg")
            .shortcut("Ctrl+Z")
            .performFn([this](PerformContext* ctx) { onUndo(ctx); })
            .install();
        action(wxID_REDO, "edit", "redo", seq++, _("Redo"), "Redo")
            .icon(wxART_REDO, dir2, "ai-redo-spark.svg")
            .shortcut("Ctrl+Y")
            .performFn([this](PerformContext* ctx) { onRedo(ctx); })
            .install();

        seq = 1000;
        action(wxID_SELECTALL, "edit", "select_all", seq++, _("Select &All"),
               "Select all")
            .icon(wxART_REPORT_VIEW, dir1, "clipboard-check.svg")
            .shortcut("Ctrl+A")
            .performFn([this](PerformContext* ctx) { onSelectAll(ctx); })
            .install();
        action(wxID_CLEAR, "edit", "clear", seq++, _("Clear"), "Clear")
            .icon(wxART_DELETE, dir1, "clipboard-remove.svg")
            .shortcut("Ctrl+K")
            .performFn([this](PerformContext* ctx) { onClear(ctx); })
            .install();

        action(wxID_CUT, "edit", "cut", seq++, _("Cu&t"), "Cut")
            .icon(wxART_CUT, dir1, "cut.svg")
            .shortcut("Ctrl+X")
            .performFn([this](PerformContext* ctx) { onCut(ctx); })
            .install();
        action(wxID_COPY, "edit", "copy", seq++, _("&Copy"), "Copy")
            .icon(wxART_COPY, dir1, "clipboard-add.svg")
            .shortcut("Ctrl+C")
            .performFn([this](PerformContext* ctx) { onCopy(ctx); })
            .install();
        action(wxID_PASTE, "edit", "paste", seq++, _("&Paste"), "Paste")
            .icon(wxART_PASTE, dir1, "empty-clipboard.svg")
            .shortcut("Ctrl+V")
            .performFn([this](PerformContext* ctx) { onPaste(ctx); })
            .install();

        seq = 2000;
        action(ID_ZOOM_IN, "view", "zoom_in", seq++, _("Zoom &In"), "Zoom in")
            .icon(wxART_PLUS, dir1, "magnifying-glass-circle.svg")
            .shortcuts({"Ctrl+=", "Ctrl++"})
            .performFn([this](PerformContext* ctx) { onZoomIn(ctx); })
            .install();
        action(ID_ZOOM_OUT, "view", "zoom_out", seq++, _("Zoom &Out"), "Zoom out")
            .icon(wxART_MINUS, dir1, "magnifying-glass.svg")
            .shortcut("Ctrl+-")
            .performFn([this](PerformContext* ctx) { onZoomOut(ctx); })
            .install();
        action(ID_ZOOM_RESET, "view", "zoom_reset", seq++, _("Zoom &Reset"),
               "Zoom reset")
            .icon(wxART_CROSS_MARK, dir1, "search-visual.svg")
            .shortcut("Ctrl+0")
            .performFn([this](PerformContext* ctx) { onZoomReset(ctx); })
            .install();
    }

    void createFragmentView(CreateViewContext* ctx) override {
        wxWindow* parent = ctx->getParent();
        m_text = new wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_WORDWRAP);

        wxSizer* rootSizer = parent->GetSizer();
        if (rootSizer == nullptr) {
            auto* box = new wxBoxSizer(wxVERTICAL);
            parent->SetSizer(box);
            rootSizer = box;
        }
        rootSizer->Add(m_text, 1, wxEXPAND);
        parent->Layout();
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
        wxString path = wxFileSelector(_("Open"), wxEmptyString, wxEmptyString, //
                                       wxEmptyString, _("All files (*.*)|*.*"), //
                                       wxFD_OPEN);
        if (path.empty())
            return;
        wxFileInputStream is(path);
        if (!is.IsOk()) {
            wxMessageBox(_("Cannot open file."), _("Error"), wxOK | wxICON_ERROR);
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
            content += chunk;
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
        wxString path = wxFileSelector(_("Save As"), wxEmptyString, wxEmptyString, //
                                       wxEmptyString, _("All files (*.*)|*.*"),    //
                                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (path.empty())
            return;
        saveTo(path);
        m_filePath = path;
    }

    void saveTo(const wxString& path) {
        wxFileOutputStream os(path);
        if (!os.IsOk()) {
            wxMessageBox(_("Cannot save file."), _("Error"), wxOK | wxICON_ERROR);
            return;
        }
        std::string utf8(m_text->GetValue().ToUTF8());
        os.Write(utf8.data(), utf8.size());
        if (!os.IsOk()) {
            wxMessageBox(_("Write failed."), _("Error"), wxOK | wxICON_ERROR);
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

    void getDefaultToolbarsSupported(std::unordered_set<std::string>& set) const override {
        uiFrame::getDefaultToolbarsSupported(set);
        // Don't add edit toolbar for legacy mode toolbar.
    }

    void getDefaultAuiToolbarsSupported(std::unordered_set<std::string>& set) const override {
        uiFrame::getDefaultAuiToolbarsSupported(set);
        set.insert("edit");
        set.insert("view");
    }

    wxAuiToolBar* makeDefaultAuiToolbar(std::string_view path) override {
        auto toolbar = uiFrame::makeDefaultAuiToolbar(path);
        if (toolbar != nullptr) {
            return toolbar;
        }
        if (path == "file") {
            if (m_fileTools == nullptr) {
                m_fileTools = m_buildViewContext.createAuiToolbar(this, wxID_ANY);
            }
            return m_fileTools;
        } else if (path == "edit") {
            if (m_editTools == nullptr) {
                m_editTools = m_buildViewContext.createAuiToolbar(this, wxID_ANY);
            }
            return m_editTools;
        } else if (path == "view") {
            if (m_viewTools == nullptr) {
                m_viewTools = m_buildViewContext.createAuiToolbar(this, wxID_ANY);
            }
            return m_viewTools;
        }
        return nullptr;
    }

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
        NodepadFrame* frame = new NodepadFrame(_("Notepad"));
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