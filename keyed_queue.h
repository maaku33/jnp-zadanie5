#ifndef JNP5_KEYED_QUEUE_H
#define JNP5_KEYED_QUEUE_H

#include <iostream>

template <class K, class V> class keyed_queue {
public:
    keyed_queue();

    keyed_queue(keyed_queue const &);

    keyed_queue(keyed_queue &&);

    void push(K const &k, V const &v);

    void pop();

    void pop(K const &);

    void move_to_back(K const &k);

    std::pair<K const &, V &> front();
    std::pair<K const &, V &> back();
    std::pair<K const &, V const &> front() const;
    std::pair<K const &, V const &> back() const;

    std::pair<K const &, V &> first(K const &key);
    std::pair<K const &, V &> last(K const &key);
    std::pair<K const &, V const &> first(K const &key) const;
    std::pair<K const &, V const &> last(K const &key) const;

    size_t size() const;

    bool empty() const;

    void clear();

    size_t count(K const &) const;

    
};


#endif //JNP5_KEYED_QUEUE_H
