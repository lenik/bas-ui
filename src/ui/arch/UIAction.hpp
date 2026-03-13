#ifndef UI_ACTION_H
#define UI_ACTION_H

#include "UIElement.hpp"

#include <bas/log/uselog.h>
#include <bas/script/property_support.hpp>

#include <wx/event.h>

#include <functional>
#include <string>
#include <vector>

class UIAction;
class UIState;

class PerformContext {
    static const char *EMPTY_ARGS[];

public:
    UIAction* const action{nullptr};
    UIState* const state{nullptr};
    std::time_t timestamp{std::time(nullptr)};

    int argc{0};
    const char* const* argv{nullptr};
    std::vector<std::string> args; // copy of argv
    int status{0}; // exit status

    const wxEvent* event{nullptr};
    const wxEvtHandler* event_handler{nullptr};

    // PerformContext() = default;

    PerformContext(
        UIAction* const action_
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

    /** Invoke the action (e.g. from menu command). */
    void perform(PerformContext* ctx);

protected:
    std::vector<std::string> shortcuts;
    PerformFn performFn;
    
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
            builder_t& performFn(UIAction::PerformFn&& fn)
                { m_performFn = std::move(fn); return this->self(); }
        
            void applyTo(UIAction* el) const {
                UIElement::_Builder<builder_t, T>::applyTo(el);
                el->shortcuts = m_shortcuts;
                el->performFn = std::move(m_performFn);
            }

            std::unique_ptr<UIAction> build() const {
                std::unique_ptr<UIAction> el = std::make_unique<UIAction>();
                applyTo(el.get());
                return el;
            }
            
        private:
            std::vector<std::string> m_shortcuts;
            UIAction::PerformFn m_performFn;
    };

    class Builder : public _Builder<Builder, UIAction> {};
    static Builder builder() { return Builder(); }
};

#endif // UI_ACTION_H