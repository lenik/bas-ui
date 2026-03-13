#include "app.hpp"

#include "proc/MyStackWalker.hpp"
#include "ui/arch/UIFragment.hpp"

#include <wx/frame.h>
#include <wx/app.h>

#include <bas/proc/stackdump.h>

#include <cstdarg>
#include <vector>

bool uiApp::OnInit() {
    if (m_frame) {
        m_frame->Show();
    }
    return true;
}

void uiApp::OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond,
                            const wxChar* msg) {
    // Print basic assert info to stdout
    printf("Assert failed: %ls:%d in %ls: %ls (%ls)\n", file, line, func, cond, msg);

    // Use wxStackWalker to print the stack trace here if supported
    MyStackWalker walker;
    walker.Walk();
}

int uiApp::main(int argc, char** argv, UIFragment* fragment, ...) {
    stackdump_install_crash_handler(&stackdump_color_schema_default);
    stackdump_set_interactive(1);

    std::vector<wxFrame*> frames;
    std::vector<UIFragment*> fragments;

    va_list args;
    va_start(args, fragment);
    while (fragment) {
        fragments.push_back(fragment);
        wxFrame* frame = dynamic_cast<wxFrame*>(fragment);
        if (frame) {
            frames.push_back(frame);
        }
        fragment = va_arg(args, UIFragment*);
    }
    va_end(args);

    wxFrame* frame = frames.empty() ? nullptr : frames[0];
    uiApp app(frame, fragments);
    wxApp::SetInstance(&app);
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