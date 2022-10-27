#include "record_set.h"
#include <spdlog/spdlog.h>


namespace mls {

    std::optional<record::value_type> record_set::find(record::key_type key) const noexcept
    {
        // see if we can locate the key inside the records
        auto iter = std::lower_bound(begin(_records), end(_records), key, [](const record& record, const record::key_type& key) {
            return record.key < key;
        });

        // check if we found the right record
        if (iter != end(_records) && iter->key == key) {
            return iter->value;
        }

        // no match found
        return std::nullopt;
    }

    void record_set::merge(std::vector<record>&& set) noexcept
    {
        // ensure the data is sorted
        spdlog::debug("Sorting dataset");
        std::sort(begin(set), end(set), [](const auto& a, const auto& b) {
            return a.key < b.key;
        });

        // if we have no existing set, we can take the new set as-is
        if (_records.empty()) {
            spdlog::debug("Replacing empty recordset");
            _records = std::move(set);
            return;
        }

        // retrieve iterators to both sets
        auto old_iter = begin(_records);
        auto new_iter = begin(set);

        std::uint64_t updated{};
        std::uint64_t inserted{};

        // process all records until we reach the end
        while (old_iter != end(_records) && new_iter != end(set)) {
            // if both iterators point to the same key, we should
            // simply update the existing record with new data
            if (old_iter->key == new_iter->key) {
                ++updated;
                old_iter->value = new_iter->value;

                ++old_iter;
                ++new_iter;
                continue;
            }

            // if the existing records are smaller the new record
            // must come after it, so move on to the next
            if (old_iter->key < new_iter->key) {
                ++old_iter;
                continue;
            }

            ++inserted;
            old_iter = _records.insert(old_iter, *new_iter);
            ++new_iter;
        }

        // all remaining new records go at the end
        _records.insert(end(_records), new_iter, end(set));
        inserted += std::distance(new_iter, end(set));

        spdlog::debug("Merge complete, inserted {} new records, updated {}", inserted, updated);
    }

}
