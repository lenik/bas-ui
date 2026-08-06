#ifndef BAS_UI_AUTOMATION_IAUTOMATION_HPP
#define BAS_UI_AUTOMATION_IAUTOMATION_HPP

#include <boost/json.hpp>

#include <string>

namespace bas::ui::automation {

/**
 * In-process UI automation surface.
 *
 * Implementations synthesize wx events (typically via ProcessEvent) against
 * named controls so tests can drive dialogs without OS-level input injection.
 */
class IAutomation {
  public:
    virtual ~IAutomation() = default;

    /**
     * Emulate an interaction named by @p eventName on the control identified by
     * @p objId. Optional @p data carries event-specific fields (coordinates,
     * text, key codes, …).
     *
     * @return true on success. Implementations may throw AutomationError on
     *         hard failures, or return false for soft failures (see tryEmulate).
     */
    virtual bool emulate(const std::string& eventName, const std::string& objId,
                         const boost::json::object& data = {}) = 0;
};

} // namespace bas::ui::automation

#endif
