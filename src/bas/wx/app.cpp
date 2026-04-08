#include "app.hpp"

#include "i18n.hpp"
#include "../proc/MyStackWalker.hpp"

#include <wx/app.h>
#include <wx/frame.h>

bool uiApp::OnInit() {
    if (!wxApp::OnInit()) {
        return false;
    }
    basUiInitI18n();
    wxInitAllImageHandlers();
    return OnUserInit();
}

void uiApp::OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond,
                            const wxChar* msg) {
    // Print basic assert info to stdout
    printf("Assert failed: %ls:%d in %ls: %ls (%ls)\n", file, line, func, cond, msg);

    // Use wxStackWalker to print the stack trace here if supported
    MyStackWalker walker;
    walker.Walk();
}

int uiApp::main(int argc, char** argv) {
    wxApp::SetInstance(this);
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

    // Prevent wxEntryCleanup from deleting this stack-allocated instance.
    // We already ran OnExit() above, so just clear the global pointer.
    wxApp::SetInstance(nullptr);

    wxEntryCleanup();
    return rc;
}