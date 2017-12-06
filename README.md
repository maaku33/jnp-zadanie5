Zadanie kolejka z kluczami, JNPI C++ 2017/18
============================================

Celem tego zadania jest zaimplementowanie wzorca kontenera zachowującego
się jak kolejka FIFO, w której każdy element ma przyporządkowany klucz.
Kontener ten powinien zapewniać możliwie duże gwarancje odporności na wyjątki
oraz semantykę kopiowania przy zapisie (copy-on-write).

Kopiowanie przy zapisie to technika optymalizacji szeroko stosowana
m.in. w strukturach danych z biblioteki Qt oraz dawniej w implementacjach
std::string. Podstawowa jej idea jest taka, że gdy tworzymy kopię obiektu
(w C++ za pomocą konstruktora kopiującego lub operator przypisania), to
współdzieli ona wszystkie wewnętrzne zasoby (które mogą np. w rzeczywistości
istnieć w oddzielnym obiekcie na stercie) z obiektem źródłowym.
Taki stan rzeczy może trwać do momentu, w którym jedna z kopii musi zostać
zmodyfikowana – wtedy modyfikowany obiekt tworzy własną kopię owych zasobów,
na których wykonuje modyfikacje.

Wzorzec ma być sparametryzowany typami K i V. Można założyć, że typ klucza K
ma semantykę wartości, czyli w szczególności dostępny jest dla niego
bezparametrowy konstruktor domyślny, konstruktor kopiujący, przenoszący
i operatory przypisania. Od typu K można również wymagać, aby zdefiniowany był
na nim porządek liniowy i można na obiektach tego typu wykonywać wszelkie
porównania. O typie V można jedynie założyć, że ma konstruktor kopiujący.

W ramach tego zadania należy zaimplementować szablon

```
#!c++
template <class K, class V> class keyed_queue {
};
```

Klasa keyed_queue powinna udostępniać niżej opisane operacje. Przy każdej
operacji podana jest jej oczekiwana złożoność czasowa, gdzie n oznacza
liczbą elementów przechowywanych w kolejce. Złożoność czasowa operacji
kopiowania przy zapisie powinna wynosić O(n log n).
Wszystkie operacje muszą zapewniać co najmniej silną odporność na wyjątki,
a konstruktor przenoszący i destruktor muszą być no-throw.
Tam gdzie jest to możliwe i uzasadnione należy dodać kwalifikator noexcept.

- Konstruktor bezparametrowy tworzący pustą kolejkę. Czas O(1).
```
#!c++
keyed_queue();
```

- Konstruktor kopiujący.
  Powinien on mieć semantykę copy-on-write, a więc nie kopiuje wewnętrznych
  struktur danych, jeśli nie jest to potrzebne.
  Kolejki współdzielą struktury do czasu modyfikacji jednej z nich.
  Czas O(1) lub O(n log n), jeśli konieczne jest wykonanie kopii.
```
#!c++
keyed_queue(keyed_queue const &);
```

- Konstruktor przenoszący. Czas O(1).
```
#!c++
keyed_queue(keyed_queue &&);
```

- Operator przypisania. Operator powinien przyjmować argument przez wartość.
  Czas O(1).
```
#!c++
keyed_queue &operator=(keyed_queue other);
```

- Metoda push wstawia wartość v na koniec kolejki, nadając jej klucz k.
  Czas O(log n) + ew. czas kopiowania.
```
#!c++
void push(K const &k, V const &v);
```

- Metoda pop() usuwa pierwszy element z kolejki.
  Jeśli kolejka jest pusta, to podnosi wyjątek lookup_error.
  Czas O(log n) + ew. czas kopiowania.
```
#!c++
void pop();
```

- Metoda pop(k) usuwa pierwszy element o podanym kluczu z kolejki.
  Jeśli podanego klucza nie ma w kolejce, to podnosi wyjątek lookup_error.
  Czas O(log n) + ew. czas kopiowania.
```
#!c++
void pop(K const &);
```

- Metoda move_to_back przesuwa elementy o kluczu k na koniec kolejki,
  zachowując ich kolejność względem siebie. Zgłasza wyjątek lookup_error,
  gdy elementu o podanym kluczu nie ma na liście.
  Czas O(m + log n) + ew. czas kopiowania, gdzie m == count(k).
```
#!c++
void move_to_back(K const &k);
```

- Metody front i back zwracają parę klucz-wartość będącą odpowiednio
  na początku i końcu kolejki. Zwrócona para powinna umożliwić modyfikację
  wartości, ale nie klucza. Dowolna operacja potencjalnie modyfikująca
  kolejkę może unieważnić zwrócone referencje.
  Jeśli kolejka jest pusta, to podnosi wyjątek lookup_error.
  Czas O(1) + dla wersji bez const ew. czas kopiowania.
```
#!c++
std::pair<K const &, V &> front();
std::pair<K const &, V &> back();
std::pair<K const &, V const &> front() const;
std::pair<K const &, V const &> back() const;
```

- Metody first i last zwracają odpowiednio pierwszą i ostatnią parę
  klucz-wartość o danym kluczu, podobnie jak front i back.
  Jeśli podanego klucza nie ma w kolejce, to podnosi wyjątek lookup_error.
  Czas O(log n) + dla wersji bez const ew. czas kopiowania.
```
#!c++
std::pair<K const &, V &> first(K const &key);
std::pair<K const &, V &> last(K const &key);
std::pair<K const &, V const &> first(K const &key) const;
std::pair<K const &, V const &> last(K const &key) const;
```

- Metoda size zwraca liczbę elementów w kolejce. Czas O(1).
```
#!c++
size_t size() const;
```

- Metoda empty zwraca true, gdy kolejka jest pusta, a false w przeciwnym
  przypadku.
  Czas O(1).
```
#!c++
bool empty() const;
```

- Metoda clear usuwa wszystkie elementy z kolejki.
  Czas O(n).
```
#!c++
void clear();
```

- Metoda count zwraca liczbę elementów o podanym kluczu.
  Czas O(log n).
```
#!c++
size_t count(K const &) const;
```

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

Klasa lookup_error powinna być zdefiniowana na zewnątrz klasy keyed_queue
i powinna dziedziczyć po std::exception.

Klasa keyed_queue powinna być przezroczysta na wyjątki, czyli powinna
przepuszczać wszelkie wyjątki zgłaszane przez wywoływane przez nią funkcje
i przez operacje na jej składowych, a obserwowalny stan obiektów
nie powinien się zmienić. W szczególności operacje modyfikacji zakończone
niepowodzeniem nie powinny unieważniać iteratorów.

Przykład użycia
===============

```
#!c++
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
```

Rozwiązanie
===========

Rozwiązanie powinno być zawarte w pliku keyed_queue.h, który należy umieścić
w repozytorium w katalogu

`grupaN/zadanie5/ab123456+cd123456`

lub

`grupaN/zadanie5/ab123456+cd123456+ef123456`

gdzie N jest numerem grupy, a `ab123456`, `cd123456`, `ef123456` są identyfikatorami
członków zespołu umieszczającego to rozwiązanie.
Katalog z rozwiązaniem nie powinien zawierać innych plików, ale może zawierać
podkatalog private, gdzie można umieszczać różne pliki, np. swoje testy. Pliki
umieszczone w tym podkatalogu nie będą oceniane.