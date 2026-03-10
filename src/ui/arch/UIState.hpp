#ifndef UI_STATE_H
#define UI_STATE_H

#include "UIElement.hpp"

#include <bas/script/property_support.hpp>

#include <functional>
#include <string>
#include <variant>

/** State type for UIState. */
enum class UIStateType {
    BOOL,   // checked / unchecked
    ENUM,   // radio group (int)
    STRING, // text (e.g. status bar)
    ICON,   // icon (iconId/iconAsset)
    IMAGE,  // image asset
};

/** Value held by UIState. */
using UIStateVariant = std::variant<
    bool, 
    int, 
    double, 
    std::string, 
    ImageSet
    >;

/** Descriptor for one ENUM value: label, icon, help for UI. */
struct UIStateValueDescriptor {
    std::string label;
    std::string description;
    std::string doc;
    ImageSet icon;
};

/**
 * State node: get/set for BOOL, ENUM, STRING, ICON, IMAGE.
 * IStatusItem-like: position ("left"/"right"/"none"), priority, visibility, icon, text.
 */
class UIState : public UIElement {
public:
    using ValueDescriptorFn = std::function<UIStateValueDescriptor(int value)>;

    UIStateType stateType{UIStateType::BOOL};
    int enumValueCount{0};
    ValueDescriptorFn valueDescriptorFn;

    observable<UIStateVariant> value;

    UIState() = default;
    UIState(int id
            , const std::string& dir
            , const std::string& name
            , UIStateType type = UIStateType::BOOL
            , ValueDescriptorFn valueDesc = nullptr
            )
        : UIElement(id, dir, name)
        , stateType(type)
        , valueDescriptorFn(std::move(valueDesc))
        {}

    bool isState() const override { return true; }
    
    UIStateType getType() const { return stateType; }
    int getEnumCount() const { return enumValueCount; }

    /** For ENUM: return label, icon, helpDoc for the given value. */
    UIStateValueDescriptor getValueDescriptor(int value) const;

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

            builder_t& stateType(UIStateType t)
                { m_stateType = t; return this->self(); }      
            builder_t& valueDescriptorFn(UIState::ValueDescriptorFn fn)
                { m_valueDescriptorFn = fn; return this->self(); }
        
            void applyTo(UIState* el) {
                UIElement::_Builder<builder_t, T>::applyTo(el);
                el->stateType = m_stateType;
                el->valueDescriptorFn = m_valueDescriptorFn;
            }

        private:
            UIStateType m_stateType{UIStateType::BOOL};
            UIState::ValueDescriptorFn m_valueDescriptorFn{nullptr};
    };

    class Builder : public _Builder<Builder, UIState> {};
    static Builder builder() { return Builder(); }
};

#endif // UI_STATE_H