#include <iostream>


//42) (3*a*a-40-3*b)/(4*c-98/d);
int calc(int a, int b, int c, int d, int* hasError) {
    int res = 0;

    __asm {
        mov eax, a
        mov ebx, b
        mov ecx, c
        mov edx, d
        mov esi, hasError
        mov dword ptr[esi], 0

        cmp edx, 0
        jne d_ok;
        je error;

        mov res, 0
        jmp done
    d_ok:
        imul eax, eax;  a* a
        imul eax, 3; 3 * a * a
        sub eax, 40; 3 * a * a - 40

        imul ebx, 3; b * 3
        sub eax, ebx; 3 * a * a - 40 - 3 * b

        push eax; stack

        mov eax, 98; eax = 98
        cdq;
        idiv d; 98 / d
        mov edx, eax;  b = 98 / d
        
        pop eax;

        imul ecx, 4;c*4
        sub ecx, edx;c-98/d

        cmp ecx, 0;
        je error;

        cdq;
        idiv ecx;  (3*a*a-40-3*b)/(4*c-98/d)
        mov res, eax;

        jmp done;
    error:
        mov dword ptr[esi], 1
        mov res, 0
    done:
    }

    return res;
}

//42) (3*a*a-40-3*b)/(4*c-98/d);
int calc_cpp(int a, int b, int c, int d, bool& hasError) {
    hasError = false;

    if (d == 0) {
        hasError = true;
        return 0;
    }

    int num = 3 * a * a - 40 - 3 * b;
    int part = 98 / d;
    int den = 4 * c - part;

    if (den == 0) {
        hasError = true;
        return 0;
    }

    return num / den;
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

    int a, b, c, d, status;
    readInt(a, "a");
    readInt(b, "b");
    readInt(c, "c");
    readInt(d, "d");

    std::cout << "Результат выполенения функции" << std::endl;
    
    int res = calc(a, b, c, d, &status);
    if (status == 0) {
        std::cout << "ASM: " << res << std::endl;
    }
    else {
        std::cout << "ASM: ошибка (деление на 0)" << std::endl;
    }

    bool cppError;
    int resCpp = calc_cpp(a, b, c, d, cppError);
    if (!cppError) {
        std::cout << "CPP: " << resCpp << std::endl;
    }
    else {
        std::cout << "CPP: ошибка (деление на 0)" << std::endl;
    }

    system("pause");
}