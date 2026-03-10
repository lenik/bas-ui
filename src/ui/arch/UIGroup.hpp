#ifndef UI_GROUP_H
#define UI_GROUP_H

#include "UIElement.hpp"

#include <wx/menu.h>
#include <wx/toolbar.h>

#include <iostream>
#include <memory>
#include <vector>

class UIWidgetsContext;
class UIGroup;

struct InstallRecord {
    virtual ~InstallRecord();

    enum Kind { MENU, SUBMENU, MENU_ITEM, TOOLBAR_TOOL };
    Kind kind{MENU_ITEM};
    wxMenuBar* menuBar{nullptr};
    int menuPos{-1};

    wxMenu* menu{nullptr};
    int subMenuId{-1};
    wxMenuItem* menuItem{nullptr};

    wxToolBar* toolbar{nullptr};
    int toolId{-1};
    
    UIGroup* group{nullptr};
};

struct InstallRecords
    : public std::vector<std::unique_ptr<InstallRecord>>
{
    /** Dump install records to output stream */
    void dump(std::ostream& output = std::cout) const;
};

/**
* Group node. buildTree(elements) builds parent/children from a flat list
* of elements using their path; this group is the root of the resulting tree.
* When internal==true, this group is used only for grouping (e.g. priority/1000);
* pathFromRoot() skips internal group names.
*/
class UIGroup : public UIElement {
    static int s_next_internal_id;

public:
    bool internal{false};

    UIGroup() = default;

    UIGroup(int id
        , const std::string& dir
        , const std::string& name
        , int priority = 0
        , const std::string& label = ""
        , const std::string& description = ""
        , const std::string& doc = ""
        , const ImageSet& icon = ImageSet()
        , bool visible = true
        , bool enabled = true
        )
        : UIElement(id
            , dir
            , name
            , priority
            , label
            , description
            , doc
            , icon
            , visible
            , enabled)
        {}

    bool isGroup() const override { return true; }
    
    /** path is relative, leading slash is ignored */
    UIElement* resolve(std::string_view path);
    /** path is relative, leading slash is ignored */
    UIGroup* resolveGroup(std::string_view path);
    /** path is relative, leading slash is ignored */
    UIGroup* resolveGroup(const Path& path);
    
    /** Build tree from flat list: set parent/children so paths form hierarchy; this is root. */
    void buildTree(std::vector<UIElement*>& elements);

    /**
     * Install this group into context menus/toolbars: for each element, find menus/toolbars
     * for element.dir() and install group -> submenu/subgroup, action -> menuitem/toolitem,
     * state -> checked item or radio submenu/radio tools. Call tearDown() to remove.
     */
    void setUp(UIWidgetsContext& context, InstallRecords& installs);

    /** Remove all menu/toolbar items installed by setUp(). */
    void tearDown(UIWidgetsContext& context);
    void tearDown(InstallRecords& installs);

    /** Dump node tree to output stream in format "id - label [priority]" */
    void dump(std::ostream& output = std::cout) const;

private:
    /** Helper method for recursive tree dumping */
    void dumpRecursive(std::ostream& output, std::string_view indent) const;

    std::vector<std::unique_ptr<UIGroup>> internals;

public:

    template<class builder_t, class T>
    class _Builder : public UIElement::_Builder<builder_t, T> {
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
            ): UIElement::_Builder<builder_t, T>(id
            , dir
            , name
            , priority
            , label
            , description
            , doc
            , icon
            , visible
            , enabled
            ) {}

        builder_t& internal(bool v)
            { m_internal = v; return this->self(); }

        void applyTo(UIGroup* el) {
            UIElement::_Builder<builder_t, T>::applyTo(el);
            el->internal = m_internal;
        }
    private:
        bool m_internal{false};
    };

    class Builder : public _Builder<Builder, UIGroup> {};
    static Builder builder() { return Builder(); }
};

#endif // UI_GROUP_H