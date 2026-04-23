#define _USE_MATH_DEFINES
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <clocale>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

/*
  Лабораторная работа 7. Вариант 43.
  y = sum_{n=0..inf} ((x - 1)^n / n!)
*/

inline bool finiteVal(double v) {
    return std::isfinite(v) && !std::isnan(v);
}

void printHeader() {
    std::setlocale(LC_ALL, "ru_RU");
    std::cout << "\033[33m";
    std::cout << "     Лабораторная работа ";
    std::cout << "\033[1;31m07\033[33m\n";
    std::cout << "\033[0m";

    std::cout << "Группа: ";
    std::cout << "\033[36m6102-020302D\033[0m\n";

    std::cout << "Студент: ";
    std::cout << "\033[32mДудкин Егор Денисович\033[0m\n";

    std::cout << "Вариант задания: ";
    std::cout << "\033[1;31m43\033[0m\n\n";

    std::cout << "\033[90m------------------------------------------------------------\033[0m\n";
    std::cout << "\033[1mРяд\033[0m\n\n";
    std::cout << "              inf    (x - 1)^n  \n";
    std::cout << "  y = f(x) = SUM  (  ---------  )\n";
    std::cout << "             n=0        n!\n\n";
    std::cout << "\033[90m------------------------------------------------------------\033[0m\n\n";
}

void readDouble(double& v, const char* prompt) {
    while (true) {
        std::cout << prompt;
        if (std::cin >> v) {
            return;
        }
        std::cout << "Ошибка ввода.\n";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
}

void readInt(int& v, const char* prompt) {
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0) {
            return;
        }
        std::cout << "Ошибка: N должно быть целым числом >= 0.\n";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
}

double partialSumCppFormula(double x, int i) {
    const double d = x - 1.0;
    double sum = 0.0;

    for (int n = 0; n <= i; ++n) {
        double powPart = std::pow(d, n);
        double fact = 1.0;
        for (int k = 2; k <= n; ++k) {
            fact *= static_cast<double>(k);
        }
        sum += powPart / fact;
    }

    if (!finiteVal(sum)) {
        throw std::runtime_error("C++: получена некорректная частичная сумма.");
    }
    return sum;
}

void calcSeriesAsm(double x, int n, double* sumsAsm) {
    const double delta = x - 1.0;
    static const double one = 1.0;

    double term = 1.0;
    double sum = 1.0;
    int iLocal = 1;
    int err = 0;
    double* pOut = sumsAsm;

    __asm {
        finit
        mov edi, pOut
        fld qword ptr [sum]             ; st0 = sum
        fstp qword ptr [edi]            ; sumsAsm[0] = 1

        mov ecx, dword ptr [n]
        cmp ecx, 0
        jle _asm_done

        add edi, 8                      ; &sumsAsm[1]

    _asm_loop:
        ; term = term * delta / iLocal
        fld qword ptr [term]            ; st0 = term
        fmul qword ptr [delta]          ; st0 = term * delta
        fild dword ptr [iLocal]         ; st0 = i, st1 = term*delta
        fdivp st(1), st(0)              ; st0 = new term
        fst qword ptr [term]            ; term

        ; sum += term
        fadd qword ptr [sum]            ; st0 = term + sum
        fst qword ptr [sum]             ; sum
        fstp qword ptr [edi]            ; sumsAsm[i] = sum

        add edi, 8
        inc dword ptr [iLocal]
        loop _asm_loop

    _asm_done:
        finit
    }

    if (err) {
        throw std::runtime_error("ASM: ошибка при вычислении ряда.");
    }
    if (!finiteVal(term) || !finiteVal(sum)) {
        throw std::runtime_error("ASM: некорректные значения ряда (переполнение/NaN).");
    }
}

static void printBorder(int wi, int wr, int we) {
    auto bar = [](int w) {
        for (int j = 0; j < w + 2; ++j) {
            std::cout << '-';
        }
    };

    std::cout << '+';
    bar(wi);
    std::cout << '+';
    bar(wr);
    std::cout << '+';
    bar(wr);
    std::cout << '+';
    bar(we);
    std::cout << "+\n";
}

int main() {
    std::cout << std::fixed << std::setprecision(15);
    printHeader();

    double x = 0.0;
    int n = 0;
    readDouble(x, "Введите x = ");
    readInt(n, "Введите N (количество членов ряда до N включительно) = ");

    double* sumsCpp = nullptr;
    double* sumsAsm = nullptr;

    try {
        sumsCpp = new double[n + 1];
        sumsAsm = new double[n + 1];

        for (int i = 0; i <= n; ++i) {
            sumsCpp[i] = partialSumCppFormula(x, i);
        }
        calcSeriesAsm(x, n, sumsAsm);

        const int wi = 4;
        const int wr = 22;
        const int we = 22;
        const double exactRef = std::exp(x - 1.0);

        std::cout << "\n\033[1mТаблица вычислений:\033[0m\n";
        printBorder(wi, wr, we);
        std::cout << "| " << std::setw(wi) << "i"
                  << " | " << std::setw(wr) << "cpp"
                  << " | " << std::setw(wr) << "asm"
                  << " | " << std::setw(we) << "eps"
                  << " |\n";
        printBorder(wi, wr, we);

        for (int i = 0; i <= n; ++i) {
            const double eps = std::fabs(exactRef - sumsAsm[i]);
            std::cout << "| " << std::setw(wi) << i
                      << " | " << std::setw(wr) << sumsCpp[i]
                      << " | " << std::setw(wr) << sumsAsm[i]
                      << " | " << std::setw(we) << eps
                      << " |\n";
        }
        printBorder(wi, wr, we);

        const double exact = exactRef;
        const double errCpp = std::fabs(exact - sumsCpp[n]);
        const double errAsm = std::fabs(exact - sumsAsm[n]);
        const double diff = std::fabs(sumsCpp[n] - sumsAsm[n]);

        std::cout << "\nКонтроль по аналитической формуле y = exp(x - 1):\n";
        std::cout << "Точное значение          : " << exact << "\n";
        std::cout << "cpp (N)                  : " << sumsCpp[n] << "\n";
        std::cout << "asm (N)                  : " << sumsAsm[n] << "\n";
        std::cout << "|cpp - asm|              : " << diff << "\n";
        std::cout << "|exact - cpp (N)|        : " << errCpp << "\n";
        std::cout << "|exact - asm (N)|        : " << errAsm << "\n";

        delete[] sumsCpp;
        delete[] sumsAsm;
        sumsCpp = nullptr;
        sumsAsm = nullptr;
    } catch (const std::exception& e) {
        delete[] sumsCpp;
        delete[] sumsAsm;
        std::cout << "\n\033[1;31mОшибка:\033[0m " << e.what() << "\n";
        system("pause");
        return 1;
    }

    system("pause");
    return 0;
}
