#include <wx/stackwalk.h>
#include <wx/string.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <cstdint>
#include <dlfcn.h>

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_LIGHTGRAY "\033[90m"
#define COLOR_LIGHTGREEN "\033[92m"
#define COLOR_RESET "\033[0m"

#define COLOR_LEVEL COLOR_CYAN
#define COLOR_FUNC COLOR_GREEN
#define COLOR_PARAMETERS COLOR_LIGHTGREEN
#define COLOR_FILE COLOR_YELLOW
#define COLOR_DIRNAME COLOR_LIGHTGRAY
#define COLOR_BASE COLOR_YELLOW
#define COLOR_NUMBER COLOR_CYAN
#define COLOR_ADDR COLOR_BLUE
#define COLOR_OFFSET COLOR_MAGENTA
#define COLOR_COMMENT COLOR_LIGHTGRAY

struct SourceLocation {
    std::string file;
    std::string dir;
    std::string base;
    std::string line;
    std::string function;
    std::string parameters;
    std::string discriminator;
};

std::optional<SourceLocation> addr2line(size_t offset, std::string_view lib_path) {
    if (access(lib_path.data(), R_OK) != 0) {
        std::cerr << "Cannot access library: " << lib_path.data() << std::endl;
        return std::nullopt;
    }

    // Build the command: addr2line -e <lib_path> -fCi <offset>
    char command[512];
    snprintf(command, sizeof(command), "addr2line -e %s -fCi 0x%lx", lib_path.data(),
             (unsigned long)offset);

    // Execute and read the output
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe)
        return std::nullopt;

    std::array<char, 128> blk;
    std::string content;
    // The first line is the function name, the second is file:line

    // line 1: UIGroup::setUp(BuildViewContext*, BuildViewLogs*)
    // line 2: /home/udisk/bas-ui/build/../src/ui/arch/UIGroup.cpp:387 (discriminator 2)

    while (fgets(blk.data(), blk.size(), pipe.get()) != nullptr) {
        content += blk.data();
    }

    // split lines
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    std::string line1 = lines[0];
    std::string line2 = lines[1];

    SourceLocation result;
    // parse line1
    result.function = line1.substr(0, line1.find('('));
    result.parameters = line1.substr(line1.find('(') + 1, line1.find(')'));

    // parse line2
    bool has_discriminator = line2.find('(') != std::string::npos;
    std::string file_line;
    if (has_discriminator) {
        result.discriminator =
            line2.substr(line2.find('(') + 1, line2.find(')') - line2.find('(') - 1);
        file_line = line2.substr(0, line2.find('('));
    } else {
        result.discriminator = "";
        file_line = line2;
    }

    auto colon = file_line.find(':');
    if (colon != std::string::npos) {
        result.file = file_line.substr(0, colon);
        result.line = file_line.substr(colon + 1);
    } else {
        result.file = file_line;
        result.line = "";
    }

    auto last_slash = result.file.find_last_of('/');
    if (last_slash != std::string::npos) {
        result.dir = result.file.substr(0, last_slash);
        result.base = result.file.substr(last_slash + 1);
    } else {
        result.dir = "";
        result.base = result.file;
    }

    return result;
}

class MyStackWalker : public wxStackWalker {
  public:
    virtual void OnStackFrame(const wxStackFrame& frame) override {
        // Get details like function name, file, and line number
        wxString name = frame.GetName();
        wxString file = frame.GetFileName();
        int line = frame.GetLine();

        // Print to stdout
        if (name.empty())
            return;

        std::cout << COLOR_RESET << "    "                       //
                  << COLOR_LEVEL << std::dec << frame.GetLevel() //
                  << COLOR_COMMENT << ": "                       //
                  << COLOR_FUNC << name.ToStdString();

        if (frame.HasSourceLocation()) {
            std::filesystem::path path(file.ToStdString());
            std::string dirname = path.parent_path().string();
            std::string base = path.filename().string();
            std::cout << COLOR_COMMENT << " at "         //
                      << COLOR_DIRNAME << dirname << "/" //
                      << COLOR_BASE << base              //
                      << COLOR_COMMENT << ":"            //
                      << COLOR_NUMBER << std::dec << line;
        } else {
            void* addr = frame.GetAddress();
            Dl_info info;

            // dladdr finds the base address of the shared library containing 'addr'
            if (dladdr(addr, &info) && info.dli_fname) {
                // Calculate the relative offset inside the .so file
                uintptr_t offset = (uintptr_t)addr - (uintptr_t)info.dli_fbase;

                // Now use this 'offset' with addr2line manually or in your log
                std::filesystem::path path(info.dli_fname);

                std::optional<SourceLocation> sl = addr2line(offset, info.dli_fname);
                if (sl) {
                    std::cout << COLOR_COMMENT << " at "         //
                              << COLOR_DIRNAME << sl->dir << "/" //
                              << COLOR_BASE << sl->base          //
                              << COLOR_COMMENT << ":"            //
                              << COLOR_NUMBER << sl->line;
                } else {
                    std::string dirname = path.parent_path().string();
                    std::string base = path.filename().string();
                    std::cout << COLOR_COMMENT << " [Offset: "              //
                              << COLOR_OFFSET << "0x" << std::hex << offset //
                              << COLOR_COMMENT << " in "                    //
                              << COLOR_DIRNAME << dirname << "/"            //
                              << COLOR_BASE << base                         //
                              << COLOR_COMMENT << "]";
                }
            } else {
                int offset = frame.GetOffset();
                // std::cout << " (no source location)";
                std::cout << COLOR_COMMENT << " (address: " //
                          << COLOR_ADDR << addr             //
                          << COLOR_COMMENT << ", offset: "  //
                          << COLOR_OFFSET << offset         //
                          << COLOR_COMMENT << ")";
            }
        }
        std::cout << COLOR_RESET << std::endl;
    }
};
