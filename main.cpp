#include "keyed_queue.h"
#include <cassert>
#include <vector>

auto f(keyed_queue<int, int> q)
{
    return q;
}

int main()
{
    int keys[] = {3, 1, 2};

    keyed_queue<int, int> kq1 = f({});

    for (int i = 0; i < 3; ++i) {
        kq1.push(keys[i], i);
    }
    auto &ref = kq1.front().second;

    keyed_queue<int, int> kq2(kq1); // Wykonuje się pełna kopia, dlaczego?
    keyed_queue<int, int> kq3;
    kq3 = kq2;

    ref = 10;
    assert(kq1.front().second == 10);
    assert(kq2.front().second != 10);

    kq2.pop(); // Obiekt kq2 dokonuje kopii i przestaje współdzielić dane z kq3.
    assert(kq2.size() == 2);
    assert(kq2.count(3) == 0);
    assert(kq2.count(2) == 1);

    assert(kq3.size() == 3);
    assert(kq3.count(3) == 1);

    kq2.push(1, 3);
    kq2.move_to_back(1);
    assert(kq2.size() == 3);
    assert(kq2.front().second == 2
           && kq2.first(1).second == 1
           && kq2.last(1).second == 3
           && kq2.back().second == 3);

    keyed_queue<int, int> const kq4 = kq2;
    assert(kq4.front().second == 2
           && kq4.first(1).second == 1
           && kq4.last(1).second == 3
           && kq4.back().second == 3);

    int i = 1;
    for (auto k_it = kq1.k_begin(), k_end = kq1.k_end();
         k_it != k_end;
         ++k_it, ++i) {
        assert(i <= 3 && *k_it == i);
    }

    auto kq5 = std::make_unique<keyed_queue<int, int>>();
    kq5->push(4, 0);
    assert(kq5->front().first == 4 && kq5->front().second == 0);
    auto kq6(*kq5);
    kq5.reset();
    assert(kq6.front().first == 4 && kq6.front().second == 0);

    std::swap(kq1, kq2);
    std::vector<keyed_queue<int, int>> vec;
    for (int i = 0; i < 100000; i++) {
        kq1.push(i, i);
    }
    for (int i = 0; i < 1000000; i++) {
        vec.push_back(kq1);  // Wszystkie obiekty w vec współdzielą dane.
    }

    return 0;
}
