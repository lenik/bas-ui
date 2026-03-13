#include "UIGroup.hpp"

#include "BuildViewLog.hpp"

#include <bas/log/uselog.h>

#include <wx/bitmap.h>
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/toolbar.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

int UIGroup::actionCount() const {
    int count = 0;
    for (const UIElement* el : children) {
        if (el->isAction())
            count++;
    }
    return count;
}

int UIGroup::flattenActionCount() const {
    int count = 0;
    for (const UIElement* el : children) {
        if (el->isAction())
            count++;
        else if (el->isGroup())
            count += dynamic_cast<const UIGroup*>(el)->flattenActionCount();
    }
    return count;
}

UIGroup* UIGroup::resolveGroup(const Path& path) { return resolveGroup(path.str()); }

int UIGroup::s_next_internal_id = 10000;

UIGroup* UIGroup::resolveGroup(std::string_view path) {
    while (!path.empty() && path.front() == '/')
        path.remove_prefix(1);
    if (path.empty())
        return this;

    int slash = path.find('/');
    std::string_view head = slash == std::string::npos ? path : path.substr(0, slash);

    UIElement* child = getChild(head);
    if (!child) {
        std::string dir = this->path ? this->path->str() : std::string();
        auto& owned = internals.emplace_back(
            std::make_unique<UIGroup>(s_next_internal_id++, dir, std::string(head), 0, "<internal>",
                                      "", "", ImageSet(), true));
        owned->internal = true;
        child = owned.get();
        child->attach(this);
    }

    if (!child->isGroup())
        return nullptr;

    UIGroup* gchild = dynamic_cast<UIGroup*>(child);
    if (slash == std::string::npos)
        return gchild;

    std::string_view tail = path.substr(slash + 1);
    return gchild->resolveGroup(tail);
}

void UIGroup::buildTree(std::vector<UIElement*>& elements) {
    for (UIElement* el : elements) {
        std::optional<Path> path = el->path;
        if (!path)
            continue;
        std::string dir = path->dir();
        UIGroup* parent = resolveGroup(dir);
        if (parent == nullptr) {
            throw std::runtime_error("dir collision: " + dir);
        }
        el->attach(parent);
        logdebug_fmt("resolved path %s dir %s attached to %s\n", path->str().c_str(), dir.c_str(),
                     parent->str().c_str());
    }
}

void UIGroup::dump(std::ostream& output) const {
    // output << "Tree: \n";
    dumpRecursive(output, "");
    output << "\n";
    // output << "---- above ----\n";
}

void UIGroup::dumpRecursive(std::ostream& output, std::string_view indent) const {
    output << str();

    // Print children recursively
    int child_count = children.size();
    if (child_count) {
        output << "/ [" << child_count << "]";
    } else {
        output << "/";
    }

    for (int i = 0; i < child_count; i++) {
        const UIElement* child = children[i];

        output << "\n";
        output << indent;
        // Print non-group children (actions, states) with same connector as groups
        if (i != child_count - 1)
            output << " |- ";
        else
            output << " `- ";

        if (child->isGroup()) {
            const UIGroup* childGroup = dynamic_cast<const UIGroup*>(child);
            assert(childGroup);

            std::string child_indent;
            if (i != child_count - 1)
                child_indent = std::string(indent) + " |  ";
            else
                child_indent = std::string(indent) + "    ";

            childGroup->dumpRecursive(output, child_indent);
        } else {
            output << child->str();
        }
    }
}
