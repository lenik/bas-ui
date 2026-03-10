/**
 * Simple notepad application using ui/arch action/group models.
 * Menubar and toolbar are built from a UIFragment and UIWidgetsContext.
 */

#include "ui/arch/UIFragment.hpp"
#include "ui/arch/UIGroup.hpp"
#include "ui/arch/UIWidgetsContext.hpp"
#include "ui/arch/ImageSet.hpp"

#include <wx/app.h>
#include <wx/frame.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>
#include <wx/wfstream.h>

#include <memory>
#include <vector>

class NotepadFragment : public UIFragment {
public:
    explicit NotepadFragment(wxFrame* frame, wxTextCtrl* text)
        : m_frame(frame)
        , m_text(text)
    {
        group(1, "/", "file", 10, "&File")
            .install();
        action(wxID_NEW, "/file", "new", 0, "&New", "New document")
            .icon(wxART_NEW)
            .performFn([this](PerformContext* ctx) { onNew(ctx); })
            .install();
        action(wxID_OPEN, "/file", "open", 0, "&Open...", "Open file")
            .icon(wxART_FILE_OPEN)
            .performFn([this](PerformContext* ctx) { onOpen(ctx); })
            .install();
        action(wxID_SAVE, "/file", "save", 0, "&Save", "Save file")
            .icon(wxART_FILE_SAVE)
            .performFn([this](PerformContext* ctx) { onSave(ctx); })
            .install();
        action(wxID_SAVEAS, "/file", "saveas", 0, "Save &As...", "Save as")
            .icon(wxART_FILE_SAVE_AS)
            .performFn([this](PerformContext* ctx) { onSaveAs(ctx); })
            .install();
        action(wxID_EXIT, "/file", "exit", 0, "E&xit", "Exit")
            .icon(wxART_QUIT)
            .performFn([this](PerformContext* ctx) { onExit(ctx); })
            .install();

        group(2, "/", "edit", 20, "&Edit")
            .install();
        action(wxID_CUT, "/edit", "cut", 0, "Cu&t", "Cut")
            .icon(wxART_CUT)
            .performFn([this](PerformContext* ctx) { onCut(ctx); })
            .install();
        action(wxID_COPY, "/edit", "copy", 0, "&Copy", "Copy")
            .icon(wxART_COPY)
            .performFn([this](PerformContext* ctx) { onCopy(ctx); })
            .install();
        action(wxID_PASTE, "/edit", "paste", 0, "&Paste", "Paste")
            .icon(wxART_PASTE)
            .performFn([this](PerformContext* ctx) { onPaste(ctx); })
            .install();

    }

    wxEvtHandler* getEventHandler() override { return m_frame; }

private:
    wxFrame* m_frame;
    wxTextCtrl* m_text;
    wxString m_filePath;
    bool m_loaded{false};

    void onNew(PerformContext*) {
        m_text->Clear();
        m_filePath.clear();
    }

    void onOpen(PerformContext*) {
        wxString path = wxFileSelector("Open", wxEmptyString, wxEmptyString,
            wxEmptyString, "All files (*.*)|*.*", wxFD_OPEN);
        if (path.empty()) return;
        wxFileInputStream is(path);
        if (!is.IsOk()) {
            wxMessageBox("Cannot open file.", "Error", wxOK | wxICON_ERROR);
            return;
        }
        wxString content;
        char buf[4096];
        for (;;) {
            is.Read(buf, sizeof(buf));
            size_t n = is.LastRead();
            if (n == 0) break;
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
        wxString path = wxFileSelector("Save As", wxEmptyString, wxEmptyString,
            wxEmptyString, "All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (path.empty()) return;
        saveTo(path);
        m_filePath = path;
    }

    void saveTo(const wxString& path) {
        wxFileOutputStream os(path);
        if (!os.IsOk()) {
            wxMessageBox("Cannot save file.", "Error", wxOK | wxICON_ERROR);
            return;
        }
        std::string utf8(m_text->GetValue().ToUTF8());
        os.Write(utf8.data(), utf8.size());
        if (!os.IsOk()) {
            wxMessageBox("Write failed.", "Error", wxOK | wxICON_ERROR);
        }
    }

    void onExit(PerformContext*) {
        m_frame->Close();
    }

    void onCut(PerformContext*)  { m_text->Cut(); }
    void onCopy(PerformContext*) { m_text->Copy(); }
    void onPaste(PerformContext*) { m_text->Paste(); }
};

class NotepadFrame : public wxFrame {
public:
    NotepadFrame()
        : wxFrame(nullptr, wxID_ANY, "Notepad", wxDefaultPosition, wxSize(640, 480))
    {
        wxTextCtrl* text = new wxTextCtrl(this, wxID_ANY, "",
            wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_WORDWRAP);

        m_fragment = std::make_unique<NotepadFragment>(this, text);

        wxMenuBar* menubar = new wxMenuBar();
        wxToolBar* toolbar = CreateToolBar(wxTB_FLAT | wxTB_TEXT);

        UIWidgetsContext ctx;
        ctx.registerMenubar("", menubar);
        ctx.registerToolbar("", toolbar);

        std::vector<UIElement*> elements = m_fragment->loadElements();
        UIGroup root(0, "", "", 0, "", "", "", ImageSet(), true, true);
        root.buildTree(elements);
        root.setUp(ctx, m_installs);
        SetMenuBar(menubar);
        toolbar->Realize();

        Bind(wxEVT_MENU, &NotepadFrame::onCommand, this, wxID_ANY);
        Bind(wxEVT_TOOL, &NotepadFrame::onCommand, this, wxID_ANY);
    }

    ~NotepadFrame() override {
        UIGroup root(0, "", "", 0, "", "", "", ImageSet(), true, true);
        root.tearDown(m_installs);
    }

private:
    std::unique_ptr<NotepadFragment> m_fragment;
    InstallRecords m_installs;

    void onCommand(wxCommandEvent& event) {
        PerformContext ctx = m_fragment->toPerformContext(event);
        if (ctx.action)
            ctx.action->perform(&ctx);
    }
};

class NotepadApp : public wxApp {
public:
    bool OnInit() override {
        (new NotepadFrame())->Show();
        return true;
    }
};

wxIMPLEMENT_APP(NotepadApp);
