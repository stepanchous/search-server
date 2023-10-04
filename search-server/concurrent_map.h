#pragma once

#include <map>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

constexpr const size_t DEFAULT_BUCKET_COUNT = 128;

template <typename Key, typename Value>
class ConcurrentMap {
   private:
    struct Bucket {
        std::mutex lock;
        std::map<Key, Value> submap;
    };

   public:
    static_assert(std::is_integral_v<Key>,
                  "ConcurrentMap supports only integer keys");

    struct Access {
        Access(const Key& key, Bucket& bucket)
            : guard(bucket.lock), ref_to_value(bucket.submap[key]) {}

        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count = DEFAULT_BUCKET_COUNT)
        : buckets_(bucket_count) {}

    Access operator[](const Key& key) {
        return Access(key,
                      buckets_[static_cast<uint64_t>(key) % buckets_.size()]);
    }

    int erase(Key key) {
        uint64_t bucket_number = static_cast<uint64_t>(key) % buckets_.size();
        std::lock_guard guard(buckets_[bucket_number].lock);
        return buckets_[bucket_number].submap.erase(key);
    }

    size_t size() {
        size_t size = 0;
        for (auto& [lock, submap] : buckets_) {
            std::lock_guard guard(lock);
            size += submap.size();
        }
        return size;
    }

    std::map<Key, Value> BuildMap() {
        std::map<Key, Value> joined_map;
        for (auto& [lock, submap] : buckets_) {
            std::lock_guard guard(lock);
            joined_map.insert(submap.begin(), submap.end());
        }
        return joined_map;
    }

   private:
    std::vector<Bucket> buckets_;
};
