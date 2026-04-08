#include "i18n.hpp"

#include <wx/app.h>
#include <wx/intl.h>
#include <wx/stdpaths.h>

#include <memory>

wxString basUiTr(const char* utf8Msgid) {
    if (!utf8Msgid || !*utf8Msgid)
        return wxEmptyString;
    const wxString& translated =
        wxGetTranslation(wxString::FromUTF8(utf8Msgid), wxString::FromUTF8(BAS_UI_GETTEXT_DOMAIN));
    return wxString(translated);
}

wxString basUiTr(const std::string& utf8Msgid) {
    if (utf8Msgid.empty())
        return wxEmptyString;
    return basUiTr(utf8Msgid.c_str());
}

static wxLanguage basUiLangFromCode(const wxString& code) {
    wxString c = code;
    c.MakeLower();
    if (c == "en")
        return wxLANGUAGE_ENGLISH_US;
    if (c == "zh_cn" || c == "zh-cn" || c == "zh_hans" || c == "zh-hans")
        return wxLANGUAGE_CHINESE_SIMPLIFIED;
    if (c == "zh_tw" || c == "zh-tw" || c == "zh_hk" || c == "zh-hk" || c == "zh_hant" ||
        c == "zh-hant")
        return wxLANGUAGE_CHINESE_TRADITIONAL;
    if (c == "ko" || c == "ko_kr" || c == "ko-kr")
        return wxLANGUAGE_KOREAN;
    if (c == "ja" || c == "ja_jp" || c == "ja-jp")
        return wxLANGUAGE_JAPANESE;
    return wxLANGUAGE_UNKNOWN;
}

static void basUiAddCatalogLookupPrefixes() {
#ifdef BAS_UI_LOCALEDIR
    wxLocale::AddCatalogLookupPathPrefix(wxString::FromUTF8(BAS_UI_LOCALEDIR));
#endif
    wxStandardPaths& sp = wxStandardPaths::Get();
    wxString prefix = sp.GetInstallPrefix();
    if (!prefix.empty())
        wxLocale::AddCatalogLookupPathPrefix(prefix + "/share/locale");
}

void basUiInitI18n() {
    basUiAddCatalogLookupPrefixes();

    wxLanguage lang = wxLANGUAGE_DEFAULT;
    if (wxTheApp) {
        for (int i = 1; i < wxTheApp->argc; ++i) {
            wxString a = wxTheApp->argv[i];
            static const wxString prefix = "--lang=";
            if (a.length() > prefix.length() && a.StartsWith(prefix)) {
                wxString code = a.Mid(prefix.length());
                wxLanguage mapped = basUiLangFromCode(code);
                if (mapped != wxLANGUAGE_UNKNOWN)
                    lang = mapped;
                break;
            }
        }
    }

    static std::unique_ptr<wxLocale> s_locale;
    s_locale = std::make_unique<wxLocale>();

    if (!s_locale->Init(lang, wxLOCALE_LOAD_DEFAULT)) {
        s_locale = std::make_unique<wxLocale>();
        s_locale->Init(wxLANGUAGE_ENGLISH_US, wxLOCALE_LOAD_DEFAULT);
    }

    s_locale->AddCatalog(wxString::FromUTF8(BAS_UI_GETTEXT_DOMAIN));
}
