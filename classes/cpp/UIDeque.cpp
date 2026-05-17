#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include "../headers/SegmentedDeque.h"
#include "../headers/SegmentedDequeIO.h"
#include "../headers/Sequence.h"
#include "../headers/UIDeque.h"

using T = int;
using DequeType = SegmentedDeque<T>;
using SeqType = Sequence<T>;

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void safeReplace(DequeType *&current, SeqType *result) {
    if (!result) return;
    if (result != current) {
        delete current;
        current = static_cast<DequeType *>(result);
    }
}

static SeqType *createTempSequence() {
    std::cout << "Введите количество элементов: ";
    size_t n = 0;
    if (!(std::cin >> n) || n == 0) {
        clearInput();
        return nullptr;
    }
    auto *temp = new DequeType(4);
    std::cout << "Введите элементы: ";
    for (size_t i = 0; i < n; ++i) {
        T val;
        if (!(std::cin >> val)) {
            clearInput();
            delete temp;
            return nullptr;
        }
        temp->AppendImpl(val);
    }
    return temp;
}

static void printMenu() {
    std::cout << "\n========== SegmentedDeque UI ==========\n"
            << "0.  Выход из программы\n"
            << "1.  Создать новый Deque\n"
            << "2.  Отобразить текущий Deque\n"
            << "3.  Показать информацию (длина, пусто)\n"
            << "4.  Append\n"
            << "5.  Prepend\n"
            << "6.  InsertAt\n"
            << "7.  DeleteAt\n"
            << "8.  GetSubsequence\n"
            << "9.  Concat\n"
            << "10. FindSubsequence\n"
            << "11. PopFirst\n"
            << "12. PopLast\n"
            << "13. PeekFirst\n"
            << "14. PeekLast\n"
            << "15. Replace Empty\n"
            << "=========================================\n"
            << "Выбор: ";
}

int run() {
    DequeType *current = nullptr;
    int choice = -1;

    while (choice != 0) {
        printMenu();
        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "Некорректный ввод.\n";
            continue;
        }

        try {
            switch (choice) {
                case 0: break;
                case 1: {
                    if (current) delete current;
                    size_t segLen = 0;
                    std::cout << "Длина сегмента (>=2): ";
                    if (!(std::cin >> segLen) || segLen < 2) {
                        clearInput();
                        throw std::invalid_argument("Длина сегмента должна быть >= 2");
                    }
                    current = new DequeType(segLen);
                    std::cout << "Deque создан.\n";
                    break;
                }
                case 2:
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    std::cout << *current << "\n";
                    break;
                case 3:
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    std::cout << "Длина: " << current->GetLength() << "\n";
                    std::cout << "Пусто: " << (current->IsEmpty() ? "Да" : "Нет") << "\n";
                    break;
                case 4: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    T val;
                    std::cout << "Значение: ";
                    if (!(std::cin >> val)) {
                        clearInput();
                        break;
                    }
                    safeReplace(current, current->AppendImpl(val));
                    std::cout << "Добавлено.\n";
                    break;
                }
                case 5: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    T val;
                    std::cout << "Значение: ";
                    if (!(std::cin >> val)) {
                        clearInput();
                        break;
                    }
                    safeReplace(current, current->PrependImpl(val));
                    std::cout << "Добавлено.\n";
                    break;
                }
                case 6: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    T val;
                    size_t idx;
                    std::cout << "Значение: ";
                    if (!(std::cin >> val)) {
                        clearInput();
                        break;
                    }
                    std::cout << "Индекс: ";
                    if (!(std::cin >> idx)) {
                        clearInput();
                        break;
                    }
                    safeReplace(current, current->InsertAtImpl(val, idx));
                    std::cout << "Вставлено.\n";
                    break;
                }
                case 7: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    size_t idx;
                    std::cout << "Индекс: ";
                    if (!(std::cin >> idx)) {
                        clearInput();
                        break;
                    }
                    safeReplace(current, current->DelImpl(idx));
                    std::cout << "Удалено.\n";
                    break;
                }
                case 8: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    size_t start, end;
                    std::cout << "Начальный индекс: ";
                    if (!(std::cin >> start)) {
                        clearInput();
                        break;
                    }
                    std::cout << "Конечный индекс: ";
                    if (!(std::cin >> end)) {
                        clearInput();
                        break;
                    }
                    safeReplace(current, current->GetSubsequence(start, end));
                    std::cout << "Получено.\n";
                    break;
                }
                case 9: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    auto *temp = createTempSequence();
                    if (!temp) {
                        std::cout << "Отменено.\n";
                        break;
                    }
                    auto *res = current->ConcatImpl(*temp);
                    delete temp;
                    safeReplace(current, res);
                    std::cout << "Объединено.\n";
                    break;
                }
                case 10: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    auto *sub = createTempSequence();
                    if (!sub) {
                        std::cout << "Отменено.\n";
                        break;
                    }
                    int pos = current->FindSubsequence(*sub);
                    delete sub;
                    if (pos != -1) std::cout << "Найдено по индексу: " << pos << "\n";
                    else std::cout << "Не найдено.\n";
                    break;
                }
                case 11: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    if (current->IsEmpty()) {
                        std::cout << "Пусто.\n";
                        break;
                    }
                    T val = current->PopFirst();
                    std::cout << "Извлечено: " << val << "\n";
                    break;
                }
                case 12: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    if (current->IsEmpty()) {
                        std::cout << "Пусто.\n";
                        break;
                    }
                    T val = current->PopLast();
                    std::cout << "Извлечено: " << val << "\n";
                    break;
                }
                case 13: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    std::cout << "Первый: " << current->GetFirst() << "\n";
                    break;
                }
                case 14: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    std::cout << "Последний: " << current->GetLast() << "\n";
                    break;
                }
                case 15: {
                    if (!current) {
                        std::cout << "Сначала создайте Deque.\n";
                        break;
                    }
                    safeReplace(current, current->CreateEmpty());
                    std::cout << "Заменено.\n";
                    break;
                }
                default:
                    std::cout << "Неверный выбор.\n";
                    break;
            }
        } catch (const std::exception &e) {
            std::cout << "Ошибка: " << e.what() << "\n";
        }
    }
    delete current;
    std::cout << "Завершение работы.\n";
    return 0;
}