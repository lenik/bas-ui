#ifndef BAS_WX_COMPAT_HPP
#define BAS_WX_COMPAT_HPP

#include <wx/bitmap.h>
#include <wx/menuitem.h>
#include <wx/string.h>
#include <wx/tbarbase.h>
#include <wx/toolbar.h>
#include <wx/version.h>

#include <cctype>
#include <string>

#if defined(BAS_WX_API_3_0)
#define BAS_WX_MODERN 0
#elif defined(BAS_WX_API_3_1)
#define BAS_WX_MODERN 1
#elif wxCHECK_VERSION(3, 1, 0)
#define BAS_WX_MODERN 1
#else
#define BAS_WX_MODERN 0
#endif

#if BAS_WX_MODERN
#include <wx/aui/auibar.h>
#include <wx/bmpbndl.h>
#else
#include <wx/aui/auibar.h>
#endif

inline wxString basWxMenuItemLabel(const wxMenuItem* item) {
#if BAS_WX_MODERN
    return item->GetItemLabel();
#else
    return item->GetLabel();
#endif
}

inline void basWxSetMenuItemBitmap(wxMenuItem* item, const wxBitmap& bitmap) {
#if BAS_WX_MODERN
    item->SetBitmap(wxBitmapBundle(bitmap));
#else
    item->SetBitmap(bitmap);
#endif
}

/**
 * GTK rejects some bare keys as menu accelerators (Tab, arrows, Space, …).
 * Only use the wx "\tAccel" form when the shortcut is accelerator-safe; otherwise
 * append a display-only hint so CharHook/game handlers can still document the key.
 */
inline bool basWxIsMenuAcceleratorSafe(const std::string& shortcut) {
    if (shortcut.empty()) {
        return false;
    }
    // Any chord with a modifier is fine.
    if (shortcut.find("Ctrl") != std::string::npos || shortcut.find("Alt") != std::string::npos ||
        shortcut.find("Shift") != std::string::npos ||
        shortcut.find("RawCtrl") != std::string::npos) {
        return true;
    }

    auto eqIgnoreCase = [](const std::string& a, const char* b) {
        for (size_t i = 0;; ++i) {
            const unsigned char ca = static_cast<unsigned char>(a[i]);
            const unsigned char cb = static_cast<unsigned char>(b[i]);
            if (std::tolower(ca) != std::tolower(cb)) {
                return false;
            }
            if (ca == '\0') {
                return true;
            }
        }
    };

    // Keys GTK will not accept alone as accelerators (wx debug warnings).
    static const char* kUnsafe[] = {
        "Tab",    "Up",     "Down",      "Left",      "Right",  "Space",
        "Return", "Enter",  "Back",      "Backspace", "Escape", "Esc",
        "Delete", "Insert", "Home",      "End",       "PageUp", "PageDown",
        "PgUp",   "PgDn",
    };
    for (const char* key : kUnsafe) {
        if (eqIgnoreCase(shortcut, key)) {
            return false;
        }
    }
    return true;
}

inline void basWxAppendMenuShortcut(wxString& label, const std::string& shortcut) {
    if (shortcut.empty()) {
        return;
    }
    const wxString text = wxString::FromUTF8(shortcut.c_str());
    if (basWxIsMenuAcceleratorSafe(shortcut)) {
        label += "\t";
        label += text;
    } else {
        label += "\t[";
        label += text;
        label += "]";
    }
}

inline void basWxToolBarAddTool(wxToolBar* toolbar, int id, const wxString& label,
                                const wxBitmap& bitmap, const wxString& shortHelp,
                                wxItemKind kind = wxITEM_NORMAL) {
#if BAS_WX_MODERN
    toolbar->AddTool(id, label, wxBitmapBundle(bitmap), shortHelp, kind);
#else
    toolbar->AddTool(id, label, bitmap, shortHelp, kind);
#endif
}

inline void basWxAuiToolBarAddTool(wxAuiToolBar* toolbar, int id, const wxString& label,
                                   const wxBitmap& bitmap, const wxString& shortHelp,
                                   wxItemKind kind = wxITEM_NORMAL) {
#if BAS_WX_MODERN
    toolbar->AddTool(id, label, wxBitmapBundle(bitmap), shortHelp, kind);
#else
    toolbar->AddTool(id, label, bitmap, shortHelp, kind);
#endif
}

inline void basWxToolBarSetNormalBitmap(wxToolBar* toolbar, int id, const wxBitmap& bitmap) {
#if BAS_WX_MODERN
    toolbar->SetToolNormalBitmap(id, wxBitmapBundle(bitmap));
#else
    toolbar->SetToolNormalBitmap(id, bitmap);
#endif
}

inline void basWxAuiToolBarSetBitmap(wxAuiToolBar* toolbar, int id, const wxBitmap& bitmap) {
#if BAS_WX_MODERN
    toolbar->SetToolBitmap(id, wxBitmapBundle(bitmap));
#else
    toolbar->SetToolBitmap(id, bitmap);
#endif
}

#endif // BAS_WX_COMPAT_HPP
