#include <iostream>

void printMenu() {
    setlocale(LC_ALL, "ru-RU");
    std::cout << "\033[33m";
    std::cout << "     Лабораторная работа ";
    std::cout << "\033[1;31m02\033[33m\n";
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


// a/b - a, a>b
// -a, a == b
// (a*b-5)/(a+b), a<b

int calc_asm(int a, int b, bool* hasError) {
    __asm {
        mov ecx, a
        mov ebx, b
        mov esi, hasError
        mov byte ptr[esi], 0;

        cmp ecx, ebx
        jg A_BIGGER
        jl A_SMALLER

        mov eax, ecx
        neg eax

        jmp DONE
    A_BIGGER:
        mov eax, ecx;
        ; x = a / b - a
        cmp ebx, 0
        je  DIV_ZERO

        cdq
        idiv ebx  ; eax = a / b
        sub eax, ecx; eax = a / b - a

        jmp DONE
    A_SMALLER:
        ; a < b: x = (a*b-5)/(a+b)
        mov eax, ecx ; a
        imul eax, ebx ; a*b
        jo IMUL_OVERFLOW
        sub eax, 5 ; a*b-5

        mov edi, ecx ; a
        add edi, ebx ; a+b
        cmp edi, 0 ; (a+b)==0
        je  DIV_ZERO

        cdq ; a*b-5
        idiv edi ; (a*b-5)/(a+b)
        jmp DONE
    IMUL_OVERFLOW:
        xor eax, eax
        mov byte ptr[esi], 1
    DIV_ZERO:
        xor eax, eax
        mov byte ptr[esi], 1;
    DONE:
    }
}

int calc_cpp(int a, int b, bool& hasError) {
    hasError = false;

    if (a > b) {
        if (b == 0) {
            hasError = true;
            return 0;
        }
        return a / b - a;
    }

    if (a == b) {
        return -a;
    }

    int den = a + b;
    if (den == 0) {
        hasError = true;
        return 0;
    }

    int num = a * b - 5;
    return num / den;
}


int main()
{
    printMenu();

    int a, b;
    readInt(a, "a");
    readInt(b, "b");

    std::cout << "Результат выполенения функций " << std::endl;
    bool asmError = false;
    int resAsm = calc_asm(a, b, &asmError);
    if (!asmError) std::cout << "ASM: " << resAsm << "\n";
    else std::cout << "ASM: ошибка (деление на 0)\n";


    bool cppError = false;
    int resCpp = calc_cpp(a, b, cppError);
    if (!cppError) std::cout << "CPP: " << resCpp << "\n";
    else std::cout << "CPP: ошибка (деление на 0)\n";


    system("pause");
}