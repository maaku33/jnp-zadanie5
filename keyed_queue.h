#ifndef JNP5_KEYED_QUEUE_H
#define JNP5_KEYED_QUEUE_H

#include <iostream>
#include <map>
#include <deque>
#include <memory>
#include <exception>
#include <boost/operators.hpp>

class lookup_error : public std::exception {
    const char * what () const noexcept {
        return "lookup_error";
    }
};

template <class K, class V> class keyed_queue {
    //TODO zastanowić się, czy na pewno wszędzie powinny być shared pointery
    class wrapping {
    public:
        K key;
        V value;
        std::shared_ptr<wrapping> previous;
        std::shared_ptr<wrapping> next;
        std::shared_ptr<std::deque<wrapping>> myqueue;

        //TODO jakieś consty, referencje czy coś takiego?
        wrapping(K _key, V _value, std::shared_ptr<wrapping> _last, std::shared_ptr<std::deque<wrapping>> _queue) :
                key(_key), value(_value), previous(_last), next(nullptr), myqueue(_queue) {
            previous->next = this;
        }

        void destroy() {
            if (next != nullptr) previous->next = next;
            if (previous != nullptr) next->previous = previous;
            myqueue->erase(this); //TODO czy to zadziała?
        }
    };

    typedef std::map<K, std::deque<wrapping>> map_type;

    map_type mymap;
    size_t queue_size;
    std::shared_ptr<wrapping> first_wrapping;
    std::shared_ptr<wrapping> last_wrapping;

public:
    //TODO gdzieś dopisać explicit albo noexcept? + pamiętać o wyjątkach
    //TODO kopiowanie/tworzenie nowych first/last_wrappingów jest zrobione bez sensu
    keyed_queue() : queue_size(0), first_wrapping(nullptr), last_wrapping(nullptr) {}

    keyed_queue(keyed_queue const &other) :
        mymap(std::move(other.mymap)),
        queue_size(other.queue_size),
        first_wrapping(other.first_wrapping),
        last_wrapping(other.last_wrapping) {}

    keyed_queue(keyed_queue &&other) noexcept :
        mymap(std::move(other.mymap)),
        queue_size(std::move(other.queue_size)),
        first_wrapping(std::move(other.first_wrapping)),
        last_wrapping(std::move(other.last_wrapping)) {}

    keyed_queue &operator=(keyed_queue other) {
        mymap = other.mymap;
        queue_size = other.queue_size;
        first_wrapping = other.first_wrapping;
        last_wrapping = other.last_wrapping;

        return *this;
    }

    void push(K const &key, V const &value) {
        auto it = mymap.find(key);

        if (it == mymap.end()) {
            auto new_deque_ptr = std::make_shared<std::deque<wrapping>>();
            wrapping new_wrapping(key, value, last_wrapping, new_deque_ptr);
            (*new_deque_ptr).push_back(new_wrapping);
            std::deque<wrapping> new_deque = *new_deque_ptr;
            mymap.insert(std::make_pair(key, new_deque));
        } else {
            auto deque_ptr = std::make_shared<std::deque<wrapping>>(it->second);
            wrapping new_wrapping(key, value, last_wrapping, deque_ptr);
            (it->second).push_back(new_wrapping);
        }

        queue_size++;
    }

    void pop() {
        if (empty()) throw lookup_error();

        auto temp_it = first_wrapping;
        first_wrapping = first_wrapping->next;
        temp_it->destroy(); //TODO problem z zachowaniem poprawnej kolejności w kolejce odpowiadającego kluczowi elementu *temp_it

        queue_size--;
    }

    void pop(K const &key) {
        auto it = mymap.find(key);
        if (it == mymap.end()) throw lookup_error();

        wrapping first_key_wrapping = (it->second).front();
        (it->second).pop_front();
        first_key_wrapping.destroy();

        queue_size--;
    }

    void move_to_back(K const &key) {
        auto it = mymap.find(key);
        if (it == mymap.end()) throw lookup_error();

        for (auto key_it = (it->second).begin(); key_it != (it->second).end(); ++it) {
            // TODO uwaga - trochę powtórzenie kodu z funkcji destroy klasy wrapping
            if (key_it->next != nullptr) key_it->previous->next = key_it->next;
            if (key_it->previous != nullptr) key_it->next->previous = key_it->previous;

            key_it->previous = last_wrapping;
            key_it->next = nullptr;
            last_wrapping->next = key_it; //TODO przypisanie iteratora na shared_ptr nie działa
        }
    }

    std::pair<K const &, V &> front() {
        if (empty()) throw lookup_error();

        return std::make_pair(first_wrapping->key, first_wrapping->value);
    }

    std::pair<K const &, V &> back() {
        if (empty()) throw lookup_error();

        return std::make_pair(last_wrapping->key, last_wrapping->value);
    }

    std::pair<K const &, V const &> front() const {
        if (empty()) throw lookup_error();

        return std::make_pair(first_wrapping->key, first_wrapping->value);
    }

    std::pair<K const &, V const &> back() const {
        if (empty()) throw lookup_error();

        return std::make_pair(last_wrapping->key, last_wrapping->value);
    }

    std::pair<K const &, V &> first(K const &key) {
        auto it = mymap.find(key);
        if (it == mymap.end()) throw lookup_error();

        return std::make_pair((it->second).front().key, (it->second).front().value);
    }

    std::pair<K const &, V &> last(K const &key) {
        auto it = mymap.find(key);
        if (it == mymap.end()) throw lookup_error();

        return std::make_pair((it->second).back().key, (it->second).back().value);
    }

    std::pair<K const &, V const &> first(K const &key) const {
        auto it = mymap.find(key);
        if (it == mymap.end()) throw lookup_error();

        return std::make_pair((it->second).front().key, (it->second).front().value);
    }

    std::pair<K const &, V const &> last(K const &key) const {
        auto it = mymap.find(key);
        if (it == mymap.end()) throw lookup_error();

        return std::make_pair((it->second).back().key, (it->second).back().value);
    }

    size_t size() const {
        return queue_size;
    }

    bool empty() const {
        return queue_size == 0;
    }

    void clear() {
        mymap.clear();
        queue_size = 0;
        first_wrapping = nullptr;
        last_wrapping = nullptr;
    }

    size_t count(K const &key) const {
        auto it = mymap.find(key);

        return (it->second).size();
    }

    /*
- Iterator k_iterator oraz metody k_begin i k_end, pozwalające przeglądać
  zbiór kluczy w rosnącej kolejności wartości. Iteratory mogą być unieważnione
  przez dowolną operację modyfikacji kolejki zakończoną powodzeniem
  (w tym front, back, first i last w wersjach bez const).
  Iterator powinien udostępniać przynajmniej następujące operacje:
  - konstruktor bezparametrowy i kopiujący
  - operator++ prefiksowy
  - operator== i operator!=
  - operator*
  Do implementacji można użyć biblioteki Boost.Operators.
  Wszelkie operacje w czasie O(log n). Przejrzenie całej kolejki w czasie O(n).

Iteratory służą jedynie do przeglądania kolejki i za ich pomocą nie można
modyfikować listy, więc zachowują się jak const_iterator z STL.
     */
    class k_iterator {
        typename map_type::iterator map_it;

        k_iterator(typename map_type::iterator it) : map_it(it) {}

    public:
        k_iterator() : map_it() {}
        k_iterator(const k_iterator &iter) : map_it(iter.map_it) {}

        k_iterator operator++() { ++map_it; return *this; }
        k_iterator operator++(int) { k_iterator old(*this); ++map_it; return old; }

        bool operator==(const k_iterator &rhs) { return map_it == rhs.map_it; }
        bool operator!=(const k_iterator &rhs) { return map_it != rhs.map_it; }

        K operator*() { return map_it->first; }
    };

    k_iterator k_begin() noexcept { return k_iterator(mymap.begin()); }
    k_iterator k_end() noexcept { return k_iterator(mymap.end()); }

};


#endif //JNP5_KEYED_QUEUE_H
