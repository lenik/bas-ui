#include "UIState.hpp"

std::optional<UIStateValueDescriptor> UIState::getValueDescriptor(int value) const {
    if (valueDescriptorFn) {
        return valueDescriptorFn(value);
    }
    return std::nullopt;
}

std::optional<UIStateValueDescriptor> UIState::findValueDescriptorById(int id) const {
    for (int value : enumValues) {
        auto d = getValueDescriptor(value);
        if (!d) {
            continue;
        }
        if (d->m_id == id) {
            return d;
        }
    }
    return std::nullopt;
}

std::optional<int> UIState::findValueById(int id) const {
    for (int value : enumValues) {
        auto d = getValueDescriptor(value);
        if (!d) {
            continue;
        }
        if (d->m_id == id) {
            return value;
        }
    }
    return std::nullopt;
}
