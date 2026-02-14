#include <iostream>


//42) (3*a*a-40-3*b)/(4*c-98/d);
int calc(int a, int b, int c, int d) {
    int res = 0;

    __asm {
        mov eax, a
        mov ebx, b
        mov ecx, c
        mov edx, d

        cmp edx, 0
        jne d_ok

        mov res, 0
        jmp done
    d_ok:
        imul eax, eax //a*a
        imul eax, 3 //3*a*a
        sub eax, 40 //3*a*a-40

        imul ebx, 3//b*3
        sub eax, ebx//3*a*a - 40 - 3*b

        push eax;//stack

        mov eax, 98;//eax=98
        cdq;
        idiv d;//98/d
        mov edx, eax;//b=98/d
        
        pop eax;

        imul ecx, 4;//c*4
        sub ecx, edx;//c-98/d

        cdq;
        idiv ecx; // (3*a*a-40-3*b)/(4*c-98/d)
        mov res, eax;
    done:
    }

    return res;
}

void printMenu() {
    setlocale(LC_ALL,"ru-RU");
    std::cout << "\033[33m";
    std::cout << "     Лабораторная работа ";
    std::cout << "\033[1;31m01\033[33m\n";
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

int main()
{
    printMenu();

    int a, b, c, d;
    readInt(a, "a");
    readInt(b, "b");
    readInt(c, "c");
    readInt(d, "d");

    try {
        std::cout << "Результат выполенения функции calc: " << calc(a, b, c, d) << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    system("pause");
}