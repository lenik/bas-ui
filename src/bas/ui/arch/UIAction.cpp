#include "UIAction.hpp"

#include <bas/log/uselog.h>

const char *PerformContext::EMPTY_ARGS[] = { nullptr };

void UIAction::perform(PerformContext* ctx) {
    if (m_performFn) {
        m_performFn(ctx);
    }
}
