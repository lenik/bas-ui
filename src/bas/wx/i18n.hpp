#ifndef BAS_WX_I18N_HPP
#define BAS_WX_I18N_HPP

#include <wx/string.h>

#include <string>

/** Message domain for gettext / wxTranslations (must match meson gettext name). */
#define BAS_UI_GETTEXT_DOMAIN "bas-ui"

/**
 * Marks an English msgid stored in UIElement (label/description); no runtime effect.
 * xgettext must use --keyword=basUiMsg:1. Display code translates via basUiTr().
 */
inline const char* basUiMsg(const char* utf8Msgid) { return utf8Msgid; }

/**
 * Translate a UTF-8 English msgid for the bas-ui catalog.
 * Safe for empty strings; preserves menu mnemonics (&).
 */
wxString basUiTr(const char* utf8Msgid);
wxString basUiTr(const std::string& utf8Msgid);

/**
 * Initialize wxWidgets locale and load the bas-ui message catalog.
 * Call once from application OnInit (after wxApp::OnInit), before building UI.
 *
 * Honors optional `--lang=CODE` (en, zh_CN, zh_TW, ko, ja) before falling back to
 * the system locale (e.g. LANG).
 *
 * Catalog lookup uses the install prefix share/locale, plus BAS_UI_LOCALEDIR when
 * defined (build tree for uninstalled runs).
 */
void basUiInitI18n();

#endif
