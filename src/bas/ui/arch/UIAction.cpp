#include "UIAction.hpp"

#include <bas/log/uselog.h>

const char *PerformContext::EMPTY_ARGS[] = { nullptr };

void UIAction::perform(PerformContext* ctx) {
    if (performFn) {
        performFn(ctx);
    }
}
