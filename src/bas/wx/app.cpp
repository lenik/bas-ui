#include "app.hpp"

#include "../../module.def"

#include "../proc/MyStackWalker.hpp"

#include <wx/app.h>
#include <wx/frame.h>

#include <clocale>
#include <cstring>

#include <glib.h>
#include <libintl.h>


#define _(s) dgettext(TEXT_DOMAIN, (s))

// Suppress IBus "no capability of surrounding-text" warning.
static void ibus_log_filter(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message,
    gpointer user_data) {
    (void)user_data;
    if (message && std::strstr(message, "surrounding-text") != nullptr) {
        return;
    }
    g_log_default_handler(log_domain, log_level, message, nullptr);
}

bool uiApp::OnInit() {
    if (!wxApp::OnInit()) {
        return false;
    }
    wxInitAllImageHandlers();
    return OnUserInit();
}

void uiApp::OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond,
                            const wxChar* msg) {
    // Print basic assert info to stdout
    printf(_("Assert failed: %ls:%d in %ls: %ls (%ls)\n"), file, line, func, cond, msg);

    // Use wxStackWalker to print the stack trace here if supported
    MyStackWalker walker;
    walker.Walk();
}

int uiApp::main(int argc, char** argv) {
    g_log_set_handler("IBUS", G_LOG_LEVEL_WARNING, ibus_log_filter, nullptr);

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