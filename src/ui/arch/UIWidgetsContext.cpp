#include "UIWidgetsContext.hpp"

#include <stdexcept>

std::vector<wxMenuBar*> UIWidgetsContext::getMenubars(std::string_view path) {
    std::vector<wxMenuBar*> matches;

    auto it = m_menubars.find(std::string(path));
    if (it != m_menubars.end())
        matches = it->second;

    return matches;
}

std::vector<wxMenu*> UIWidgetsContext::getMenus(std::string_view path) {
    std::vector<wxMenu*> matches;

    auto it = m_menus.find(std::string(path));
    if (it != m_menus.end())
        matches = it->second;

    return matches;
}

std::vector<wxToolBar*> UIWidgetsContext::getToolbars(std::string_view path) {
    std::vector<wxToolBar*> matches;

    auto it = m_toolbars.find(std::string(path));
    if (it != m_toolbars.end())
        matches = it->second;

    return matches;
}

void UIWidgetsContext::registerMenubar(std::string_view path, wxMenuBar* menubar) {
    if (!menubar) throw std::invalid_argument("UIWidgetsContext::registerMenubar: null menubar");
    m_menubars[std::string(path)].push_back(menubar);
}

void UIWidgetsContext::registerMenu(std::string_view path, wxMenu* menu) {
    if (!menu) throw std::invalid_argument("UIWidgetsContext::registerMenu: null menu");
    m_menus[std::string(path)].push_back(menu);
}

void UIWidgetsContext::registerToolbar(std::string_view path, wxToolBar* toolbar) {
    if (!toolbar) throw std::invalid_argument("UIWidgetsContext::registerToolbar: null toolbar");
    m_toolbars[std::string(path)].push_back(toolbar);
}