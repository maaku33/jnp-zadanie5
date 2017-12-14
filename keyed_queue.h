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
    
    struct members_struct {
        queue_type key_queue;
        map_type iterators_map;

        // no-throw
        members_struct() = default;

        //no-throw
        members_struct(members_struct const &other) = default;
    };

    using members_ptr = std::shared_ptr<members_struct>;

    members_ptr members;
    //TODO uwzględnić zmiany zmiane modified w kontekście copy-on-write
    bool modified;

public:

    keyed_queue() : members(std::make_shared<members_struct>()), modified(false) {};

    // doesn't throw exception because make_shared throws only if the constructor of ot members throws
    // and the default constructor of member does not throw
    keyed_queue(keyed_queue const &other) : modified(false) {
        if (other.modified) {
            members = std::make_shared<members_struct>(*(other.members));
        } else {
            members = other.members;
        }
    }

    // default move constructor doesn't throw
    keyed_queue(keyed_queue &&other) noexcept = default;

    // doesn't throw exception because make_shared throws only if the constructor of ot members throws
    // and the default constructor of member does not throw
    keyed_queue &operator=(keyed_queue other) {
        if (other.modified) {
            members = std::make_shared<members_struct>(*(other.members));
        } else {
            members = other.members;
        }

        modified = false;

        return *this;
    }

    members_ptr copy_members() {
        members_ptr new_members = std::make_shared<members_struct>();

        auto &queue = new_members->key_queue;
        auto &map = new_members->iterators_map;

        for (auto &p : members->key_queue) {
            queue.push_back(std::make_pair(p.first, p.second));
            map[p.first].push_back(--(queue.end()));
        }

        return new_members;
    }

    // copy-and-swap idiom gives a strong exception guarantee
    void check_copy_members() {
        if (members.use_count() > 1) {
            members = copy_members();
        }
    }

    void push(K const &key, V const &value) {
        check_copy_members();

        // strong exception guarantee is given by the function list::push_back
        (members->key_queue).push_back(key_value_pair(key, value));

        try {
            auto it = (members->iterators_map).find(key);

            if (it == (members->iterators_map).end()) {
                iterators_list_type iter_list;
                iter_list.push_back(--((members->key_queue).end()));
                (members->iterators_map).insert(std::make_pair(key, iter_list));
            } else {
                (it->second).push_back(--((members->key_queue).end()));
            }
        } catch (...) {
            // rollback - erasing the element from key_queue which was inserted before the try-block;
            // changing the content of iterators_map is the last instruction in the try-block
            // and functions map::insert (in case of inserting exactly one element) and list::push_back
            // give strong exception guarantee;
            // list::pop_back has no-throw guarantee
            (members->key_queue).pop_back();

            throw;
        }
    }

    void pop() {
        if (empty()) throw lookup_error();

        members_ptr old_members = members;
        members_ptr new_members = copy_members();

        check_copy_members();

        auto it_queue = (members->key_queue).begin();
        K key = it_queue->first;
        auto it_map = (members->iterators_map).find(key);





//        // list::pop_front, list::empty give no-throw guarantee
//        auto old_front = (it_map->second).front();
//        (it_map->second).pop_front();
//
//        try {
//            // strong exception guarantee of map::erase when removing exactly one element
//            if ((it_map->second).empty()) (members->iterators_map).erase(key);
//        } catch (...) {
//
//            throw;
//        }
//        (members->key_queue).pop_front();
    }

    // if an exception is thrown in the function find, there are no changes in the container
    void pop(K const &key) {
        check_copy_members();

        auto it = (members->iterators_map).find(key);
        if (it == (members->iterators_map).end()) throw lookup_error();

        (members->key_queue).erase((it->second).front());
        (it->second).pop_front();
        if ((it->second).empty()) (members->iterators_map).erase(key);
    }

    // functions size() and splice() don't throw exceptions
    // if an exception is thrown in the function find, there are no changes in the container
    void move_to_back(K const &key) {
        check_copy_members();

        auto it = (members->iterators_map).find(key);
        if (it == (members->iterators_map).end()) throw lookup_error();

        size_t number_of_elems = (it->second).size();

        auto &queue = members->key_queue;
        for (size_t i = 0; i < number_of_elems; ++i) {
            queue.splice(queue.end(), queue, (it->second).front());
            (it->second).splice((it->second).end(), it->second, (it->second).begin());
        }
    }

    // if the list is not empty, the function front never throws exceptions
    std::pair<K const &, V &> front() {
        if (empty()) throw lookup_error();

        check_copy_members();
        modified = true;

        auto &temp = (members->key_queue).front();
        return {temp.first, temp.second};
    }

    std::pair<K const &, V &> back() {
        if (empty()) throw lookup_error();

        check_copy_members();
        modified = true;

        auto &temp = (members->key_queue).back();
        return {temp.first, temp.second};
    }

    std::pair<K const &, V const &> front() const {
        if (empty()) throw lookup_error();

        auto &temp = (members->key_queue).front();
        return {temp.first, temp.second};
    }

    std::pair<K const &, V const &> back() const {
        if (empty()) throw lookup_error();

        auto &temp = (members->key_queue).back();
        return {temp.first, temp.second};
    }

    // if an exception is thrown by find, copy-on-write changes are rolled back
    std::pair<K const &, V &> first(K const &key) {
        auto it = (members->iterators_map).find(key);
        if (it == (members->iterators_map).end()) throw lookup_error();

        members_ptr old_members;
        check_copy_members();

        try {
            auto it = (members->iterators_map).find(key);

            modified = true;

            auto &temp = *((it->second).front());
            return {temp.first, temp.second};
        } catch (...) {
            members = old_members;
            throw;
        }
    }

    // if an exception is thrown by find, copy-on-write changes are rolled back
    std::pair<K const &, V &> last(K const &key) {
        auto it = (members->iterators_map).find(key);
        if (it == (members->iterators_map).end()) throw lookup_error();

        members_ptr old_members;
        check_copy_members();

        try {
            auto it = (members->iterators_map).find(key);

            modified = true;

            auto &temp = *((it->second).back());
            return {temp.first, temp.second};
        } catch (...) {
            members = old_members;
            throw;
        }
    }

    // if an exception is thrown by find, there are no changes in the container
    std::pair<K const &, V const &> first(K const &key) const {
        auto it = (members->iterators_map).find(key);
        if (it == (members->iterators_map).end()) throw lookup_error();

        auto &temp = *((it->second).front());
        return {temp.first, temp.second};
    }

    // if an exception is thrown by find, there are no changes in the container
    std::pair<K const &, V const &> last(K const &key) const {
        auto it = (members->iterators_map).find(key);
        if (it == (members->iterators_map).end()) throw lookup_error();

        auto &temp = *((it->second).back());
        return {temp.first, temp.second};
    }

    // function list::size() doesn't throw
    size_t size() const noexcept {
        return (members->key_queue).size();
    }

    // function keyed_queue::size() doesn't throw
    bool empty() const noexcept {
        return size() == 0;
    }

    // if make_shared throws an exception, there are no changes in the container
    void clear() {
        members = std::make_shared<members_struct>();
        modified = false;
    }

    // if an exception is thrown by find, there are no changes in the container
    size_t count(K const &key) const {
        auto it = (members->iterators_map).find(key);
        if (it == (members->iterators_map).end()) return 0;

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
        return k_iterator((members->iterators_map).begin());
    }

    k_iterator k_end() noexcept {
        return k_iterator((members->iterators_map).end());
    }

};


#endif //JNP5_KEYED_QUEUE_H
