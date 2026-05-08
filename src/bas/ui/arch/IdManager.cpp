#include "IdManager.hpp"

#include <ctime>
#include <stdexcept>
#include <string>
#include <utility>

template <typename K>
IdManager<K>::IdManager(K start_id) : next_id(start_id) {}

template <typename K>
IdInfo<K>& IdManager<K>::alloc(std::string_view name, std::string_view description) {
    const K id = next_id;
    ++next_id;

    IdInfo<K> info{};
    info.id = id;
    info.timestamp = static_cast<long>(std::time(nullptr));
    info.name = name;
    info.description = description;

    auto [it, inserted] = m_infos.emplace(id, std::move(info));
    if (!inserted) {
        throw std::runtime_error("id already exists: " + std::to_string(id));
    }
    return it->second;
}

template <typename K>
IdInfo<K>& IdManager<K>::alloc(K parent, int index, std::string_view name,
                               std::string_view description) {
    const K id = next_id;
    ++next_id;

    std::string nameStr(name);
    if (nameStr.empty()) {
        nameStr = "[" + std::to_string(index) + "]";
    }

    IdInfo<K> info{};
    info.id = id;
    info.timestamp = static_cast<long>(std::time(nullptr));
    info.parent = parent;
    info.index = index;
    info.name = std::move(nameStr);
    info.description = description;

    auto [it, inserted] = m_infos.emplace(id, std::move(info));
    if (!inserted) {
        throw std::runtime_error("id already exists: " + std::to_string(id));
    }
    return it->second;
}

template <typename K>
void IdManager<K>::free(K id) {
    m_infos.erase(id);
}

template <typename K>
std::optional<IdInfo<K>> IdManager<K>::getInfo(K id) const {
    auto it = m_infos.find(id);
    if (it == m_infos.end()) {
        return std::nullopt;
    }
    return it->second;
}

template class IdManager<int>;
