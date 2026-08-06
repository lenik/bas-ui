#ifndef BAS_UI_AUTOMATION_ERROR_HPP
#define BAS_UI_AUTOMATION_ERROR_HPP

#include <stdexcept>
#include <string>

namespace bas::ui::automation {

class AutomationError : public std::runtime_error {
  public:
    AutomationError(const std::string& message) : std::runtime_error(message) {}

    AutomationError(const std::string& eventName, const std::string& objId,
                    const std::string& detail)
        : std::runtime_error(format(eventName, objId, detail)),
          m_eventName(eventName),
          m_objId(objId) {}

    const std::string& eventName() const noexcept { return m_eventName; }
    const std::string& objId() const noexcept { return m_objId; }

  private:
    static std::string format(const std::string& eventName, const std::string& objId,
                              const std::string& detail) {
        std::string msg = "automation";
        if (!eventName.empty()) {
            msg += " '" + eventName + "'";
        }
        if (!objId.empty()) {
            msg += " on '" + objId + "'";
        }
        if (!detail.empty()) {
            msg += ": " + detail;
        }
        return msg;
    }

    std::string m_eventName;
    std::string m_objId;
};

} // namespace bas::ui::automation

#endif
