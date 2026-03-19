#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include "ImageSet.hpp"

#include "wx/artprov.h"

#include <bas/script/property_support.hpp>
#include <bas/util/Path.hpp>

#include <optional>
#include <string>
#include <vector>

class UIGroup;

/**
* Base for all UI tree nodes. Elements are organized in a tree by path (Path, always absolute).
* The main UI builds the tree from a flat list via UIGroup::buildTree().
*/
class UIElement {
public:
    UIElement() = default;
    UIElement(int id_
            , const std::string& dir_
            , const std::string& tail_
            , int priority_ = 0
            , const std::string& label_ = ""
            , const std::string& description_ = ""
            , const std::string& doc_ = ""
            , const ImageSet& icon_ = ImageSet()
            , bool visible_ = true
            , bool enabled_ = true
            , bool checked_ = false
            )
            : path(Path(dir_, tail_))
            , id(id_)
            , priority(priority_)
            , label(label_)
            , description(description_)
            , doc(doc_)
            , icon(icon_)
            , visible(visible_)
            , enabled(enabled_)
            {}

    virtual ~UIElement() {
        detach();
    }

    UIElement(const UIElement&) = delete;
    UIElement& operator=(const UIElement&) = delete;

    UIElement(UIElement&&) = default;
    UIElement& operator=(UIElement&&) = default;

    virtual bool isGroup() const { return false; }
    virtual bool isAction() const { return false; }
    virtual bool isState() const { return false; }

    std::string dir() const { return path ? path->dir() : std::string(); }
    std::string name() const { return path ? path->name() : std::string(); }

    /** Logical path from root to this element; internal group names are skipped. */
    Path pathFromRoot() const;

public:
    int id{0};
    std::optional<Path> path;

    // Lower = higher priority (shown first)
    observable<int> priority;
    // Menu/toolbar label
    observable<std::string> label;
    // Tooltip
    observable<std::string> description;
    // Detailed help (e.g. HTML)
    observable<std::string> doc;
    observable<ImageSet> icon;

    observable<bool> visible;
    observable<bool> enabled;


    bool isEnabled() const { return enabled.get(); }
    bool isVisible() const { return visible.get(); }
    
    bool no_menu{false};
    bool no_tool{false};
    bool menuWanted() { return !no_menu; }
    bool toolWanted() { return !no_tool; }

    int compare(const UIElement* other) const;
    bool operator<(const UIElement& other) const { return compare(&other) < 0; }
    bool operator>(const UIElement& other) const { return compare(&other) > 0; }

    bool lessThan(const UIElement* other) const { return compare(other) < 0; }

    std::string str() const;

public:
    void attach(UIGroup* parent);
    void detach();

    UIGroup* getParent() const { return parent; }
    UIElement* getChild(std::string_view name) const {
        for (UIElement* child : children) {
            if (child->name() == name)
                return child;
        }
        return nullptr;
    }
    size_t getChildCount() const { return children.size(); }
    UIElement* getChild(size_t index) const { return children[index]; }
    
protected:
    // runtime wired
    UIGroup* parent{nullptr};
    std::vector<UIElement*> children;

public:

    template<class builder_t, class T>
    class _Builder {
        public:
            _Builder(int id = 0
                , const std::string& dir = ""
                , const std::string& name = ""
                , int priority = 0
                , const std::string& label = ""
                , const std::string& description = ""
                , const std::string& doc = ""
                , const ImageSet& icon = ImageSet()
                , bool visible = true
                , bool enabled = true
                )
                : m_id(id)
                , m_dir(dir)
                , m_name(name)
                , m_priority(priority)
                , m_label(label)
                , m_description(description)
                , m_doc(doc)
                , m_icon(icon)
                , m_visible(visible)
                , m_enabled(enabled)
                {}

            builder_t& self() { return static_cast<builder_t&>(*this); }
            builder_t& path(std::string s)
                { Path p(std::move(s)); m_dir = p.dir(); m_name = p.name(); return self(); }
            builder_t& dir(std::string s)
                { m_dir = s; return self(); }
            builder_t& name(std::string s)
                { m_name = s; return self(); }
            builder_t& id(int v)
                { m_id = v; return self(); }
            builder_t& priority(int v)
                { m_priority = v; return self(); }
            builder_t& label(std::string s)
                { m_label = s; return self(); }
            builder_t& description(std::string s)
                { m_description = s; return self(); }
            builder_t& doc(std::string s)
                { m_doc = s; return self(); }
            builder_t& visible(bool v)
                { m_visible = v; return self(); }
            builder_t& enabled(bool v)
                { m_enabled = v; return self(); }
            builder_t& icon(wxArtID id)
                { m_icon = ImageSet(id); return self(); }
            builder_t& icon(const ImageSet& v)
                { m_icon = v; return self(); }
            builder_t& icon(wxArtID id, Path path)
                { m_icon = ImageSet(id, path); return self(); }
            builder_t& icon(wxArtID id, std::string path)
                { m_icon = ImageSet(id, Path(path)); return self(); }
            builder_t& icon(wxArtID id, std::string dir, std::string tail)
                { m_icon = ImageSet(id, Path(dir, tail)); return self(); }
        
            builder_t& no_menu()
                { m_no_menu = true; return self(); }
            builder_t& no_tool()
                { m_no_tool = true; return self(); }

            void applyTo(UIElement* el) const {
                if (m_id == 0) throw std::invalid_argument("id is required");
                if (m_name.empty()) throw std::invalid_argument("name is required");

                el->id = m_id;
                
                std::string dir = m_dir;
                while (!dir.empty() && dir.back() == '/')
                    dir.pop_back();
                el->path = Path(dir, m_name);
                
                el->priority.set(m_priority);
                el->label.set(m_label);
                el->description.set(m_description);
                el->doc.set(m_doc);
                el->visible.set(m_visible);
                el->enabled.set(m_enabled);
                el->icon.set(m_icon);
                el->no_menu = m_no_menu;
                el->no_tool = m_no_tool;
            }
        
        protected:
            std::string m_dir;
            std::string m_name;
            int m_id{0};
            int m_priority{0};
            std::string m_label;
            std::string m_description;
            std::string m_doc;
            bool m_visible{true};
            bool m_enabled{true};
            ImageSet m_icon;
            bool m_no_menu{false};
            bool m_no_tool{false};
    };
    
};

#endif // UI_ELEMENT_H