#ifndef UI_FRAGMENT_H
#define UI_FRAGMENT_H

#include "CreateViewContext.hpp"
#include "UIAction.hpp"
#include "UIElement.hpp"
#include "UIGroup.hpp"
#include "UIState.hpp"

#include <bas/script/scriptable.hpp>

#include <vector>

/**
 * Mixin interface: provides a flat list of UIElement for the main UI
 * to build a tree.
 * Fragment-provided elements should use absolute paths, e.g. /dir/../name.
 */
class UIFragment : public IScriptSymbols {
  public:
    virtual ~UIFragment() {}

    // override this to load elements delayed after object construction
    virtual std::vector<UIElement*> loadElements() { return m_elements; }

    virtual wxEvtHandler* getEventHandler() = 0;

    PerformContext toPerformContext(wxEvent& event);

    // UI lifecycle

    virtual void createView(CreateViewContext* ctx) {}
    virtual void destroyView(CreateViewContext* ctx) {}

    // scripting
    std::vector<std::string> findVars(std::string_view prefix) override;

    bool hasVar(std::string_view name) override;

    std::string formatVar(std::string_view name) override;

    void parseVar(std::string_view name, std::string_view str) override;

    std::vector<std::string> findMethods(std::string_view prefix) override;

    bool hasMethod(std::string_view name) override;

    int invokeMethod(std::string_view name, const char* const* argv, int argc) override;

    class ActionBuilder : public UIAction::_Builder<ActionBuilder, UIAction> {
      public:
        ActionBuilder(UIFragment* owner) : m_owner(owner) {}
        ActionBuilder(UIFragment* owner, int id, const std::string& dir, const std::string& name,
                      int priority = 0, const std::string& label = "",
                      const std::string& description = "", const std::string& doc = "",
                      const ImageSet& icon = ImageSet(), bool visible = true, bool enabled = true)
            : m_owner(owner),
              UIAction::_Builder<ActionBuilder, UIAction>(
                  id, dir, name, priority, label, description, doc, icon, visible, enabled) {}
        void install() {
            auto owned = build();
            auto action = owned.get();
            m_owner->m_owned_elements.push_back(std::move(owned));
            m_owner->m_elements.push_back(action);
        }

      private:
        UIFragment* m_owner;
    };

    class StateBuilder : public UIState::_Builder<StateBuilder, UIState> {
      public:
        StateBuilder(UIFragment* owner) : m_owner(owner) {}
        StateBuilder(UIFragment* owner, int id, const std::string& dir, const std::string& name,
                     int priority = 0, const std::string& label = "",
                     const std::string& description = "", const std::string& doc = "",
                     const ImageSet& icon = ImageSet(), bool visible = true, bool enabled = true)
            : m_owner(owner),
              UIState::_Builder<StateBuilder, UIState>(id, dir, name, priority, label, description,
                                                       doc, icon, visible, enabled) {}

        void install() {
            auto owned = build();
            auto state = owned.get();
            m_owner->m_owned_elements.push_back(std::move(owned));
            m_owner->m_elements.push_back(state);
        }

      private:
        UIFragment* m_owner;
    };

    class GroupBuilder : public UIGroup::_Builder<GroupBuilder, UIGroup> {
      public:
        GroupBuilder(UIFragment* owner) : m_owner(owner) {}
        GroupBuilder(UIFragment* owner, int id, const std::string& dir, const std::string& name,
                     int priority = 0, const std::string& label = "",
                     const std::string& description = "", const std::string& doc = "",
                     const ImageSet& icon = ImageSet(), bool visible = true, bool enabled = true)
            : m_owner(owner),
              UIGroup::_Builder<GroupBuilder, UIGroup>(id, dir, name, priority, label, description,
                                                       doc, icon, visible, enabled) {}

        void install() {
            auto owned = build();
            auto group = owned.get();
            m_owner->m_owned_elements.push_back(std::move(owned));
            m_owner->m_elements.push_back(group);
        }

      private:
        UIFragment* m_owner;
    };
    ActionBuilder action() { return ActionBuilder(this); }
    StateBuilder state() { return StateBuilder(this); }
    GroupBuilder group() { return GroupBuilder(this); }

    ActionBuilder action(int id, const std::string& dir, const std::string& name, int priority = 0,
                         const std::string& label = "", const std::string& description = "",
                         const std::string& doc = "", const ImageSet& icon = ImageSet(),
                         bool visible = true, bool enabled = true) {
        return ActionBuilder(this, id, dir, name, priority, label, description, doc, icon, visible,
                             enabled);
    }

    StateBuilder state(int id, const std::string& dir, const std::string& name, int priority = 0,
                       const std::string& label = "", const std::string& description = "",
                       const std::string& doc = "", const ImageSet& icon = ImageSet(),
                       bool visible = true, bool enabled = true) {
        return StateBuilder(this, id, dir, name, priority, label, description, doc, icon, visible,
                            enabled);
    }

    GroupBuilder group(int id, const std::string& dir, const std::string& name, int priority = 0,
                       const std::string& label = "", const std::string& description = "",
                       const std::string& doc = "", const ImageSet& icon = ImageSet(),
                       bool visible = true, bool enabled = true) {
        return GroupBuilder(this, id, dir, name, priority, label, description, doc, icon, visible,
                            enabled);
    }

  private:
    std::vector<std::unique_ptr<UIElement>> m_owned_elements;
    std::vector<UIElement*> m_elements;
};

#endif // UI_FRAGMENT_H