#ifndef BAS_WX_COMPAT_HPP
#define BAS_WX_COMPAT_HPP

#include <wx/bitmap.h>
#include <wx/menuitem.h>
#include <wx/tbarbase.h>
#include <wx/toolbar.h>
#include <wx/version.h>

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
