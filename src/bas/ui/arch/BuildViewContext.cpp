#include "BuildViewContext.hpp"

#include <wx/aui/auibar.h>

#include <stdexcept>

BuildViewContext::BuildViewContext()
    : IdManager<int>(10000)
    , m_auiPreferred(true)
{
}

std::vector<wxMenuBar*> BuildViewContext::getMenubars(std::string_view path) {
    std::vector<wxMenuBar*> matches;

    auto it = m_menubars.find(std::string(path));
    if (it != m_menubars.end())
        matches = it->second;

    return matches;
}

std::vector<wxMenu*> BuildViewContext::getMenus(std::string_view path) {
    std::vector<wxMenu*> matches;

    auto it = m_menus.find(std::string(path));
    if (it != m_menus.end())
        matches = it->second;

    return matches;
}

std::vector<wxToolBar*> BuildViewContext::getToolbars(std::string_view _path) {
    std::vector<wxToolBar*> matches;
    std::string path = std::string(_path);
    while (true) {
        auto it = m_toolbars.find(path);
        if (it != m_toolbars.end()) {
            matches.insert(matches.end(), it->second.begin(), it->second.end());
        }
        if (path.empty())
            break;
        size_t last_slash = path.find_last_of('/');
        if (last_slash == std::string::npos)
            path = "";
        else
            path = path.substr(0, last_slash);
    }
    return matches;
}

std::vector<wxAuiToolBar*> BuildViewContext::getAuiToolbars(std::string_view _path) {
    std::vector<wxAuiToolBar*> matches;

    std::string path = std::string(_path);

    while (true) {
        auto it = m_auiToolbars.find(path);
        if (it != m_auiToolbars.end()) {
            matches.insert(matches.end(), it->second.begin(), it->second.end());
            break; // no dup.
        }
        if (path.empty())
            break;

        size_t last_slash = path.find_last_of('/');
        if (last_slash == std::string::npos)
            path = "";
        else
            path = path.substr(0, last_slash);
    }
    return matches;
}

void BuildViewContext::registerMenubar(std::string_view path, wxMenuBar* menubar) {
    if (!menubar)
        throw std::invalid_argument("BuildViewContext::registerMenubar: null menubar");
    m_menubars[std::string(path)].push_back(menubar);
}

void BuildViewContext::registerMenu(std::string_view path, wxMenu* menu) {
    if (!menu)
        throw std::invalid_argument("BuildViewContext::registerMenu: null menu");
    m_menus[std::string(path)].push_back(menu);
}

void BuildViewContext::registerToolbar(std::string_view path, wxToolBar* toolbar) {
    if (!toolbar)
        throw std::invalid_argument("BuildViewContext::registerToolbar: null toolbar");
    m_toolbars[std::string(path)].push_back(toolbar);
}

void BuildViewContext::registerAuiToolbar(std::string_view path, wxAuiToolBar* toolbar) {
    if (!toolbar)
        throw std::invalid_argument("BuildViewContext::registerAuiToolbar: null toolbar");
    m_auiToolbars[std::string(path)].push_back(toolbar);
}

void BuildViewContext::forMenubars(std::function<void(wxMenuBar*)> fn,
                                   std::optional<std::string_view> path) {
    if (path) {
        std::string k(*path);
        if (auto it = m_menubars.find(k); it != m_menubars.end()) {
            for (auto& menubar : it->second) {
                fn(menubar);
            }
        }
    } else {
        for (auto& [name, list] : m_menubars) {
            for (auto& menubar : list) {
                fn(menubar);
            }
        }
    }
}

void BuildViewContext::forMenus(std::function<void(wxMenu*)> fn,
                                std::optional<std::string_view> path) {
    if (path) {
        std::string k(*path);
        if (auto it = m_menus.find(k); it != m_menus.end()) {
            for (auto& menu : it->second) {
                fn(menu);
            }
        }
    } else {
        for (auto& [name, list] : m_menus) {
            for (auto& menu : list) {
                fn(menu);
            }
        }
    }
}

void BuildViewContext::forToolbars(std::function<void(wxToolBar*)> fn,
                                   std::optional<std::string_view> path) {
    if (path) {
        std::string k(*path);
        if (auto it = m_toolbars.find(k); it != m_toolbars.end()) {
            for (auto& toolbar : it->second) {
                fn(toolbar);
            }
        }
    } else {
        for (auto& [name, list] : m_toolbars) {
            for (auto& toolbar : list) {
                fn(toolbar);
            }
        }
    }
}

void BuildViewContext::forAuiToolbars(std::function<void(wxAuiToolBar*)> fn,
                                      std::optional<std::string_view> path) {
    if (path) {
        std::string k(*path);
        if (auto it = m_auiToolbars.find(k); it != m_auiToolbars.end()) {
            for (auto& toolbar : it->second) {
                fn(toolbar);
            }
        }
    } else {
        for (auto& [name, list] : m_auiToolbars) {
            for (auto& toolbar : list) {
                fn(toolbar);
            }
        }
    }
}

wxAuiToolBar* BuildViewContext::createAuiToolbar(wxWindow* parent, wxWindowID id,
                                                 const wxPoint& pos, const wxSize& size,
                                                 long style) {
    wxAuiToolBar* toolbar = new wxAuiToolBar(parent, id, pos, size, style);
    toolbar->SetArtProvider(new LegacyToolBarArt());
    return toolbar;
}