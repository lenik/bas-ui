#ifndef UTIL_VECTORS_HPP
#define UTIL_VECTORS_HPP

#include "../ui/arch/UIElement.hpp"

#include <vector>

inline std::vector<UIElement*> operator+(const std::vector<UIElement*>& a,
                                         const std::vector<UIElement*>& b) {
    std::vector<UIElement*> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

#endif // UTIL_VECTORS_HPP