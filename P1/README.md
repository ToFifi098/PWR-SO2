**[Powrót](../README.md)**

# Problem jedzących filozofów

## Uruchomienie projektu
  Program jako argument przyjmuje:
  
  `liczba filozofów` - musi być ona większa od 1

  Program nie ma określonego limitu wykonań, w celu zatrzymania należy użyć skrótu `CTRL + C`

## Opis problemu

Problem ucztujących filozofów jest prezentacją problemu synchronizacji pracujących współbieżnie procesów.

N filozofów siedzi przy stole i każdy wykonuje jedną z dwóch czynności – albo je, albo rozmyśla. Stół jest okrągły, przed każdym z nich znajduje się miska ze spaghetti, a pomiędzy każdą sąsiadującą parą filozofów leży widelec, a więc każda osoba ma przy sobie dwie sztuki – po swojej lewej i prawej stronie. Ponieważ jedzenie potrawy jest trudne przy użyciu jednego widelca, zakłada się, że każdy filozof korzysta z dwóch. Dodatkowo nie ma możliwości skorzystania z widelca, który nie znajduje się bezpośrednio przed daną osobą. [<sub>wikipedia</sub>](https://pl.wikipedia.org/wiki/Problem_ucztuj%C4%85cych_filozof%C3%B3w)

![dining_philosopher_problem](https://github.com/user-attachments/assets/fe9a5000-09c6-4541-9998-948e7b51f056)\
[<sub>geeksforgeeks</sub>](https://www.geeksforgeeks.org/dining-philosopher-problem-using-semaphores/)

### Wątki w programie

W programie elementem wykonywanym wspóbieżnie jest element jedzenia, podnoszenia widelców, przez każdy wątek, który reprezentuje jednego filozofa.

Do rozwiązania problemu synchronizacji został wykorzystany mechanizm Semaforów Liczących.

Każdy semafor posiada parametr `count` który odpowiada za ilość dostępnych zasobów danego typu. Wątek uzyskuje dostęp jeżeli `count` jest większy od 0. W przeciwnym wypadku czeka na jego zwolnienie.

### Sekcje krytyczne
#### Widelce
Każdy widelec może być podniesiony maksymalnie przez jednego filozofa jednocześnie. Ale do zjedzenia posiłku potrzebuje dwóch.

Poprzez blokowanie dostępu do widelców
```
forks[leftFork].wait();
forks[rightFork].wait();
```
zostaje zapewnione, że każdy widelec jest podniesiony tylko przez jednego filozofa.

Następnie po zjedzeniu posiłku zostaje on odblokowany.
```
forks[leftFork].signal();
forks[rightFork].signal();
```

Aby nie doszło do trwałego zablokowania programu, do stołu dopuszczane jest maksymalnie `N - 1` filozofów jednocześnie.
Gwarantuje to, że zawsze istnieją minimum dwa widelce do podniesienia przez któregoś z filozofów.

Przed podniesieniem widelców, filozof sprawdza czy przy stole jest dla niego miejsce,
```
diningPhilosophers.wait();
```
jeżeli nie ma to czega na jego zwolnienie, po czym po spożytym posiłku je zwalnia
```
diningPhilosophers.signal();
```

#### Strumień wyjściowy
W celu zapewnienia poprawnego wyświetlania postępu programu
```
Philosopher 0 is thinking.
Philosopher 3 is thinking.
Philosopher 2 is thinking.
Philosopher 4 is thinking.
```
dostęp do strumienia wyjściowego jest zabezpieczony semaforem
```
stdSemaphore.wait();
std::cout << "Philosopher " << philosopher << " is eating.\n";
stdSemaphore.signal();
```
gwarantuje to poprawną kolejność i brak nakładania się na siebie dwóch tekstów.

