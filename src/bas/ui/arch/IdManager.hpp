#ifndef UI_ID_MANAGER_H
#define UI_ID_MANAGER_H

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

/** Metadata for an id issued by IdManager. */
template <typename K>
struct IdInfo {
    K id{};
    K parent{};
    int index{-1};
    long timestamp{0};
    std::string name;
    std::string description;
};

/**
 * Allocates monotonic ids of type K and stores IdInfo per id.
 * K must be hashable (for std::unordered_map), equality-comparable, and pre-/post-incrementable.
 */
template <typename K>
class IdManager {
  public:
    K next_id{};

    explicit IdManager(K start_id = K{1});

    IdInfo<K>& alloc(std::string_view name = "", std::string_view description = "");
    IdInfo<K>& alloc(K parent, int index, std::string_view name = "",
                     std::string_view description = "");

    void free(K id);
    std::optional<IdInfo<K>> getInfo(K id) const;

  private:
    std::unordered_map<K, IdInfo<K>> m_infos;
};

extern template class IdManager<int>;

#endif // UI_ID_MANAGER_H
