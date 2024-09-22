#ifndef hashmap_h
#define hashmap_h

#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <functional>
#include <utility>

using namespace std;

template <typename T>
class HashMap {
public:
    HashMap(double max_load_factor = 0.75)
    : max_load_factor(max_load_factor), buckets(10), count(0) {}

    ~HashMap() {
        clear();
    }

    int size() const {
        return count;
    }
    
    void insert(const string& key, const T& value) {
        checks();
        auto& bucket = buckets[getBucketIndex(key)];
        auto it = findInBucket(bucket, key);
        if (it != bucket.end()) {
            it->second = value;
        } else {
            bucket.emplace_back(key, value);
            ++count;
        }
    }

    
    T& operator[](const string& key) {
        checks();
        auto& bucket = buckets[getBucketIndex(key)];
        auto it = findInBucket(bucket, key);
        if (it == bucket.end()) {
            bucket.emplace_back(key, T());
            ++count;
            it = prev(bucket.end());
        }
        return it->second;
    }
    
   const T* find(const string& key) const {
        const auto& bucket = buckets[getBucketIndex(key)];
        auto it = findInBucket(bucket, key);
        return it != bucket.end() ? &it->second : nullptr;
    }


    T* find(const string& key) {
     const auto& hm = *this;
     return const_cast<T*>(hm.find(key));
     }

private:
    double max_load_factor;
        vector<list<pair<string, T>>> buckets;
        int count;

    size_t getBucketIndex(const string& key, size_t customSize = 0) const {
        hash<string> hasher;
        size_t size = customSize ? customSize : buckets.size();
        return hasher(key) % size;
    }

        void rehash(size_t new_size) {
            vector<list<pair<string, T>>> new_buckets(new_size);
            for (const auto& bucket : buckets) {
                for (const auto& item : bucket) {
                    size_t newIndex = getBucketIndex(item.first, new_size);
                    new_buckets[newIndex].push_back(item);

                }
            }
            buckets = std::move(new_buckets);
        }

    
        
        void checks() {
            if (static_cast<double>(count) / buckets.size() >= max_load_factor) {
                rehash(buckets.size() * 2);
            }
        }

        typename list<pair<string, T>>::const_iterator findInBucket(const list<pair<string, T>>& bucket, const string& key) const {
            return find_if(bucket.begin(), bucket.end(), [&key](const pair<string, T>& item) {
                return item.first == key;
            });
        }

        typename list<pair<string, T>>::iterator findInBucket(list<pair<string, T>>& bucket, const string& key) {
            return find_if(bucket.begin(), bucket.end(), [&key](const pair<string, T>& item) {
                return item.first == key;
            });
        }

        void clear() {
            for (auto& bucket : buckets) {
                bucket.clear();
            }
            count = 0;
        }
    };

#endif /* hashmap_h */
