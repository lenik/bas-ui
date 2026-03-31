#ifndef UI_GROUP_H
#define UI_GROUP_H

#include "BuildViewContext.hpp"
#include "BuildViewLog.hpp"
#include "UIElement.hpp"

#include <wx/menu.h>
#include <wx/toolbar.h>

#include <iostream>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

/**
 * Group node. buildTree(elements) builds parent/children from a flat list
 * of elements using their path; this group is the root of the resulting tree.
 * When internal==true, this group is used only for grouping (e.g. priority/1000);
 * pathFromRoot() skips internal group names.
 */
class UIGroup : public UIElement {
  public:
    bool internal{false};

    UIGroup() = default;

    UIGroup(int id, const std::string& dir, const std::string& name, int priority = 0,
            const std::string& label = "", const std::string& description = "",
            const std::string& doc = "", const ImageSet& icon = ImageSet(), bool visible = true,
            bool enabled = true)
        : UIElement(id, dir, name, priority, label, description, doc, icon, visible, enabled) {}

    bool isGroup() const override { return true; }

    int actionCount() const;
    int flattenActionCount() const;

    /** path is relative, leading slash is ignored */
    UIElement* resolve(std::string_view path, BuildViewContext* context);
    /** path is relative, leading slash is ignored */
    UIGroup* resolveGroup(std::string_view path, BuildViewContext* context);
    /** path is relative, leading slash is ignored */
    UIGroup* resolveGroup(const Path& path, BuildViewContext* context);

    /** Add to tree from flat list: set parent/children so paths form hierarchy; this is root. */
    void addToTree(std::vector<UIElement*>& elements, BuildViewContext* context);

    /** Remove from tree:unset parent/children so paths form hierarchy; this is root. */
    void removeFromTree(std::vector<UIElement*>& elements);

    /**
     * Install this group into context menus/toolbars: for each element, find menus/toolbars
     * for element.dir() and install group -> submenu/subgroup, action -> menuitem/toolitem,
     * state -> checked item or radio submenu/radio tools. Call tearDown() to remove.
     */
    void buildView(BuildViewContext* context, BuildViewLogs* logs, //
                   std::optional<std::unordered_set<UIElement*>> white_set = std::nullopt);

    /** Remove all menu/toolbar items installed by setUp(). */
    void removeBuild(BuildViewContext* context, //
                     std::optional<std::unordered_set<UIElement*>> white_set = std::nullopt);
    void removeBuild(BuildViewLogs* logs);

    /** Dump node tree to output stream in format "id - label [priority]" */
    void dump(std::ostream& output = std::cout) const;

  private:
    /** Helper method for recursive tree dumping */
    void dumpRecursive(std::ostream& output, std::string_view indent) const;

    std::vector<std::unique_ptr<UIGroup>> internals;

  public:
    template <class builder_t, class T> class _Builder : public UIElement::_Builder<builder_t, T> {
      public:
        _Builder(int id = 0, const std::string& dir = "", const std::string& name = "",
                 int priority = 0, const std::string& label = "",
                 const std::string& description = "", const std::string& doc = "",
                 const ImageSet& icon = ImageSet(), bool visible = true, bool enabled = true)
            : UIElement::_Builder<builder_t, T>(id, dir, name, priority, label, description, doc,
                                                icon, visible, enabled) {}

        builder_t& internal(bool v) {
            m_internal = v;
            return this->self();
        }

        void applyTo(UIGroup* el) const {
            UIElement::_Builder<builder_t, T>::applyTo(el);
            el->internal = m_internal;
        }

        std::unique_ptr<UIGroup> build() const {
            std::unique_ptr<UIGroup> el = std::make_unique<UIGroup>();
            applyTo(el.get());
            return el;
        }

      private:
        bool m_internal{false};
    };

    class Builder : public _Builder<Builder, UIGroup> {};
    static Builder builder() { return Builder(); }
};

#endif // UI_GROUP_H