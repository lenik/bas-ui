#include "text.hpp"

EolMode detectEolMode(const std::string& text) {
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r') {
            if (i + 1 < text.size() && text[i + 1] == '\n') {
                return EOL_WINDOWS;
            }
        } else if (text[i] == '\n') {
            return EOL_LINUX;
        }
    }
    return EOL_LINUX;
}

std::string applyEolMode(const std::string& text, EolMode mode) {
    std::string normalized;
    normalized.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == '\r') {
            if (i + 1 < text.size() && text[i + 1] == '\n') {
                ++i;
            }
            normalized.push_back('\n');
        } else {
            normalized.push_back(c);
        }
    }
    if (mode != EOL_WINDOWS) {
        return normalized;
    }
    std::string crlf;
    crlf.reserve(normalized.size() + normalized.size() / 8);
    for (char c : normalized) {
        if (c == '\n') {
            crlf.push_back('\r');
        }
        crlf.push_back(c);
    }
    return crlf;
}

int leadingIndentWidth(const std::string& line, int tabSize) {
    int width = 0;
    for (char c : line) {
        if (c == ' ') {
            ++width;
        } else if (c == '\t') {
            int nextStop = ((width / tabSize) + 1) * tabSize;
            width = nextStop;
        } else {
            break;
        }
    }
    return width;
}

std::string makeIndentFromLine(const std::string& line, IndentMode mode, int tabSize) {
    if (mode == INDENT_NONE) {
        return "";
    }
    int width = leadingIndentWidth(line, tabSize);
    if (mode == INDENT_AUTO_SPACE) {
        return std::string(width, ' ');
    }
    int tabs = width / tabSize;
    int spaces = width % tabSize;
    return std::string(tabs, '\t') + std::string(spaces, ' ');
}
