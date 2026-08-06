#ifndef BAS_UI_AUTOMATION_JSON_UTIL_HPP
#define BAS_UI_AUTOMATION_JSON_UTIL_HPP

#include <boost/json.hpp>

#include <cstdint>
#include <string>

namespace bas::ui::automation {

inline const boost::json::value* findValue(const boost::json::object& data, const char* key) {
    if (!key) {
        return nullptr;
    }
    const auto it = data.find(key);
    if (it == data.end()) {
        return nullptr;
    }
    return &it->value();
}

inline const boost::json::value* findValue(const boost::json::object& data, const char* keyA,
                                           const char* keyB) {
    if (const auto* v = findValue(data, keyA)) {
        return v;
    }
    return findValue(data, keyB);
}

inline bool jsonAsBool(const boost::json::value& v, bool fallback = false) {
    if (v.is_bool()) {
        return v.as_bool();
    }
    if (v.is_int64()) {
        return v.as_int64() != 0;
    }
    if (v.is_uint64()) {
        return v.as_uint64() != 0;
    }
    if (v.is_string()) {
        const auto& s = v.as_string();
        return s == "1" || s == "true" || s == "True" || s == "TRUE" || s == "yes";
    }
    return fallback;
}

inline std::int64_t jsonAsInt(const boost::json::value& v, std::int64_t fallback = 0) {
    if (v.is_int64()) {
        return v.as_int64();
    }
    if (v.is_uint64()) {
        return static_cast<std::int64_t>(v.as_uint64());
    }
    if (v.is_double()) {
        return static_cast<std::int64_t>(v.as_double());
    }
    if (v.is_bool()) {
        return v.as_bool() ? 1 : 0;
    }
    if (v.is_string()) {
        try {
            return std::stoll(std::string(v.as_string().c_str()));
        } catch (...) {
            return fallback;
        }
    }
    return fallback;
}

inline double jsonAsDouble(const boost::json::value& v, double fallback = 0.0) {
    if (v.is_double()) {
        return v.as_double();
    }
    if (v.is_int64()) {
        return static_cast<double>(v.as_int64());
    }
    if (v.is_uint64()) {
        return static_cast<double>(v.as_uint64());
    }
    if (v.is_string()) {
        try {
            return std::stod(std::string(v.as_string().c_str()));
        } catch (...) {
            return fallback;
        }
    }
    return fallback;
}

inline std::string jsonAsString(const boost::json::value& v, const char* fallback = "") {
    if (v.is_string()) {
        return std::string(v.as_string().c_str());
    }
    if (v.is_int64()) {
        return std::to_string(v.as_int64());
    }
    if (v.is_uint64()) {
        return std::to_string(v.as_uint64());
    }
    if (v.is_double()) {
        return std::to_string(v.as_double());
    }
    if (v.is_bool()) {
        return v.as_bool() ? "true" : "false";
    }
    return fallback ? std::string(fallback) : std::string{};
}

inline bool getBool(const boost::json::object& data, const char* key, bool fallback = false) {
    const auto* v = findValue(data, key);
    return v ? jsonAsBool(*v, fallback) : fallback;
}

inline std::int64_t getInt(const boost::json::object& data, const char* key,
                           std::int64_t fallback = 0) {
    const auto* v = findValue(data, key);
    return v ? jsonAsInt(*v, fallback) : fallback;
}

/** First matching key among keyA/keyB. */
inline std::int64_t getInt2(const boost::json::object& data, const char* keyA, const char* keyB,
                            std::int64_t fallback = 0) {
    const auto* v = findValue(data, keyA, keyB);
    return v ? jsonAsInt(*v, fallback) : fallback;
}

inline double getDouble(const boost::json::object& data, const char* key, double fallback = 0.0) {
    const auto* v = findValue(data, key);
    return v ? jsonAsDouble(*v, fallback) : fallback;
}

inline std::string getString(const boost::json::object& data, const char* key,
                             const char* fallback = "") {
    const auto* v = findValue(data, key);
    return v ? jsonAsString(*v, fallback) : std::string(fallback ? fallback : "");
}

/** First matching key among keyA/keyB. */
inline std::string getString2(const boost::json::object& data, const char* keyA, const char* keyB,
                              const char* fallback = "") {
    const auto* v = findValue(data, keyA, keyB);
    return v ? jsonAsString(*v, fallback) : std::string(fallback ? fallback : "");
}

/** Mouse X: prefers `x`, falls back to `left`. */
inline int getMouseX(const boost::json::object& data, int fallback = 0) {
    return static_cast<int>(getInt2(data, "x", "left", fallback));
}

/** Mouse Y: prefers `y`, falls back to `top`. */
inline int getMouseY(const boost::json::object& data, int fallback = 0) {
    return static_cast<int>(getInt2(data, "y", "top", fallback));
}

} // namespace bas::ui::automation

#endif
