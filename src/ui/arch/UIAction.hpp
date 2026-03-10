#ifndef UI_ACTION_H
#define UI_ACTION_H

#include "UIElement.hpp"

#include <bas/script/property_support.hpp>

#include <wx/event.h>

#include <functional>
#include <string>
#include <vector>

class UIAction;

class PerformContext {
    static const char *EMPTY_ARGS[];

public:
    const UIAction* action{nullptr};
    std::time_t timestamp{std::time(nullptr)};

    int argc{0};
    const char* const* argv{nullptr};
    std::vector<std::string> args; // copy of argv
    int status{0}; // exit status

    const wxEvent* event{nullptr};
    const wxEvtHandler* event_handler{nullptr};

    // PerformContext() = default;

    PerformContext(
        const UIAction* action_
        , int argc
        , const char* const* argv
        , const wxEvent* event_
        , const wxEvtHandler* handler
        )
        : action(action_)
        , argc(argc)
        , argv(argv == nullptr ? EMPTY_ARGS : argv)
        , args(argv, argv + argc)
        , status(0)
        , event(event_)
        , event_handler(handler)
    {}

    PerformContext(PerformContext&&) = default;
    PerformContext& operator=(PerformContext&&) = default;
};
    
/** Action node: invokes performFn when triggered. */
class UIAction : public UIElement {
public:
    using PerformFn = std::function<void(PerformContext*)>;

    UIAction() = default;
    UIAction(int id
            , const std::string& dir
            , const std::string& name
            )
        : UIElement(id, dir, name)
        {}

    bool isAction() const override { return true; }

    /** For checkable menu/tool items. */
    bool isChecked() const { return checked.get(); }

    /** Invoke the action (e.g. from menu command). */
    void perform(PerformContext* ctx) const { if (performFn) performFn(ctx); }

protected:
    std::vector<std::string> shortcuts;
    PerformFn performFn;
    observable<bool> checked;
    
public:
    template<class builder_t, class T>
    class _Builder : public UIElement::_Builder<builder_t, T> {
        public:
            _Builder() = default;
            _Builder(int id
                , const std::string& dir
                , const std::string& name
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

            builder_t& shortcuts(std::vector<std::string> v)
                { m_shortcuts = v; return this->self(); }
            builder_t& addShortcut(std::string s)
                { m_shortcuts.push_back(s); return this->self(); }
            builder_t& performFn(UIAction::PerformFn fn)
                { m_performFn = fn; return this->self(); }
            builder_t& checked(bool v)
                { m_checked = v; return this->self(); }
        
            void applyTo(UIAction* el) {
                UIElement::_Builder<builder_t, T>::applyTo(el);
                el->shortcuts = m_shortcuts;
                el->performFn = m_performFn;
                el->checked.set(m_checked);
            }

        private:
            std::vector<std::string> m_shortcuts;
            UIAction::PerformFn m_performFn;
            bool m_checked{false};
    };

    class Builder : public _Builder<Builder, UIAction> {};
    static Builder builder() { return Builder(); }
};

#endif // UI_ACTION_H