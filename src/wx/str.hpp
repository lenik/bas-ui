#ifndef WX_STR_HPP
#define WX_STR_HPP

inline wxString wxUtf8(const std::string& s) { //
    return wxString::FromUTF8(s.c_str());
}

inline wxString operator"" _wx(const char* str, size_t len) { //
    return wxString::FromUTF8(str, len);
}

#endif // WX_STR_HPP