#include "UIElement.hpp"

#include "UIGroup.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

Path UIElement::pathFromRoot() const {
    std::vector<std::string> segments;
    for (const UIElement* p = this; p; p = p->parent) {
        const UIGroup* g = dynamic_cast<const UIGroup*>(p);
        if (g && g->internal)
            continue;
        if (!p->path || p->path->str().empty() || p->path->str() == "/")
            continue;
        std::string_view base = p->path->base();
        if (!base.empty())
            segments.push_back(std::string(base));
    }
    std::reverse(segments.begin(), segments.end());
    std::string out;
    for (size_t i = 0; i < segments.size(); ++i) {
        if (i) out += '/';
        out += segments[i];
    }
    return Path(out).toAbsolute().normalize();
}

int UIElement::compare(const UIElement* other) const {
    if (!other) throw std::invalid_argument("UIElement::compare: null other");
    int cmp = priority.get() - other->priority.get();
    if (cmp != 0) return cmp;

    cmp = id - other->id;
    if (cmp != 0) return cmp;

    // std::string_view an = name();
    // std::string_view bn = other->name();
    // cmp = an.compare(bn);
    // if (cmp != 0) return cmp;

    assert(false);
    return -1;
}

std::string UIElement::str() const {
    // --[x] .id. path[priority] - label(description)
    std::string line;
    if (!this->enabled.get())
        line.append("# ");
    if (!line.empty())
        line.append(" ");
    if (!visible.get())
        line.append(".");
    line.append(std::to_string(id));
    line.append(". ");
    
    if (path)
        line.append(path->str());
    if (priority.get() != 0)
        line.append("[" + std::to_string(priority.get()) + "]");

    std::string label = this->label.get();
    if (!label.empty()) {
        line.append(" - ");
        line.append(label);
    }
    std::string description = this->description.get();
    if (!description.empty()) {
        line.append(" -- ");
        line.append(description);
    }
    return line;
}
    
void UIElement::attach(UIElement* parent) {
    if (this->parent != parent) {
        detach();
        this->parent = parent;
        if (parent != nullptr) {
            // find pos to insert by priority
            auto it = std::lower_bound(parent->children.begin(), 
                    parent->children.end(), 
                    this, 
                    std::mem_fn(&UIElement::lessThan));
            parent->children.insert(it, this);
        }
    }
}

void UIElement::detach() {
    if (this->parent) {
        this->parent->children.erase(
            std::remove(this->parent->children.begin(), this->parent->children.end(), this), this->parent->children.end());
        this->parent = nullptr;
    }
}
