#ifndef BAS_UI_AUTOMATION_SCRIPT_RUNNER_HPP
#define BAS_UI_AUTOMATION_SCRIPT_RUNNER_HPP

#include "IAutomation.hpp"

#include <boost/json.hpp>

#include <string>

namespace bas::ui::automation {

/**
 * Plays a JSON array of automation steps against an IAutomation target.
 *
 * Step shape:
 *   {"do": "click", "on": "btn1", "data": {"x": 10, "y": 20}}
 *
 * Special step "wait" is handled here (wxYield + sleep); it is not forwarded
 * to DefaultAutomation.
 */
class ScriptRunner {
  public:
    explicit ScriptRunner(IAutomation& automation);

    /** Run a JSON array value (or a string containing JSON). Throws on failure. */
    void run(const boost::json::value& script);
    void run(const boost::json::array& steps);
    void runJson(const std::string& jsonText);

    /** Soft-fail: returns false on first error instead of throwing. */
    bool tryRun(const boost::json::array& steps);

  private:
    void runStep(const boost::json::value& step);
    static void doWait(const boost::json::object& data);

    IAutomation& m_automation;
};

} // namespace bas::ui::automation

#endif
