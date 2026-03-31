#ifndef UI_STATE_H
#define UI_STATE_H

#include "BuildViewContext.hpp"
#include "UIElement.hpp"

#include <bas/script/property_support.hpp>

#include <functional>
#include <initializer_list>
#include <string>
#include <variant>

// inline int enumItemId(int stateId, int value) {
//     int id = stateId * 1000 + value;
//     int l = wxID_AUTO_LOWEST;
//     int h = wxID_AUTO_HIGHEST;
//     return id;
// }

/** State type for UIState. */
enum class UIStateType {
    BOOL,   // checked / unchecked
    ENUM,   // radio group (int)
    STRING, // text (e.g. status bar)
    ICON,   // icon (iconId/iconAsset)
    IMAGE,  // image asset
};

/** Value held by UIState. */
using UIStateVariant = std::variant<bool, int, double, std::string, ImageSet>;

/** Descriptor for one ENUM value: label, icon, help for UI. */
struct UIStateValueDescriptor {
    std::string label;
    std::string description;
    std::string doc;
    ImageSet icon;
    int m_id{-1};

    int id(BuildViewContext* context) {
        if (m_id == -1) {
            if (context == nullptr) {
                throw std::runtime_error("context is required");
            } else {
                m_id = context->getNextId();
            }
        }
        return m_id;
    }
};

/**
 * State node: get/set for BOOL, ENUM, STRING, ICON, IMAGE.
 * IStatusItem-like: position ("left"/"right"/"none"), priority, visibility, icon, text.
 */
class UIState : public UIElement {
  public:
    using ValueDescriptorFn = std::function<UIStateValueDescriptor(int value)>;

    UIStateType stateType{UIStateType::BOOL};
    std::vector<int> enumValues;
    ValueDescriptorFn valueDescriptorFn;

    observable<UIStateVariant> value;

    UIState() = default;
    UIState(int id, const std::string& dir, const std::string& name,
            UIStateType type = UIStateType::BOOL, ValueDescriptorFn valueDesc = nullptr)
        : UIElement(id, dir, name), stateType(type), valueDescriptorFn(std::move(valueDesc)) {}

    bool isState() const override { return true; }

    UIStateType getType() const { return stateType; }
    const std::vector<int>& getEnumValues() const { return enumValues; }
    int getEnumCount() const { return enumValues.size(); }

    /** For ENUM: return label, icon, helpDoc for the given value. */
    UIStateValueDescriptor getValueDescriptor(int value) const;

    std::optional<UIStateValueDescriptor> findValueDescriptorById(int id) const;
    std::optional<int> findValueById(int id) const;

    template <class builder_t, class T> class _Builder : public UIElement::_Builder<builder_t, T> {
      public:
        _Builder() = default;
        _Builder(int id, const std::string& dir, const std::string& name, int priority = 0,
                 const std::string& label = "", const std::string& description = "",
                 const std::string& doc = "", const ImageSet& icon = ImageSet(),
                 bool visible = true, bool enabled = true)
            : UIElement::_Builder<builder_t, T>(id, dir, name, priority, label, description, doc,
                                                icon, visible, enabled) {}

        builder_t& stateType(UIStateType t) {
            m_stateType = t;
            return this->self();
        }
        builder_t& enumValues(std::initializer_list<int> values) {
            m_enumValues = values;
            return this->self();
        }
        builder_t& valueDescriptorFn(UIState::ValueDescriptorFn fn) {
            m_valueDescriptorFn = fn;
            return this->self();
        }
        builder_t& initValue(UIStateVariant v) {
            m_initValue = v;
            return this->self();
        }
        builder_t& valueRef(observable<UIStateVariant>** ref) {
            m_valueRef = ref;
            return this->self();
        }
        builder_t& connect(observable<UIStateVariant>::slot_type&& slot) {
            m_slot = std::move(slot);
            return this->self();
        }

        void applyTo(UIState* el) const {
            UIElement::_Builder<builder_t, T>::applyTo(el);
            el->stateType = m_stateType;
            el->enumValues = m_enumValues;
            el->valueDescriptorFn = m_valueDescriptorFn;
            if (m_initValue) {
                el->value.set(*m_initValue);
            }
            if (m_slot) {
                el->value.connect(*m_slot);
            }
        }

        std::unique_ptr<UIState> build() const {
            std::unique_ptr<UIState> el = std::make_unique<UIState>();
            applyTo(el.get());
            if (m_valueRef) {
                *m_valueRef = &el->value;
            }
            return el;
        }

      private:
        UIStateType m_stateType{UIStateType::BOOL};
        std::vector<int> m_enumValues;
        UIState::ValueDescriptorFn m_valueDescriptorFn{nullptr};
        observable<UIStateVariant>** m_valueRef;
        std::optional<UIStateVariant> m_initValue;
        std::optional<observable<UIStateVariant>::slot_type> m_slot;
    };

    class Builder : public _Builder<Builder, UIState> {};
    static Builder builder() { return Builder(); }
};

#endif // UI_STATE_H