/**
 * Simple notepad application using ui/arch action/group models.
 * Menubar and toolbar are built from a UIFragment and UIWidgetsContext.
 */
#include "ui/arch/UIFragment.hpp"
#include "ui/arch/UIGroup.hpp"
#include "ui/arch/UIWidgetsContext.hpp"

#include "proc/MyStackWalker.hpp"
#include "wx/artprov.h"
#include "wx/defs.h"

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

#include <iostream>

enum {
    ID_ZOOM_IN = wxID_HIGHEST + 1,
    ID_ZOOM_OUT,
    ID_ZOOM_RESET,
    ID_TOOLBAR_SMALL,
    ID_TOOLBAR_SHOW_LABEL,
};

class NotepadFragment : public UIFragment {
  public:
    explicit NotepadFragment(wxFrame* frame, wxTextCtrl* text) : m_frame(frame), m_text(text) {
        int seq = 0;

        // Store lambdas in local variables to ensure they live long enough
        action(wxID_NEW, "file", "new", seq++, "&New", "New document")
            .icon(wxART_NEW)
            .performFn([this](PerformContext* ctx) { onNew(ctx); })
            .install();
        action(wxID_OPEN, "file", "open", seq++, "&Open...", "Open file")
            .icon(wxART_FILE_OPEN)
            .performFn([this](PerformContext* ctx) { onOpen(ctx); })
            .install();
        action(wxID_SAVE, "file", "save", seq++, "&Save", "Save file")
            .icon(wxART_FILE_SAVE)
            .performFn([this](PerformContext* ctx) { onSave(ctx); })
            .install();
        action(wxID_SAVEAS, "file", "saveas", seq++, "Save &As...", "Save as")
            .icon(wxART_FILE_SAVE_AS)
            .performFn([this](PerformContext* ctx) { onSaveAs(ctx); })
            .install();

        seq = 1000;
        action(wxID_EXIT, "file", "exit", seq++, "E&xit", "Exit")
            .icon(wxART_QUIT)
            .performFn([this](PerformContext* ctx) { onExit(ctx); })
            .no_tool()
            .install();

        seq = 0;
        action(wxID_UNDO, "edit", "undo", seq++, "Undo", "Undo")
            .icon(wxART_UNDO)
            .performFn([this](PerformContext* ctx) { onUndo(ctx); })
            .install();
        action(wxID_REDO, "edit", "redo", seq++, "Redo", "Redo")
            .icon(wxART_REDO)
            .performFn([this](PerformContext* ctx) { onRedo(ctx); })
            .install();

        seq = 1000;
        action(wxID_SELECTALL, "edit", "select_all", seq++, "Select &All", "Select all")
            .icon(wxART_REPORT_VIEW)
            .performFn([this](PerformContext* ctx) { onSelectAll(ctx); })
            .install();
        action(wxID_CLEAR, "edit", "clear", seq++, "Clear", "Clear")
            .icon(wxART_DELETE)
            .performFn([this](PerformContext* ctx) { onClear(ctx); })
            .install();

        action(wxID_CUT, "edit", "cut", seq++, "Cu&t", "Cut")
            .icon(wxART_CUT)
            .performFn([this](PerformContext* ctx) { onCut(ctx); })
            .install();
        action(wxID_COPY, "edit", "copy", seq++, "&Copy", "Copy")
            .icon(wxART_COPY)
            .performFn([this](PerformContext* ctx) { onCopy(ctx); })
            .install();
        action(wxID_PASTE, "edit", "paste", seq++, "&Paste", "Paste")
            .icon(wxART_PASTE)
            .performFn([this](PerformContext* ctx) { onPaste(ctx); })
            .install();

        seq = 0;
        action(ID_ZOOM_IN, "view", "zoom_in", seq++, "Zoom &In", "Zoom in")
            .icon(wxART_PLUS)
            .performFn([this](PerformContext* ctx) { onZoomIn(ctx); })
            .install();
        action(ID_ZOOM_OUT, "view", "zoom_out", seq++, "Zoom &Out", "Zoom out")
            .icon(wxART_MINUS)
            .performFn([this](PerformContext* ctx) { onZoomOut(ctx); })
            .install();
        action(ID_ZOOM_RESET, "view", "zoom_reset", seq++, "Zoom &Reset", "Zoom reset")
            .icon(wxART_CROSS_MARK)
            .performFn([this](PerformContext* ctx) { onZoomReset(ctx); })
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
        wxString path = wxFileSelector("Open", wxEmptyString, wxEmptyString, //
                                       wxEmptyString, "All files (*.*)|*.*", //
                                       wxFD_OPEN);
        if (path.empty())
            return;
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
        wxString path = wxFileSelector("Save As", wxEmptyString, wxEmptyString, //
                                       wxEmptyString, "All files (*.*)|*.*",    //
                                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (path.empty())
            return;
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

    void onExit(PerformContext*) { m_frame->Close(); }

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

std::vector<UIElement*> operator+(const std::vector<UIElement*>& a,
                                  const std::vector<UIElement*>& b) {
    std::vector<UIElement*> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

class NotepadFrame : public wxFrame, public UIFragment {
  public:
    NotepadFrame() : wxFrame(nullptr, wxID_ANY, "Notepad", wxDefaultPosition, wxSize(640, 480)) {
        wxTextCtrl* text = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                          wxTE_MULTILINE | wxTE_WORDWRAP);

        auto f_this = dynamic_cast<UIFragment*>(this);
        f_editor = std::make_unique<NotepadFragment>(this, text);

        group(1, "", "file", 10, "&File").install();
        group(2, "", "edit", 20, "&Edit").install();
        group(3, "", "view", 30, "&View").install();

        int seq = 1000;

        // action(ID_TOOLBAR_SMALL, "view", "toolbar_small", seq++, "Toolbar &Small", "Toolbar
        // small")
        //     .icon(wxART_LIST)
        //     .performFn([this](PerformContext* ctx) { onToolbarSmall(ctx); })
        //     .install();

        state(ID_TOOLBAR_SHOW_LABEL, "view", "toolbar_show_label", seq++, "Toolbar &Show Label",
              "Toolbar show label")
            .icon(wxART_LIST_VIEW)
            .stateType(UIStateType::BOOL)
            .valueDescriptorFn([this](int value) {
                return UIStateValueDescriptor{
                    .label = value ? "Show Label" : "Hide Label",       //
                    .description = value ? "Show label" : "Hide label", //
                };
            })
            .initValue(false)
            .connect([this](UIStateVariant const value, UIStateVariant const old_value) {
                onToolbarShowLabel(std::get<bool>(value));
            })
        .install();

        m_menubar = new wxMenuBar();
        m_toolbar = CreateToolBar(wxTB_FLAT | wxTB_TEXT);

        UIWidgetsContext ctx;
        ctx.registerMenubar("", m_menubar);
        ctx.registerToolbar("", m_toolbar);

        std::vector<UIElement*> elements;
        elements = f_this->loadElements() + f_editor->loadElements();

        m_root = UIGroup(0, "", "", 0, "", "", "", ImageSet(), true, true);
        m_root.buildTree(elements);
        m_root.setUp(&ctx, &m_installs);

        SetMenuBar(m_menubar);
        m_toolbar->Realize();

        // Connect menu and toolbar events for each action ID
        for (auto& el : elements) {
            if (el->isAction()) {
                Bind(wxEVT_MENU, &NotepadFrame::onCommand, this, el->id);
                Bind(wxEVT_TOOL, &NotepadFrame::onCommand, this, el->id);
            }
        }

        // Bind to window show event to simulate exit after window opens
        Bind(wxEVT_SHOW, &NotepadFrame::onShow, this);

        // std::cout << "Menubar:" << std::endl;
        // dumpMenubar("  ");
        // std::cout << "Toolbar:" << std::endl;
        // dumpToolbar("  ");
    }

    void dumpMenubar(std::string prefix) {
        for (int i = 0; i < m_menubar->GetMenuCount(); i++) {
            wxMenu* menu = m_menubar->GetMenu(i);
            wxString menuLabel = m_menubar->GetMenuLabel(i);
            std::cout << prefix << "Menu: " << menuLabel << std::endl;
            for (int j = 0; j < menu->GetMenuItemCount(); j++) {
                wxMenuItem* item = menu->FindItemByPosition(j);
                if (item) {
                    std::cout << prefix << "  Item: " << item->GetLabel() << std::endl;
                    if (item->IsSubMenu()) {
                        dumpMenu(item->GetSubMenu(), prefix + "    ");
                    }
                }
            }
        }
    }
    void dumpMenu(wxMenu* menu, std::string prefix) {
        for (int j = 0; j < menu->GetMenuItemCount(); j++) {
            wxMenuItem* item = menu->FindItemByPosition(j);
            if (item) {
                std::cout << prefix << "Item: " << item->GetLabel() << std::endl;
                if (item->GetSubMenu()) {
                    dumpMenu(item->GetSubMenu(), prefix + "  ");
                }
            }
        }
    }
    void dumpToolbar(std::string prefix) {
        for (size_t i = 0; i < m_toolbar->GetToolsCount(); ++i) {
            const wxToolBarToolBase* tool = m_toolbar->GetToolByPos(i);
            if (tool) {
                std::cout << prefix << tool->GetLabel() << std::endl;
            }
        }
    }

  private:
    std::unique_ptr<NotepadFragment> f_editor;
    UIGroup m_root;
    InstallRecords m_installs;

    wxMenuBar* m_menubar;
    wxToolBar* m_toolbar;

    void onCommand(wxCommandEvent& event) {
        PerformContext ctx = f_editor->toPerformContext(event);
        if (ctx.action) {
            ctx.action->perform(&ctx);
        }
    }

    void onShow(wxShowEvent& event) {
        // Simulate exit command after window is shown (for testing)
        // std::cout << "Window shown, simulating exit..." << std::endl;
        // wxCommandEvent exitEvent(wxEVT_MENU, wxID_EXIT);
        // GetEventHandler()->AddPendingEvent(exitEvent);
    }

    // UIFragment
  public:
    wxEvtHandler* getEventHandler() override { return this; }

    void onToolbarNormal(PerformContext*) {
        // m_toolbar->SetToolBarStyle(wxTB_TEXT | wxTB_HORIZONTAL);
    }
    void onToolbarSmall(PerformContext*) {
        // m_toolbar->SetToolBarStyle(wxTB_TEXT | wxTB_HORIZONTAL | wxTB_FLAT | wxTB_SMALL_ICONS);
    }
    void onToolbarShowLabel(bool value) {
        long style = m_toolbar->GetWindowStyle();
        if (value) {
            style |= wxTB_TEXT;     // Add text
            style &= ~wxTB_NOICONS; // Ensure icons are NOT hidden
        } else {
            style &= ~wxTB_TEXT; // Remove text
        }
        m_toolbar->SetWindowStyle(style);
        m_toolbar->Realize();
        // m_toolbar->GetParent()->Layout();
    }
};

class NotepadApp : public wxApp {
  public:
    bool OnInit() override {
        NotepadFrame* frame = new NotepadFrame();
        frame->Show();
        return true;
    }

    void OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond,
                         const wxChar* msg) {
        // Print basic assert info to stdout
        printf("Assert failed: %ls:%d in %ls: %ls (%ls)\n", file, line, func, cond, msg);

        // Use wxStackWalker to print the stack trace here if supported
        MyStackWalker walker;
        walker.Walk();
    }
};

int main(int argc, char** argv) {
    stackdump_install_crash_handler(&stackdump_color_schema_default);
    stackdump_set_interactive(1);

    // Explicit wxWidgets lifecycle management (no wxEntry()).
    wxApp::SetInstance(new NotepadApp());
    if (!wxEntryStart(argc, argv)) {
        return 1;
    }

    int rc = 0;
    if (wxTheApp && wxTheApp->CallOnInit()) {
        rc = wxTheApp->OnRun();
        wxTheApp->OnExit();
    } else {
        rc = 1;
    }

    wxEntryCleanup();
    return rc;
}