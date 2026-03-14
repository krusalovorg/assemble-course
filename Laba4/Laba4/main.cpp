#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <string>
#include <stdexcept>

void printMenu() {
    setlocale(LC_ALL, "ru_RU");
    std::cout << "\033[33m";
    std::cout << "     Лабораторная работа ";
    std::cout << "\033[1;31m04\033[33m\n";
    std::cout << "\033[0m";

    std::cout << "Группа: ";
    std::cout << "\033[36m6102-020302D\033[0m\n";

    std::cout << "Студент: ";
    std::cout << "\033[32mДудкин Егор Денисович\033[0m\n";

    std::cout << "Вариант задания: ";
    std::cout << "\033[1;31m42\033[0m\n\n";

    std::cout << "Задание: X = 42*pi - sin(b) - cos(-b) - tg(a) - ctg(a) - pi * (42 * b + a)/(42 * a - b)\n\n";
}

void readDouble(double& value, std::string name) {
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


// 42) X= 42 * pi -sin(b)-cos(-b)-tg(a)-ctg(a)- pi*(42*b+a)/(42*a-b)
// = 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a) - pi* ( 42 * b + a ) / (42 * a - b)
// ctg(a) = 1/tg(a), tg(a) = 0 если a=kpi
// деление на 0 если a=0 b=0

double calc(double a, double b) {
    double res;
    int status;
    int error = 0;
    const int c42 = 42;

    __asm {
        finit; init FPU

        ; 42 * pi
        fild dword ptr[c42]; st0 = 42
        fldpi; pi | 42
        fmulp st(1), st; 42 * pi

        ; -sin(b)
        fld qword ptr[b]; b | 42 * pi
        fsin; sin(b) | 42 * pi
        fsubp st(1), st; 42 * pi - sin(b)

        ; -cos(b)
        fld qword ptr[b]; b | 42 * pi - sin(b)
        fcos; cos(b) | 42 * pi - sin(b)
        fsubp st(1), st; 42 * pi - sin(b) - cos(b)

        ; -tg(a)
        fld qword ptr[a]; a | 42 * pi - sin(b) - cos(b)
        fptan; 1 | tg(a) | 42 * pi - sin(b) - cos(b)
        fstp st(0); tg(a) | 42 * pi..
        fsubp st(1), st; 42 * pi - sin(b) - cos(b) - tg(a)

        ; -ctg(a)
        fld qword ptr[a]; a | 42 * pi - sin(b) - cos(b) - tg(a)
        fptan; 1 | tg(a) | ...
        fstp st(0); tg(a) | ...

        ftst; st0 = tg(a) == 0 ?
        fstsw status; FPU status word сохраняем в status
        mov ah, byte ptr[status + 1]
        sahf
        je error_tg; если tg(a) == 0, то ошибка

        fld1
        fdivrp st(1), st; 1 / tg(a) | ...
        fsubp st(1), st; 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)

        ; 42 * b + a
        fld qword ptr[a]; a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fld qword ptr[b]; b | a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fild dword ptr[c42]; 42 | b | a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fmulp st(1), st; 42 * b | a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        faddp st(1), st; 42 * b + a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)

        ; 42 * a - b
        fld qword ptr[b]; b | 42 * b + a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fld qword ptr[a]; a | b | 42 * b + a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fild dword ptr[c42]; 42 | a | b | 42 * b + a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fmulp st(1), st; 42 * a | b | 42 * b + a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fsubrp st(1), st; 42 * a - b | 42 * b + a | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)

        ftst; denom ? 0
        fstsw status
        mov ah, byte ptr[status + 1]
        sahf
        je error_denom; если denom == 0, то ошибка


        ; (42 * b + a) / (42 * a - b)
        fdivp st(1), st; (42 * b + a) / (42 * a - b) | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)

        ; res
        fldpi; pi | (42 * b + a) / (42 * a - b) | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fmulp st(1), st; pi* (42 * b + a) / (42 * a - b) | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fsubp st(1), st; 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a) - pi * (42 * b + a) / (42 * a - b)

        fstp qword ptr[res];
        jmp done
            error_tg :
        mov error, 1
            finit
            fldz
            fstp qword ptr[res]
            jmp done
            error_denom :
        mov error, 2
            finit
            fldz
            fstp qword ptr[res]
            done :
    }

    if (error == 1) {
        throw std::invalid_argument("ОШИБКА: Деление на ноль! (tg(a)=0, ctg не определён)");
    }
    if (error == 2) {
        throw std::invalid_argument("ОШИБКА: Деление на ноль! (42*a-b=0)");
    }
    return res;
}

double calcCpp(double a, double b) {
    const double denom = 42.0 * a - b;
    double tgA = std::tan(a);

    if (tgA == 0.0) {
        throw std::invalid_argument("ОШИБКА: Деление на ноль! (tg(a)=0, ctg не определён)");
    }
    if (denom == 0.0) {
        throw std::invalid_argument("ОШИБКА: Деление на ноль! (42*a-b=0)");
    }

    return 42.0 * M_PI - std::sin(b) - std::cos(-b) - tgA - 1.0 / tgA
        - M_PI * (42.0 * b + a) / denom;
}

int main()
{
    printMenu();

    double a, b;
    readDouble(a, "a");
    readDouble(b, "b");

    try {
        std::cout << "ASM: " << calc(a, b);
    }
    catch (const std::invalid_argument& e) {
        std::cout << e.what();
    }
    std::cout << std::endl;
    try {
        std::cout << "CPP: " << calcCpp(a, b);
    }
    catch (const std::invalid_argument& e) {
        std::cout << e.what();
    }
    std::cout << std::endl;

    system("pause");
}