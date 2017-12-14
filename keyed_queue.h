#ifndef JNP5_KEYED_QUEUE_H
#define JNP5_KEYED_QUEUE_H


#include <map>
#include <list>
#include <memory>
#include <exception>
#include <boost/operators.hpp>


class lookup_error : public std::exception {

public:
    const char * what () const noexcept {
        return "lookup_error";
    }

};


template <class K, class V> class keyed_queue {

    using key_value_pair = std::pair<K, V>;
    using queue_type = std::list<key_value_pair>;
    using iterators_list_type = std::list<typename queue_type::iterator>;
    using map_type = std::map<K, iterators_list_type>;

    struct members {
        queue_type key_queue;
        map_type iterators_map;

        // no-throw
        members() = default;

        //no-throw
        members(members const &other) = default;
    };

    std::shared_ptr<members> members_ptr;
    //TODO uwzględnić zmiany zmiane modified w kontekście copy-on-write
    bool modified;

public:

    keyed_queue() : members_ptr(std::make_shared<members>()), modified(false) {};

    // doesn't throw exception because make_shared throws only if the constructor of ot members throws
    // and the default constructor of member does not throw
    keyed_queue(keyed_queue const &other) : modified(false) {
        if (other.modified) {
            members_ptr = std::make_shared<members>(*(other.members_ptr));
        } else {
            members_ptr = other.members_ptr;
        }
    }

    // default move constructor doesn't throw
    keyed_queue(keyed_queue &&other) noexcept = default;

    // doesn't throw exception because make_shared throws only if the constructor of ot members throws
    // and the default constructor of member does not throw
    keyed_queue &operator=(keyed_queue other) {
        if (other.modified) {
            members_ptr = std::make_shared<members>(*(other.members_ptr));
        } else {
            members_ptr = other.members_ptr;
        }

        modified = false;

        return *this;
    }

    void write_imminent() {
        if (members_ptr.use_count() > 1) {
            members_ptr = std::make_shared<members>(*members_ptr);
        }
    }

    // if an exception is thrown in the function find, there are no changes in the container
    void push(K const &key, V const &value) {
        write_imminent();

        (members_ptr->key_queue).push_back(key_value_pair(key, value));
        auto it = (members_ptr->iterators_map).find(key);

        if (it == (members_ptr->iterators_map).end()) {
            iterators_list_type iter_list;
            iter_list.push_back(--((members_ptr->key_queue).end()));
            (members_ptr->iterators_map).insert(std::make_pair(key, iter_list));
        } else {
            (it->second).push_back(--((members_ptr->key_queue).end()));
        }
    }

//TODO powtórzenie kodu? wszędzie jest podobne throwowanie i podobne są obie funkcje pop

    // if an exception is thrown in the function find, there are no changes in the container
    void pop() {
        if (empty()) throw lookup_error();

        write_imminent();

        auto it_queue = (members_ptr->key_queue).begin();
        K key = it_queue->first;
        auto it_map = (members_ptr->iterators_map).find(key);

        (it_map->second).pop_front();
        if ((it_map->second).empty()) (members_ptr->iterators_map).erase(key);
        (members_ptr->key_queue).pop_front();
    }

    // if an exception is thrown in the function find, there are no changes in the container
    void pop(K const &key) {
        write_imminent();

        auto it = (members_ptr->iterators_map).find(key);
        if (it == (members_ptr->iterators_map).end()) throw lookup_error();

        (members_ptr->key_queue).erase((it->second).front());
        (it->second).pop_front();
        if ((it->second).empty()) (members_ptr->iterators_map).erase(key);
    }

    // functions size() and splice() don't throw exceptions
    // if an exception is thrown in the function find, there are no changes in the container
    void move_to_back(K const &key) {
        write_imminent();

        auto it = (members_ptr->iterators_map).find(key);
        if (it == (members_ptr->iterators_map).end()) throw lookup_error();

        size_t number_of_elems = (it->second).size();

        auto &queue = members_ptr->key_queue;
        for (size_t i = 0; i < number_of_elems; ++i) {
            queue.splice(queue.end(), queue, (it->second).front());
            (it->second).splice((it->second).end(), it->second, (it->second).begin());
        }
    }

    // if the list is not empty, the function front never throws exceptions
    std::pair<K const &, V &> front() {
        if (empty()) throw lookup_error();

        write_imminent();
        modified = true;

        auto &temp = (members_ptr->key_queue).front();
        return {temp.first, temp.second};
    }

    std::pair<K const &, V &> back() {
        if (empty()) throw lookup_error();

        write_imminent();
        modified = true;

        auto &temp = (members_ptr->key_queue).back();
        return {temp.first, temp.second};
    }

    std::pair<K const &, V const &> front() const {
        if (empty()) throw lookup_error();

        auto &temp = (members_ptr->key_queue).front();
        return {temp.first, temp.second};
    }

    std::pair<K const &, V const &> back() const {
        if (empty()) throw lookup_error();

        auto &temp = (members_ptr->key_queue).back();
        return {temp.first, temp.second};
    }

    // if an exception is thrown in the function find, there are no changes in the container
    std::pair<K const &, V &> first(K const &key) {
        auto it = (members_ptr->iterators_map).find(key);
        if (it == (members_ptr->iterators_map).end()) throw lookup_error();

        write_imminent();
        modified = true;

        auto &temp = *((it->second).front());
        return {temp.first, temp.second};
    }

    // if an exception is thrown in the function find, there are no changes in the container
    std::pair<K const &, V &> last(K const &key) {
        auto it = (members_ptr->iterators_map).find(key);
        if (it == (members_ptr->iterators_map).end()) throw lookup_error();

        write_imminent();
        modified = true;

        auto &temp = *((it->second).back());
        return {temp.first, temp.second};
    }

    // if an exception is thrown in the function find, there are no changes in the container
    std::pair<K const &, V const &> first(K const &key) const {
        auto it = (members_ptr->iterators_map).find(key);
        if (it == (members_ptr->iterators_map).end()) throw lookup_error();

        auto &temp = *((it->second).front());
        return {temp.first, temp.second};
    }

    // if an exception is thrown in the function find, there are no changes in the container
    std::pair<K const &, V const &> last(K const &key) const {
        auto it = (members_ptr->iterators_map).find(key);
        if (it == (members_ptr->iterators_map).end()) throw lookup_error();

        auto &temp = *((it->second).back());
        return {temp.first, temp.second};
    }

    // function list::size() doesn't throw
    size_t size() const {
        return (members_ptr->key_queue).size();
    }

    // function keyed_queue::size() doesn't throw
    bool empty() const {
        return size() == 0;
    }

    // function reset has the noexcept specifier
    void clear() {
        write_imminent();

        members_ptr.reset();
        modified = false;
    }

    // if an exception is thrown in the function find, there are no changes in the container
    size_t count(K const &key) const {
        auto it = (members_ptr->iterators_map).find(key);
        if (it == (members_ptr->iterators_map).end()) return 0;

        return (it->second).size();
    }


    class k_iterator : boost::input_iteratable<k_iterator, K*> {

        using map_iterator = typename map_type::const_iterator;

        friend class keyed_queue;
        map_iterator map_it;

        explicit k_iterator(const map_iterator& it) : map_it(it) {}

    public:

        k_iterator() = default;

        k_iterator(const k_iterator &iter) : map_it(iter.map_it) {}

        k_iterator operator++() {
            ++map_it;

            return *this;
        }

        bool operator==(const k_iterator &it) const {
            return map_it == it.map_it;
        }

        const K& operator*() const {
            return map_it->first;
        }

    };

    k_iterator k_begin() noexcept {
        return k_iterator((members_ptr->iterators_map).begin());
    }

    k_iterator k_end() noexcept {
        return k_iterator((members_ptr->iterators_map).end());
    }

};


#endif //JNP5_KEYED_QUEUE_H
