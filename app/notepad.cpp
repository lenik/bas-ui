/**
 * Simple notepad application using ui/arch action/group models.
 * Menubar and toolbar are built from a UIFragment and UIWidgetsContext.
 */

#include "notepad.hpp"

#include "bas/proc/MyStackWalker.hpp"
#include "bas/ui/arch/UIFragment.hpp"
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
#include "text.hpp"

#define _(s) dgettext(TEXT_DOMAIN, (s))

const std::string dir1 = "streamline-vectors/core/pop/interface-essential";
const std::string dir2 = "streamline-vectors/core/pop/artificial-intelligence";

NotepadBody::NotepadBody() {
    defineActions();
    defineStates();
}

void NotepadBody::defineActions() {
    int seq = 0;
    action(wxID_NEW, "file", "new", seq++, _("&New"), "New document")
        .icon(wxART_NEW, dir1, "new-file.svg")
        .shortcut("Ctrl+N")
        .performFn([this](PerformContext* ctx) { doNew(ctx); })
        .install();
    action(wxID_OPEN, "file", "open", seq++, _("&Open..."), "Open file")
        .icon(wxART_FILE_OPEN, dir1, "open-book.svg")
        .shortcut("Ctrl+O")
        .performFn([this](PerformContext* ctx) { doOpen(ctx); })
        .install();
    action(wxID_SAVE, "file", "save", seq++, _("&Save"), "Save file")
        .icon(wxART_FILE_SAVE, dir1, "file-add-alternate.svg")
        .shortcut("Ctrl+S")
        .performFn([this](PerformContext* ctx) { doSave(ctx); })
        .install();
    action(wxID_SAVEAS, "file", "saveas", seq++, _("Save &As..."), "Save as")
        .icon(wxART_FILE_SAVE_AS, dir1, "multiple-file-2.svg")
        .shortcut("Ctrl+Shift+S")
        .performFn([this](PerformContext* ctx) { doSaveAs(ctx); })
        .install();

    seq = 0;
    action(wxID_UNDO, "edit", "undo", seq++, _("Undo"), "Undo")
        .icon(wxART_UNDO, dir1, "line-arrow-reload-horizontal-1.svg")
        .shortcut("Ctrl+Z")
        .performFn([this](PerformContext* ctx) { doUndo(ctx); })
        .install();
    action(wxID_REDO, "edit", "redo", seq++, _("Redo"), "Redo")
        .icon(wxART_REDO, dir2, "ai-redo-spark.svg")
        .shortcut("Ctrl+Y")
        .performFn([this](PerformContext* ctx) { doRedo(ctx); })
        .install();

    seq = 1000;
    action(wxID_SELECTALL, "edit", "select_all", seq++, _("Select &All"), "Select all")
        .icon(wxART_REPORT_VIEW, dir1, "clipboard-check.svg")
        .shortcut("Ctrl+A")
        .performFn([this](PerformContext* ctx) { doSelectAll(ctx); })
        .install();
    action(wxID_CLEAR, "edit", "clear", seq++, _("Clear"), "Clear")
        .icon(wxART_DELETE, dir1, "clipboard-remove.svg")
        .shortcut("Ctrl+K")
        .performFn([this](PerformContext* ctx) { doClear(ctx); })
        .install();

    action(wxID_CUT, "edit", "cut", seq++, _("Cu&t"), "Cut")
        .icon(wxART_CUT, dir1, "cut.svg")
        .shortcut("Ctrl+X")
        .performFn([this](PerformContext* ctx) { doCut(ctx); })
        .install();
    action(wxID_COPY, "edit", "copy", seq++, _("&Copy"), "Copy")
        .icon(wxART_COPY, dir1, "clipboard-add.svg")
        .shortcut("Ctrl+C")
        .performFn([this](PerformContext* ctx) { doCopy(ctx); })
        .install();
    action(wxID_PASTE, "edit", "paste", seq++, _("&Paste"), "Paste")
        .icon(wxART_PASTE, dir1, "empty-clipboard.svg")
        .shortcut("Ctrl+V")
        .performFn([this](PerformContext* ctx) { doPaste(ctx); })
        .install();

    seq = 2000;
    action(ID_ZOOM_IN, "view", "zoom_in", seq++, _("Zoom &In"), "Zoom in")
        .icon(wxART_PLUS, dir1, "magnifying-glass-circle.svg")
        .shortcuts({"Ctrl+=", "Ctrl++"})
        .performFn([this](PerformContext* ctx) { doZoomIn(ctx); })
        .install();
    action(ID_ZOOM_OUT, "view", "zoom_out", seq++, _("Zoom &Out"), "Zoom out")
        .icon(wxART_MINUS, dir1, "magnifying-glass.svg")
        .shortcut("Ctrl+-")
        .performFn([this](PerformContext* ctx) { doZoomOut(ctx); })
        .install();
    action(ID_ZOOM_RESET, "view", "zoom_reset", seq++, _("Zoom &Reset"), "Zoom reset")
        .icon(wxART_CROSS_MARK, dir1, "search-visual.svg")
        .shortcut("Ctrl+0")
        .performFn([this](PerformContext* ctx) { doZoomReset(ctx); })
        .install();
}

void NotepadBody::defineStates() {
    int seq = 3000;
    state(ID_EDIT_EOL_MODE, "edit", "eol_mode", seq++, _("EOL mode"),
          _("Line ending mode for save"))
        .stateType(UIStateType::ENUM)
        .enumValues({EOL_AUTO, EOL_LINUX, EOL_WINDOWS})
        .cycled()
        .valueDescriptorFn([](int value) {
            const std::string tabler_icons = "tabler-icon/svg/outline";
            UIStateValueDescriptor d;
            switch (value) {
            case EOL_AUTO:
                d.label = _("Auto");
                d.description = _("Use detected line ending");
                d.icon = ImageSet("auto", tabler_icons, "a-b-2.svg");
                break;
            case EOL_LINUX:
                d.label = _("Linux");
                d.description = _("Use LF");
                d.icon = ImageSet("linux", tabler_icons, "brand-debian.svg");
                break;
            case EOL_WINDOWS:
                d.label = _("Windows");
                d.description = _("Use CRLF");
                d.icon = ImageSet("windows", tabler_icons, "brand-windows.svg");
                break;
            default:
                break;
            }
            return d;
        })
        .initValue(EOL_AUTO)
        .connect([this](UIStateVariant const value, UIStateVariant const) {
            int n = std::get<int>(value);
            m_eolMode = static_cast<EolMode>(n);
        })
        .install();

    state(ID_EDIT_INDENT_MODE, "edit", "indent_mode", seq++, _("Indent mode"),
          _("Auto indentation mode"))
        .stateType(UIStateType::ENUM)
        .enumValues({INDENT_AUTO_TAB, INDENT_AUTO_SPACE, INDENT_NONE})
        .cycled()
        .valueDescriptorFn([](int value) {
            const std::string tabler_icons = "tabler-icon/svg/outline";
            UIStateValueDescriptor d;
            switch (value) {
            case INDENT_AUTO_TAB:
                d.label = _("Tab");
                d.description = _("Keep indentation with tab");
                d.icon = ImageSet("tab", tabler_icons, "arrow-bar-to-right.svg");
                break;
            case INDENT_AUTO_SPACE:
                d.label = _("Space");
                d.description = _("Keep indentation with spaces");
                d.icon = ImageSet("space", tabler_icons, "space.svg");
                break;
            case INDENT_NONE:
                d.label = _("None");
                d.description = _("Do not auto indent");
                d.icon = ImageSet("none", tabler_icons, "space-off.svg");
                break;
            default:
                break;
            }
            return d;
        })
        .initValue(INDENT_AUTO_TAB)
        .connect([this](UIStateVariant const value, UIStateVariant const) {
            int n = std::get<int>(value);
            m_indentMode = static_cast<IndentMode>(n);
        })
        .install();

    state(ID_EDIT_TAB_SIZE, "edit", "tab_size", seq++, _("Tab Size"), _("Tab width"))
        .stateType(UIStateType::ENUM)
        .enumValues({TAB_SIZE_2, TAB_SIZE_4, TAB_SIZE_8})
        .cycled()
        .valueDescriptorFn([](int value) {
            const std::string tabler_icons = "tabler-icon/svg/outline";
            UIStateValueDescriptor d;
            switch (value) {
            case TAB_SIZE_2:
                d.label = _("Tab Size 2");
                d.description = _("Use 2 spaces for indentation");
                d.icon = ImageSet("2", tabler_icons, "number-2.svg");
                break;
            case TAB_SIZE_4:
                d.label = _("Tab Size 4");
                d.description = _("Use 4 spaces for indentation");
                d.icon = ImageSet("4", tabler_icons, "number-4.svg");
                break;
            case TAB_SIZE_8:
                d.label = _("Tab Size 8");
                d.description = _("Use 8 spaces for indentation");
                d.icon = ImageSet("8", tabler_icons, "number-8.svg");
                break;
            default:
                break;
            }
            return d;
        })
        .initValue(4)
        .connect([this](UIStateVariant const value, UIStateVariant const) {
            int n = std::get<int>(value);
            m_tabSize = static_cast<int>(n);
        })
        .install();
}

void NotepadBody::createFragmentView(CreateViewContext* ctx) {
    wxWindow* parent = ctx->getParent();
    m_text = new wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                            wxTE_MULTILINE | wxTE_WORDWRAP | wxTE_PROCESS_TAB);
    m_text->Bind(wxEVT_CHAR, &NotepadBody::onTextChar, this);

    ctx->addContent(m_text);
}

void NotepadBody::doNew(PerformContext*) {
    m_text->Clear();
    m_filePath.clear();
}

void NotepadBody::doOpen(PerformContext*) {
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
    m_detectedEolMode = detectEolMode(content.ToStdString());
}

void NotepadBody::doSave(PerformContext*) {
    if (m_filePath.empty()) {
        doSaveAs(nullptr);
        return;
    }
    saveTo(m_filePath);
}

void NotepadBody::doSaveAs(PerformContext*) {
    wxString path = wxFileSelector(_("Save As"), wxEmptyString, wxEmptyString, //
                                   wxEmptyString, _("All files (*.*)|*.*"),    //
                                   wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (path.empty())
        return;
    saveTo(path);
    m_filePath = path;
}

void NotepadBody::saveTo(const wxString& path) {
    wxFileOutputStream os(path);
    if (!os.IsOk()) {
        wxMessageBox(_("Cannot save file."), _("Error"), wxOK | wxICON_ERROR);
        return;
    }
    std::string utf8(m_text->GetValue().ToUTF8());

    EolMode eolMode = m_eolMode;
    if (eolMode == EOL_AUTO) {
        eolMode = m_detectedEolMode = detectEolMode(utf8);
    }

    utf8 = applyEolMode(utf8, eolMode);
    os.Write(utf8.data(), utf8.size());
    if (!os.IsOk()) {
        wxMessageBox(_("Write failed."), _("Error"), wxOK | wxICON_ERROR);
    }
}

void NotepadBody::onTextChar(wxKeyEvent& event) {
    const int key = event.GetKeyCode();
    if (key == WXK_TAB && m_indentMode == INDENT_AUTO_SPACE) {
        m_text->WriteText(wxString(m_tabSize, ' '));
        return;
    }
    if ((key == WXK_RETURN || key == WXK_NUMPAD_ENTER) && m_indentMode != INDENT_NONE) {
        long from = 0;
        long to = 0;
        m_text->GetSelection(&from, &to);
        long insertPos = m_text->GetInsertionPoint();
        long basePos = from == to ? insertPos : from;
        long col = 0;
        long lineNo = 0;
        bool hasPos = m_text->PositionToXY(basePos, &col, &lineNo);
        wxString line = hasPos ? m_text->GetLineText(lineNo) : wxString();
        std::string indent = makeIndentFromLine(line.ToStdString(), m_indentMode, m_tabSize);
        m_text->Replace(from, to, "\n" + indent);
        m_text->SetInsertionPoint(from + 1 + indent.size());
        return;
    }
    event.Skip();
}

// zoom by adjusting font size
void NotepadBody::doZoomIn(PerformContext*) {
    int fontSize = m_text->GetFont().GetPointSize();
    m_text->SetFont(
        wxFont(fontSize + 1, wxFONTFAMILY_DEFAULT, wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL));
}
void NotepadBody::doZoomOut(PerformContext*) {
    int fontSize = m_text->GetFont().GetPointSize();
    m_text->SetFont(
        wxFont(fontSize - 1, wxFONTFAMILY_DEFAULT, wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL));
}
void NotepadBody::doZoomReset(PerformContext*) {
    m_text->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL));
}

wxAuiToolBar* NotepadFrame::makeDefaultAuiToolbar(std::string_view path) {
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

int main(int argc, char** argv) {
    stackdump_install_crash_handler(&stackdump_color_schema_default);
    stackdump_set_interactive(1);

    Notepad app;
    return app.main(argc, argv);
}