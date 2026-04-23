#ifndef TEXT_HPP
#define TEXT_HPP

#include <string>

enum EolMode {
    EOL_AUTO = 0,
    EOL_LINUX = 1,
    EOL_WINDOWS = 2,
};

enum IndentMode {
    INDENT_AUTO_TAB = 0,
    INDENT_AUTO_SPACE = 1,
    INDENT_NONE = 2,
};

enum TabSizeEnum {
    TAB_SIZE_2 = 2,
    TAB_SIZE_4 = 4,
    TAB_SIZE_8 = 8,
};

EolMode detectEolMode(const std::string& text);
std::string applyEolMode(const std::string& text, EolMode mode);
int leadingIndentWidth(const std::string& line, int tabSize);
std::string makeIndentFromLine(const std::string& line, IndentMode mode, int tabSize);

#endif