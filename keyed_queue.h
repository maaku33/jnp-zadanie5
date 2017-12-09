#ifndef JNP5_KEYED_QUEUE_H
#define JNP5_KEYED_QUEUE_H

#include <iostream>
#include <map>
#include <deque>
#include <memory>
#include <exception>


class lookup_error : public std::exception {
    const char * what () const throw () {
        return "lookup_error";
    }
};


template <class K, class V> class keyed_queue {
public:
    //TODO gdzieś dopisać explicit albo noexcept? + pamiętać o wyjątkach
    keyed_queue() : queue_size(0) {}

    keyed_queue(keyed_queue const &other);

    keyed_queue(keyed_queue &&other) :
        mymap(std::move(other.mymap)),
        queue_size(std::move(other.queue_size)),
        first_wrapping(std::move(other.first_wrapping)),
        last_wrapping(std::move(other.last_wrapping)) {}

    //TODO co jeśli pushujemy na pustą kolejkę?
    void push(K const &key, V const &value) {
        auto it = mymap.find(key);
        wrapping new_wrapping(key, value, last_wrapping, it->second);
        it->second.push_back(new_wrapping);
        queue_size++;
    }

    void pop() {
        if (empty()) throw lookup_error();

        //TODO czy ten kod poniżej się wykona w przypadku wyrzucenia wyjątku powyżej?
        auto temp_it = first_wrapping;
        first_wrapping = first_wrapping->next;
        temp_it->destroy();

        queue_size--;
    }

    void pop(K const &key) {
        auto it = mymap.find(key);
        if (key == mymap.end()) throw lookup_error();

        wrapping first_key_wrapping = it->second.front();
        it->second.pop_front();
        first_key_wrapping.destroy();

        queue_size--;
    }

    void move_to_back(K const &key) {
        auto it = mymap.find(key);
        if (key == mymap.end()) throw lookup_error();
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
        if (key == mymap.end()) throw lookup_error();

        return std::make_pair(it->second.front().key, it->second.front().value); // coś tu nie tak ze zwracaniem klucza
    }

    std::pair<K const &, V &> last(K const &key) {
        auto it = mymap.find(key);
        if (key == mymap.end()) throw lookup_error();
    }

    std::pair<K const &, V const &> first(K const &key) const {
        auto it = mymap.find(key);
        if (key == mymap.end()) throw lookup_error();
    }

    std::pair<K const &, V const &> last(K const &key) const {
        auto it = mymap.find(key);
        if (key == mymap.end()) throw lookup_error();
    }

    size_t size() const {
        return queue_size;
    }

    bool empty() const {
        return queue_size == 0;
    }

    void clear() {

    }

    size_t count(K const &key) const {
        auto it = mymap.find(key);

        return it->second.size();
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

private:
    //TODO zastanowić się, czy na pewno wszędzie powinny być shared pointery
    class wrapping {
    public:
        K key;
        V value;
        std::shared_ptr<wrapping> previous;
        std::shared_ptr<wrapping> next;
        std::shared_ptr<std::deque> myqueue;

        //TODO jakieś const referencje czy coś innego?
        wrapping(K _key, V _value, std::shared_ptr<wrapping> _last, std::shared_ptr<std::deque> _queue) :
                key(_key), value(_value), previous(_last), next(nullptr), myqueue(_queue) {
            previous->next = this;
        }

        void destroy() {
            if (next != nullptr) previous->next = next;
            if (previous != nullptr) next->previous = previous;
            myqueue->erase(this); //czy to zadziała?
        }
    };

    std::map<K, std::deque<wrapping>> mymap;

    size_t queue_size;

    std::shared_ptr<wrapping> first_wrapping;
    std::shared_ptr<wrapping> last_wrapping;
};


#endif //JNP5_KEYED_QUEUE_H
