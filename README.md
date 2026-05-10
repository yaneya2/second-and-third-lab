# Лабораторные работы по структурам данных

Данный проект содержит реализации структур данных на C++ с использованием CMake для сборки.

## Файловая структура

```
├── CMakeLists.txt              # Конфигурация сборки проекта
├── main.cpp                    # Главный файл программы
├── classes/                    # Исходный код библиотек структур данных
│   ├── cpp/                    # Реализации классов (.cpp файлы)
│   │   ├── BItSequence.cpp
│   │   ├── UI.cpp
│   │   └── UIDeque.cpp         [Лабораторная 3]
│   └── headers/                # Заголовочные файлы (.h)
│       ├── ArraySequence.h
│       ├── BItSequence.h
│       ├── DoublyLinkedList.h  [Лабораторная 3]
│       ├── DynamicArray.h
│       ├── IEnumerator.h
│       ├── ImmutableArraySequence.h
│       ├── ImmutableListSequence.h
│       ├── LinkedList.h
│       ├── ListSequence.h
│       ├── MutableArraySequence.h
│       ├── MutableListSequence.h
│       ├── Option.h
│       ├── SegmentedDeque.h    [Лабораторная 3]
│       ├── Sequence.h
│       ├── Student.h           [Лабораторная 3]
│       ├── Teacher.h           [Лабораторная 3]
│       ├── UI.h
│       └── UIDeque.h           [Лабораторная 3]
└── tests/                      # Тесты для структур данных
    ├── testsDynamicArray.cpp
    ├── testsLinkedList.cpp
    ├── testsSegmentedDeque.cpp [Лабораторная 3]
    └── testsSequence.cpp
```

---

## Лабораторная работа №2

Файлы, относящиеся ко второй лабораторной работе:

### Основные файлы
- `CMakeLists.txt`
- `main.cpp`

### Классы и структуры данных (classes/)
**Заголовочные файлы (classes/headers/):**
- `ArraySequence.h`
- `BItSequence.h`
- `DynamicArray.h`
- `IEnumerator.h`
- `ImmutableArraySequence.h`
- `ImmutableListSequence.h`
- `LinkedList.h`
- `ListSequence.h`
- `MutableArraySequence.h`
- `MutableListSequence.h`
- `Option.h`
- `Sequence.h`
- `UI.h`

**Файлы реализации (classes/cpp/):**
- `BItSequence.cpp`
- `UI.cpp`

### Тесты (tests/)
- `testsDynamicArray.cpp`
- `testsLinkedList.cpp`
- `testsSequence.cpp`

---

## Лабораторная работа №3

Файлы, относящиеся к третьей лабораторной работе:

### Классы и структуры данных (classes/)
**Заголовочные файлы (classes/headers/):**
- `DoublyLinkedList.h` — реализация двунаправленного связного списка
- `SegmentedDeque.h` — сегментированный дек
- `Student.h` — модель студента
- `Teacher.h` — модель преподавателя
- `UIDeque.h` — интерфейс дека

**Файлы реализации (classes/cpp/):**
- `UIDeque.cpp` — реализация UIDeque

### Тесты (tests/)
- `testsSegmentedDeque.cpp` — тесты для SegmentedDeque

---

## Описание структур данных

### Последовательности (Sequence)
- `ArraySequence` — последовательность на основе массива
- `ListSequence` — последовательность на основе связного списка
- `ImmutableArraySequence` — неизменяемая последовательность на массиве
- `ImmutableListSequence` — неизменяемая последовательность на списке
- `MutableArraySequence` — изменяемая последовательность на массиве
- `MutableListSequence` — изменяемая последовательность на списке

### Массивы
- `DynamicArray` — динамический массив с автоматическим изменением размера

### Связные списки
- `LinkedList` — однонаправленный связный список
- `DoublyLinkedList` — двунаправленный связный список (Лабораторная 3)

### Деки
- `SegmentedDeque` — сегментированный дек для эффективной работы с данными (Лабораторная 3)
- `UIDeque` — универсальный интерфейс дека (Лабораторная 3)

### Утилиты
- `Option` — контейнер для опциональных значений
- `IEnumerator` — интерфейс для перечисления элементов
- `BItSequence` — битовая последовательность

### Модели
- `Student` — класс студента (Лабораторная 3)
- `Teacher` — класс преподавателя (Лабораторная 3)
