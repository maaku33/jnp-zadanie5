#ifndef JNP5_KEYED_QUEUE_H
#define JNP5_KEYED_QUEUE_H


#include <iostream>
#include <map>
#include <list>
#include <memory>
#include <exception>
#include <boost/operators.hpp>


class lookup_error : public std::exception {
    const char * what () const noexcept {
        return "lookup_error";
    }
};


template <class K, class V> class keyed_queue {

    using key_value_pair = std::pair<K, V>;
    using queue_type = std::list<key_value_pair>;
    using iterators_list_type = std::list<typename queue_type::iterator>;
    using map_type = std::map<K, iterators_list_type>;

    queue_type key_queue;
    map_type iterators_map;
    size_t queue_size;
    //TODO uwzględnić zmiany zmiane shareable w kontekście copy-on-write
    bool shareable;

public:

    keyed_queue() : queue_size(0), shareable(true) {};

    //TODO czy te iteratory z mapy się dobrze przepisują?
    keyed_queue(keyed_queue const &other) :
        key_queue(other.key_queue),
        iterators_map(other.iterators_map),
        queue_size(other.queue_size),
        shareable(other.shareable) {}

    keyed_queue(keyed_queue &&other) noexcept :
        key_queue(std::move(other.key_queue)),
        iterators_map(std::move(other.iterators_map)),
        queue_size(std::move(other.queue_size)),
        shareable(std::move(other.shareable)) {}

    keyed_queue &operator=(keyed_queue other) {
        key_queue = other.key_queue;
        iterators_map = other.iterators_map;
        queue_size = other.queue_size;
        shareable = other.shareable;

        return *this;
    }

    void push(K const &key, V const &value) {
        key_queue.push_back(std::make_pair(key, value));
        auto it = iterators_map.find(key);

        if (it == iterators_map.end()) {
            iterators_list_type _list;
            _list.push_back(key_queue.end()-1);
            iterators_map.insert(std::make_pair(key, _list));
        } else {
            (it->second).push_back(key_queue.end()-1);
        }

        queue_size++;
    }
//TODO usunąć całą listę po usunięciu ostatniego jej elementu (we wszystkich popach)
//TODO Czy pisanie auto jest ładne?
    void pop() {
        if (empty()) throw lookup_error();

        auto it_queue = key_queue.begin();
        K key = it_queue->first;
        auto it_map = iterators_map.find(key);
        (it_map->second).pop_front();
        key_queue.pop_front();

        queue_size--;
    }

    void pop(K const &key) {
        auto it_map = iterators_map.find(key);
        if (it == iterators_map.end()) throw lookup_error();

        key_queue.erase((it_map->second).front());
        (it_map->second).pop_front();

        queue_size--;
    }

    void move_to_back(K const &key) {
        auto it_map = iterators_map.find(key);
        if (it_map == iterators_map.end()) throw lookup_error();

        for (auto it_iter = (it_map->second).begin(); it_iter != (it_map->second).end(); ++it_iter) {
            key_value_pair moved_element(**it_iter);
            key_queue.erase(*it_iter);
            key_queue.push_back(moved_element);
        }
    }

    std::pair<K const &, V &> front() {
        if (empty()) throw lookup_error();

        return key_queue.front();
    }

    std::pair<K const &, V &> back() {
        if (empty()) throw lookup_error();

        return key_queue.back();
    }

    std::pair<K const &, V const &> front() const {
        if (empty()) throw lookup_error();

        return key_queue.front();
    }

    std::pair<K const &, V const &> back() const {
        if (empty()) throw lookup_error();

        return key_queue.back();
    }

    std::pair<K const &, V &> first(K const &key) {
        auto it = iterators_map.find(key);
        if (it == iterators_map.end()) throw lookup_error();

        return *(it->second.front());
    }

    std::pair<K const &, V &> last(K const &key) {
        auto it = iterators_map.find(key);
        if (it == iterators_map.end()) throw lookup_error();

        return *(it->second.back());
    }

    std::pair<K const &, V const &> first(K const &key) const {
        auto it = iterators_mapp.find(key);
        if (it == iterators_map.end()) throw lookup_error();

        return *(it->second.front());
    }

    std::pair<K const &, V const &> last(K const &key) const {
        auto it = iterators_map.find(key);
        if (it == iterators_map.end()) throw lookup_error();

        return *(it->second.back());
    }

    size_t size() const {
        return queue_size;
    }

    bool empty() const {
        return queue_size == 0;
    }

    void clear() {
        key_queue.clear();
        iterators_map.clear();
        queue_size = 0;
    }

    size_t count(K const &key) const {
        auto it = iterators_map.find(key);

        return (it->second).size();
    }


    class k_iterator : boost::input_iteratable<k_iterator, K*> {

        friend class keyed_queue;

        typename map_type::const_iterator map_it;

        k_iterator(typename map_type::const_iterator it) : map_it(it) {}

    public:

        k_iterator() : map_it() {}
        k_iterator(const k_iterator &iter) : map_it(iter.map_it) {}

        k_iterator operator++() { ++map_it; return *this; }

        bool operator==(const k_iterator &it) const { return map_it == it.map_it; }

        const K& operator*() const { return map_it->first; }

    };

    k_iterator k_begin() noexcept { return k_iterator(iterators_map.begin()); }
    k_iterator k_end() noexcept { return k_iterator(iterators_map.end()); }

};


#endif //JNP5_KEYED_QUEUE_H
