#ifndef INSERTION_ORDERED_MAP_H
#define INSERTION_ORDERED_MAP_H

#include <unordered_map>
#include <list>
#include <limits>
#include <memory>
#include <cassert>

const static size_t unshareable = std::numeric_limits<size_t>::max();

class lookup_error : std::exception {
    [[nodiscard]] const char *what() const noexcept override {
        return "lookup_error";
    }
};

template<class K, class V, class Hash = std::hash<K>>
class insertion_ordered_map {
private:
    using pair_t = std::pair<K, V>;
    using list_t = std::list<pair_t>;
    using map_t = std::unordered_map<const K, typename list_t::iterator, Hash>;

    class data_t {
    private:
        void fill_copy(data_t const &other) {
            for (auto it = other.list->begin(); it != other.list->end(); ++it) {
                auto list_it = list->insert(list->end(), std::pair(*it));
                map->insert({list_it->first, list_it});
            }
        }

    public: //TODO handle this
        std::unique_ptr<map_t> map;
        std::unique_ptr<list_t> list;
        size_t ref_count;

        data_t() : map(std::make_unique<map_t>()), list(std::make_unique<list_t>()), ref_count(1) {};

        data_t(data_t const &other) : map(std::make_unique<map_t>()), list(std::make_unique<list_t>()), ref_count(1) {
            fill_copy(other);
        };

        ~data_t() = default;

    };

    std::shared_ptr<data_t> data;

    using backup_data_t = std::pair<std::shared_ptr<data_t>, size_t>;

    backup_data_t prepare_to_modify(bool mark_unshareable) {
        backup_data_t backup = {data, data->ref_count};


        if (data->ref_count > 1 && data->ref_count != unshareable) {
            std::shared_ptr<data_t> prev_data = data;

            // Throws std::bad_alloc in case of memory allocation error and structure remains unchanged.
            data = std::make_shared<data_t>(*data);
            prev_data->ref_count--;
        }

        data->ref_count = mark_unshareable ? unshareable : 1;

        return backup;
    }

    void restore_data(backup_data_t const &backup) {
        backup.first->ref_count = backup.second;
        data = backup.first;
    }

    void copy(insertion_ordered_map const &other) {
        if (other.data->ref_count == unshareable) {
            // Throws std::bad_alloc in case of memory allocation error and structure remains unchanged.
            data = std::make_shared<data_t>(*other.data);
        } else {
            data = other.data;
            ++data->ref_count;
        }
    }

public:
    using iterator = typename list_t::const_iterator;

    insertion_ordered_map() noexcept : data(std::make_shared<data_t>()) {};

    insertion_ordered_map(insertion_ordered_map const &other) {
        copy(other);
    };

    ~insertion_ordered_map() noexcept {
        if (data)
            data->ref_count--;
    };

    insertion_ordered_map(insertion_ordered_map &&other) noexcept : data(move(other.data)) {}

    insertion_ordered_map &operator=(insertion_ordered_map other) {
        copy(other);
        return *this;
    };

    bool insert(K const &k, V const &v) {
        // Throws std::bad_alloc in case of memory allocation error and structure remains unchanged.
        backup_data_t backup = prepare_to_modify(false);

        pair_t pair;
        bool duplicate = false;
        typename map_t::iterator map_it = data->map->find(k);
        typename list_t::iterator list_it;

        if (map_it != data->map->end()) {
            duplicate = true;
            list_it = map_it->second;
            pair = *list_it;
        } else {
            pair = {k, v};
        }

        try {
            auto new_list_it = data->list->insert(end(), pair);

            if (duplicate) {
                map_it->second = new_list_it;
                data->list->erase(list_it);
            } else {
                data->map->insert({pair.first, new_list_it});
            }

            return !duplicate;
        } catch (...) {
            if (data != backup.first)
                restore_data(backup);
            else if (!data->list->empty() && data->list->back() == pair)
                data->list->pop_back();
            throw;
        }
    };

    void erase(K const &k) {
        backup_data_t backup = prepare_to_modify(false);

        typename map_t::iterator it = data->map->find(k);
        if (it == data->map->end())
            throw lookup_error();

        typename list_t::iterator list_it = it->second;
        data->map->erase(it);
        data->list->erase(list_it);
    };

    void merge(insertion_ordered_map const &other) {
        backup_data_t backup = prepare_to_modify(false);

        std::shared_ptr<data_t> data_cp = (data == backup.first) ? std::make_shared<data_t>(*data) : data;

        pair_t pair;
        bool duplicate;
        typename list_t::iterator list_it;

        for (auto other_list_it = other.begin(); other_list_it != other.end(); ++other_list_it) {
            auto map_it = data_cp->map->find(other_list_it->first);

            if (map_it != data_cp->map->end()) {
                duplicate = true;
                list_it = map_it->second;
                pair = *list_it;
            } else {
                duplicate = false;
                pair = std::pair(*other_list_it);
            }

            try {
                auto new_list_it = data_cp->list->insert(data_cp->list->end(), pair);
                data_cp->map->insert({pair.first, new_list_it});

                if (duplicate) {
                    map_it->second = new_list_it;
                    data_cp->list->erase(list_it);
                } else {
                    data_cp->map->insert({pair.first, new_list_it});
                }
            } catch (...){
                restore_data(backup);
                throw;
            }
        }

        data = data_cp;
    };

    V const &at(K const &k) const {
        typename map_t::iterator it = data->map->find(k);

        if (it == data->map->end())
            throw lookup_error();

        return it->second->second;
    };

    V &at(K const &k) {
        return const_cast<V &>(const_cast<const insertion_ordered_map *>(this)->at(k));
    };

    V &operator[](K const &k) {
        backup_data_t backup = prepare_to_modify(true);

        pair_t pair;
        typename map_t::iterator it = data->map->find(k);

        if (it != data->map->end())
            return it->second->second;

        pair = {k, V()};
        try {
            data->list->push_back(pair);
            typename list_t::iterator it_list = --data->list->end();
            data->map->insert({k, it_list});

            return it_list->second;
        } catch (...) {
            if (data != backup.first)
                restore_data(backup);
            else if (!data->list->empty() && data->list->back() == pair)
                data->list->pop_back();
            throw;
        }
    };

    [[nodiscard]] size_t size() const noexcept {
        return data->list->size();
    };

    [[nodiscard]] bool empty() const noexcept {
        return data->list->empty();
    };

    void clear() {
        prepare_to_modify(false); //TODO
        data->list->clear();
        data->map->clear();
    };

    [[nodiscard]] bool contains(K const &k) noexcept {
        return (data->map->find(k) != data->map->end());
    };

    iterator begin() const noexcept {
        return iterator(data->list->begin());
    }

    iterator end() const noexcept {
        return iterator(data->list->end());
    }

};

#endif
