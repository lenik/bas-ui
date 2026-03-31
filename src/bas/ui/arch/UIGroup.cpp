#include "UIGroup.hpp"

#include "BuildViewLog.hpp"

#include <wx/bitmap.h>
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/toolbar.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <bas/log/uselog.h>

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
    int i = 0;
    for (const UIElement* child : children) {
        if (child == nullptr)
            logerror_fmt("nullptr child in group %s[%d]", str().c_str(), i);
        else if (child == this)
            logerror_fmt("self reference in group %s[%d]", str().c_str(), i);
        else if (child->isAction())
            count++;
        else if (child->isGroup())
            count += dynamic_cast<const UIGroup*>(child)->flattenActionCount();
        i++;
    }
    return count;
}

UIGroup* UIGroup::resolveGroup(const Path& path, BuildViewContext* context) {
    return resolveGroup(path.str(), context);
}

UIGroup* UIGroup::resolveGroup(std::string_view path, BuildViewContext* context) {
    while (!path.empty() && path.front() == '/')
        path.remove_prefix(1);
    if (path.empty())
        return this;

    int slash = path.find('/');
    std::string_view head = slash == std::string::npos ? path : path.substr(0, slash);

    UIElement* child = getChild(head);
    if (!child) {
        std::string dir = this->path ? this->path->str() : std::string();
        int internal_id = context->getNextId();
        std::string name = std::string(head);
        std::string label = name;
        if (!label.empty())
            label[0] = std::toupper(label[0]);
        std::string description = "<internal: " + std::string(path) + ">";
        UIGroup* node = new UIGroup(internal_id, dir, name, 0, //
                                    label, description, "",    //
                                    ImageSet(), true);
        internals.emplace_back(std::unique_ptr<UIGroup>(node));
        node->internal = true;
        child = node;
        child->attach(this);
    }

    if (!child->isGroup())
        return nullptr;

    UIGroup* gchild = dynamic_cast<UIGroup*>(child);
    if (slash == std::string::npos)
        return gchild;

    std::string_view tail = path.substr(slash + 1);
    return gchild->resolveGroup(tail, context);
}

void UIGroup::addToTree(std::vector<UIElement*>& elements, BuildViewContext* context) {
    for (UIElement* el : elements) {
        std::optional<Path> path = el->path;
        if (!path)
            continue;
        std::string dir = path->dir();
        UIGroup* parent = resolveGroup(dir, context);
        if (parent == nullptr) {
            throw std::runtime_error("dir collision: " + dir);
        }
        el->attach(parent);
        logdebug_fmt("resolved path %s dir %s attached to %s\n", path->str().c_str(), dir.c_str(),
                     parent->str().c_str());
    }
}

void UIGroup::removeFromTree(std::vector<UIElement*>& elements) {
    for (UIElement* el : elements) {
        std::optional<Path> path = el->path;
        if (!path)
            continue;
        el->detach();

        // remove internal parent groups
        std::string dir = path->dir();
        UIGroup* parent = resolveGroup(dir, nullptr);
        assert(parent);
        while (parent && parent != this) {
            if (!parent->internal)
                break;
            if (parent->getChildCount() == 0) {
                parent->detach();
                delete parent;
            }
            parent = parent->getParent();
        }
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
