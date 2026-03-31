#include "UIState.hpp"

UIStateValueDescriptor UIState::getValueDescriptor(int value) const {
    if (valueDescriptorFn) {
        return valueDescriptorFn(value);
    }
    UIStateValueDescriptor d;
    d.label = "Value " + std::to_string(value);
    return d;
}

std::optional<UIStateValueDescriptor> UIState::findValueDescriptorById(int id) const {
    for (int value : enumValues) {
        UIStateValueDescriptor d = getValueDescriptor(value);
        if (d.m_id == id) {
            return d;
        }
    }
    return std::nullopt;
}

std::optional<int> UIState::findValueById(int id) const {
    for (int value : enumValues) {
        UIStateValueDescriptor d = getValueDescriptor(value);
        if (d.m_id == id) {
            return value;
        }
    }
    return std::nullopt;
}
