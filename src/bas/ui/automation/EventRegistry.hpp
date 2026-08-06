#ifndef BAS_UI_AUTOMATION_EVENT_REGISTRY_HPP
#define BAS_UI_AUTOMATION_EVENT_REGISTRY_HPP

#include <bas/wx/wx_compat.hpp>

#include <boost/json.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace bas::ui::automation {

class DefaultAutomation;

/**
 * Maps normalized event names → handler methods on DefaultAutomation.
 *
 * Built once at startup; entries that need newer wx APIs are registered only
 * when BAS_WX_MODERN / wxCHECK_VERSION gates allow.
 */
class EventRegistry {
  public:
    using Handler = void (DefaultAutomation::*)(const std::string& objId,
                                                const boost::json::object& data);

    static const EventRegistry& instance();

    const Handler* find(const std::string& eventName) const;

    /** All registered canonical + alias names (for docs / dump). */
    std::vector<std::string> names() const;

  private:
    EventRegistry();
    void add(const char* name, Handler handler);
    void addAlias(const char* alias, const char* canonical);

    std::unordered_map<std::string, Handler> m_handlers;
};

} // namespace bas::ui::automation

#endif
