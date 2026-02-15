#include <iostream>
#include <string>

void printMenu() {
    setlocale(LC_ALL, "ru-RU");
    std::cout << "\033[33m";
    std::cout << "     Лабораторная работа ";
    std::cout << "\033[1;31m03\033[33m\n";
    std::cout << "\033[0m";

    std::cout << "Группа: ";
    std::cout << "\033[36m6102-020302D\033[0m\n";

    std::cout << "Студент: ";
    std::cout << "\033[32mДудкин Егор Денисович\033[0m\n";

    std::cout << "Вариант задания: ";
    std::cout << "\033[1;31m42\033[0m\n\n";
}

void readInt(int& value, std::string name) {
    while (true) {
        std::cout << "Введите значение " << name << ": ";
        std::cin >> value;

        if (!std::cin.fail()) {
            return;
        }

        std::cout << "Ошибка ввода\n";
        std::cin.clear();
        std::cin.ignore(100, '\n');
    }
}


// 42)	В одномерном массиве A={a[i]} целых чисел 
// вычислить сумму элементов с нечетными номерами.

int calc(int* arr, int n) {
    __asm {
        xor esi, esi; индекс
        xor eax, eax; сумма эл
        mov ebx, arr; начало массива
        mov ecx, n; счетчик по всем эл.

        jecxz done;
    begin_loop:
        test esi, 1; проверяю первый бит
        jz next ; i четная скип

        add eax, [ebx + esi * 4]; текущий элемент
    next:
        inc	esi; i++
        loop begin_loop
    done:
    }
}

int main()
{
    printMenu();

    int n;
    readInt(n, "n");
    int* arr = new int[n];

    for (int i = 0; i < n; i++) {
        int v;
        readInt(v, "arr[" + std::to_string(i) + "]");
        arr[i] = v;
    }

    try {
        std::cout << "Результат выполенения функции: " << calc(arr, n) << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    system("pause");
}