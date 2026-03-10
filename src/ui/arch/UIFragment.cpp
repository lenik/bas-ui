#include "UIFragment.hpp"

#include "UIAction.hpp"

#include <wx/event.h>

std::vector<std::string> UIFragment::findVars(std::string_view prefix) {
    std::vector<std::string> symbols;
    for (auto& el : m_elements) {
        if (el->name().find(prefix) == 0) {
            if (el->isAction()) {
                symbols.push_back(el->name());
                symbols.push_back(el->name() + ".checked");
                symbols.push_back(el->name() + ".visible");
                symbols.push_back(el->name() + ".enabled");
            }
            if (el->isState()) {
                symbols.push_back(el->name());
            }
        }
    }
    return symbols;
}

static bool ends_with(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool UIFragment::hasVar(std::string_view name) {
    if (ends_with(name, ".visible")) {
        std::string_view element_name = name.substr(0, name.size() - 8);
        for (auto& element : m_elements) {
            if (element->name() == element_name) {
                return true;
            }
        }
    }
    if (ends_with(name, ".enabled")) {
        std::string_view element_name = name.substr(0, name.size() - 8);
        for (auto& element : m_elements) {
            if (element->name() == element_name) {
                return true;
            }
        }
    }
    if (ends_with(name, ".checked"))
        name = name.substr(0, name.size() - 8);

    for (auto& element : m_elements) {
        if (element->name() == name) {
            return true;
        }
    }
    return false;
}

std::string UIFragment::formatVar(std::string_view name) {
    if (ends_with(name, ".visible")) {
        std::string_view element_name = name.substr(0, name.size() - 8);
        for (auto& element : m_elements) {
            if (element->name() == element_name) {
                bool val = element->visible.get();
                return std::to_string(val);
            }
        }
        return "undefined";
    }
    if (ends_with(name, ".enabled")) {
        std::string_view element_name = name.substr(0, name.size() - 8);
        for (auto& element : m_elements) {
            if (element->name() == element_name) {
                bool val = element->enabled.get();
                return std::to_string(val);
            }
        }
        return "undefined";
    }
    if (ends_with(name, ".checked"))
        name = name.substr(0, name.size() - 8);
    for (auto& element : m_elements) {
        if (element->name() == name) {
            bool val = element->checked.get();
            return std::to_string(val);
        }
    }
    return "undefined";
}

void UIFragment::parseVar(std::string_view name, std::string_view str) {
    if (ends_with(name, ".visible")) {
        std::string_view element_name = name.substr(0, name.size() - 8);
        for (auto& element : m_elements) {
            if (element->name() == element_name) {
                element->visible.set(str == "true");
            }
        }
    }
    if (ends_with(name, ".enabled")) {
        std::string_view element_name = name.substr(0, name.size() - 8);
        for (auto& element : m_elements) {
            if (element->name() == element_name) {
                element->enabled.set(str == "true");
            }
        }
    }
    if (ends_with(name, ".checked"))
        name = name.substr(0, name.size() - 8);

    for (auto& element : m_elements) {
        if (element->name() == name) {
            element->checked.set(str == "true");
        }
    }
}

std::vector<std::string> UIFragment::findMethods(std::string_view prefix) {
    std::vector<std::string> symbols;
    for (auto& el : m_elements) {
        if (el->name().find(prefix) == 0) {
            if (el->isAction()) {
                symbols.push_back(el->name());
            }
        }
    }
    return symbols;
}

bool UIFragment::hasMethod(std::string_view name) {
    for (auto& el : m_elements) {
        if (el->name() == name) {
            if (el->isAction()) {
                return true;
            }
        }
    }
    return false;
}

int UIFragment::invokeMethod(std::string_view name, const char* const* argv, int argc) {
    for (auto& el : m_elements) {
        if (el->name() == name) {
            if (el->isAction()) {
                UIAction* action = dynamic_cast<UIAction*>(el);
                if (action) {
                    wxMenuEvent event(wxEVT_MENU, action->id);
                    PerformContext ctx(action, argc, argv, &event, getEventHandler());
                    action->perform(&ctx);
                    return ctx.status;
                }
            }
        }
    }
    return -1;
}

PerformContext UIFragment::toPerformContext(wxEvent& event) {
    UIAction* action = nullptr;
    int id = event.GetId();
    for (auto& el : m_elements) {
        if (el->id == id) {
            if (el->isAction()) {
                action = dynamic_cast<UIAction*>(el);
                break;
            }
        }
    }
    return PerformContext(action, 0, nullptr,
        &event, getEventHandler());
}
