#include "AutomationMap.hpp"

#include "AutomationError.hpp"

#include "../arch/UIElement.hpp"

#include <cctype>
#include <cstdlib>

namespace bas::ui::automation {

void AutomationMap::bind(const std::string& name, int id) {
    if (name.empty()) {
        throw AutomationError("bind", name, "name is required");
    }
    m_entries[name] = id;
}

void AutomationMap::bind(const std::string& name, wxWindow* window) {
    if (name.empty()) {
        throw AutomationError("bind", name, "name is required");
    }
    if (!window) {
        throw AutomationError("bind", name, "window is null");
    }
    m_entries[name] = window;
}

void AutomationMap::bind(const std::string& name, UIElement* element) {
    if (name.empty()) {
        throw AutomationError("bind", name, "name is required");
    }
    if (!element) {
        throw AutomationError("bind", name, "element is null");
    }
    m_entries[name] = element;
}

void AutomationMap::bindElement(UIElement* element) {
    if (!element) {
        return;
    }
    const std::string leaf = element->name();
    if (!leaf.empty() && !contains(leaf)) {
        bind(leaf, element);
    }
    if (element->path) {
        const std::string full = element->pathFromRoot().str();
        if (!full.empty() && full != "/" && !contains(full)) {
            bind(full, element);
        }
        // Also bind without leading slash for convenience.
        if (full.size() > 1 && full[0] == '/') {
            const std::string rel = full.substr(1);
            if (!contains(rel)) {
                bind(rel, element);
            }
        }
    }
    // Always bind by numeric id string for arch command routing.
    if (element->id != 0) {
        const std::string idKey = "#" + std::to_string(element->id);
        if (!contains(idKey)) {
            bind(idKey, element);
        }
    }
}

void AutomationMap::bindTree(UIElement* root) {
    if (!root) {
        return;
    }
    bindElement(root);
    for (size_t i = 0; i < root->getChildCount(); ++i) {
        bindTree(root->getChild(i));
    }
}

void AutomationMap::unbind(const std::string& name) { m_entries.erase(name); }

void AutomationMap::clear() { m_entries.clear(); }

bool AutomationMap::contains(const std::string& name) const {
    return m_entries.find(name) != m_entries.end();
}

const AutomationMap::Entry* AutomationMap::find(const std::string& name) const {
    const auto it = m_entries.find(name);
    if (it == m_entries.end()) {
        return nullptr;
    }
    return &it->second;
}

wxWindow* AutomationMap::findById(wxWindow* root, int id) {
    if (!root || id == wxID_ANY || id == 0) {
        return nullptr;
    }
    if (root->GetId() == id) {
        return root;
    }
    return root->FindWindow(id);
}

bool AutomationMap::tryParseId(const std::string& text, int& outId) {
    if (text.empty()) {
        return false;
    }
    std::string s = text;
    if (s[0] == '#') {
        s = s.substr(1);
    }
    if (s.empty()) {
        return false;
    }
    for (unsigned char c : s) {
        if (!std::isdigit(c) && c != '-' && c != '+') {
            return false;
        }
    }
    char* end = nullptr;
    const long value = std::strtol(s.c_str(), &end, 10);
    if (!end || *end != '\0') {
        return false;
    }
    outId = static_cast<int>(value);
    return true;
}

wxWindow* AutomationMap::findByNameDeep(wxWindow* root, const std::string& name) {
    if (!root || name.empty()) {
        return nullptr;
    }
    const wxString want = wxString::FromUTF8(name.c_str());
    if (root->GetName() == want) {
        return root;
    }
    for (wxWindowList::compatibility_iterator node = root->GetChildren().GetFirst(); node;
         node = node->GetNext()) {
        if (wxWindow* found = findByNameDeep(node->GetData(), name)) {
            return found;
        }
    }
    return nullptr;
}

wxWindow* AutomationMap::resolve(wxWindow* root, const std::string& nameOrId) const {
    if (!root || nameOrId.empty()) {
        return nullptr;
    }

    if (const Entry* entry = find(nameOrId)) {
        if (std::holds_alternative<int>(*entry)) {
            return findById(root, std::get<int>(*entry));
        }
        if (std::holds_alternative<wxWindow*>(*entry)) {
            wxWindow* window = std::get<wxWindow*>(*entry);
            if (!window) {
                return nullptr;
            }
            if (window == root) {
                return window;
            }
            for (wxWindow* p = window->GetParent(); p; p = p->GetParent()) {
                if (p == root) {
                    return window;
                }
            }
            return window;
        }
        // UIElement*: try FindWindow by element id.
        if (UIElement* el = std::get<UIElement*>(*entry)) {
            return findById(root, el->id);
        }
    }

    int id = 0;
    if (tryParseId(nameOrId, id)) {
        if (wxWindow* byId = findById(root, id)) {
            return byId;
        }
    }

    return findByNameDeep(root, nameOrId);
}

UIElement* AutomationMap::resolveElement(const std::string& nameOrPath) const {
    if (nameOrPath.empty()) {
        return nullptr;
    }
    if (const Entry* entry = find(nameOrPath)) {
        if (std::holds_alternative<UIElement*>(*entry)) {
            return std::get<UIElement*>(*entry);
        }
    }
    // Try with/without leading slash.
    if (nameOrPath[0] != '/') {
        if (const Entry* entry = find("/" + nameOrPath)) {
            if (std::holds_alternative<UIElement*>(*entry)) {
                return std::get<UIElement*>(*entry);
            }
        }
    } else if (nameOrPath.size() > 1) {
        if (const Entry* entry = find(nameOrPath.substr(1))) {
            if (std::holds_alternative<UIElement*>(*entry)) {
                return std::get<UIElement*>(*entry);
            }
        }
    }
    return nullptr;
}

} // namespace bas::ui::automation
