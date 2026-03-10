#include "UIState.hpp"

UIStateValueDescriptor UIState::getValueDescriptor(int value) const {
    if (valueDescriptorFn) {
        return valueDescriptorFn(value);
    }
    UIStateValueDescriptor d;
    d.label = "Value " + std::to_string(value);
    return d;
}
