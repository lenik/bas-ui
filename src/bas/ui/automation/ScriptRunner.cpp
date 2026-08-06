#include "ScriptRunner.hpp"

#include "AutomationError.hpp"
#include "json_util.hpp"

#include <wx/utils.h>

#include <chrono>
#include <cctype>
#include <thread>

namespace bas::ui::automation {

namespace {

std::string toLower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

bool isWaitAction(const std::string& doName) {
    const std::string s = toLower(doName);
    return s == "wait" || s == "sleep" || s == "pause";
}

} // namespace

ScriptRunner::ScriptRunner(IAutomation& automation) : m_automation(automation) {}

void ScriptRunner::runJson(const std::string& jsonText) {
    boost::system::error_code ec;
    boost::json::value value = boost::json::parse(jsonText, ec);
    if (ec) {
        throw AutomationError("script", "", "invalid JSON: " + ec.message());
    }
    run(value);
}

void ScriptRunner::run(const boost::json::value& script) {
    if (script.is_array()) {
        run(script.as_array());
        return;
    }
    if (script.is_object()) {
        const auto& obj = script.as_object();
        if (const auto* steps = findValue(obj, "steps")) {
            if (!steps->is_array()) {
                throw AutomationError("script", "", "'steps' must be an array");
            }
            run(steps->as_array());
            return;
        }
        runStep(script);
        return;
    }
    throw AutomationError("script", "", "script must be an array or object");
}

void ScriptRunner::run(const boost::json::array& steps) {
    for (const auto& step : steps) {
        runStep(step);
    }
}

bool ScriptRunner::tryRun(const boost::json::array& steps) {
    try {
        run(steps);
        return true;
    } catch (const AutomationError&) {
        return false;
    }
}

void ScriptRunner::doWait(const boost::json::object& data) {
    const auto ms = static_cast<unsigned long>(getInt2(data, "ms", "millis", 0));
    if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    wxYield();
}

void ScriptRunner::runStep(const boost::json::value& step) {
    if (step.is_array()) {
        const auto& arr = step.as_array();
        if (arr.size() < 2 || !arr[0].is_string()) {
            throw AutomationError("script", "", "compact step needs [do, on, data?]");
        }
        const std::string doName = std::string(arr[0].as_string().c_str());
        const std::string on =
            arr[1].is_string() ? std::string(arr[1].as_string().c_str()) : jsonAsString(arr[1]);
        boost::json::object data;
        if (arr.size() >= 3) {
            if (!arr[2].is_object()) {
                throw AutomationError("script", on, "compact step data must be an object");
            }
            data = arr[2].as_object();
        }
        if (isWaitAction(doName)) {
            doWait(data);
            return;
        }
        if (!m_automation.emulate(doName, on, data)) {
            throw AutomationError(doName, on, "emulate returned false");
        }
        return;
    }

    if (!step.is_object()) {
        throw AutomationError("script", "", "step must be an object or array");
    }
    const auto& obj = step.as_object();
    std::string doName = getString2(obj, "do", "emulate");
    if (doName.empty()) {
        doName = getString(obj, "event");
    }
    std::string on = getString2(obj, "on", "target");
    if (on.empty()) {
        on = getString(obj, "objId");
    }

    boost::json::object data;
    if (const auto* d = findValue(obj, "data")) {
        if (!d->is_object()) {
            throw AutomationError(doName, on, "data must be an object");
        }
        data = d->as_object();
    }

    if (doName.empty()) {
        throw AutomationError("script", on, "step 'do' is required");
    }

    if (isWaitAction(doName)) {
        doWait(data);
        return;
    }

    if (on.empty()) {
        throw AutomationError(doName, on, "step 'on' is required");
    }

    if (!m_automation.emulate(doName, on, data)) {
        throw AutomationError(doName, on, "emulate returned false");
    }
}

} // namespace bas::ui::automation
