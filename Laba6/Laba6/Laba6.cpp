#define _USE_MATH_DEFINES
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <clocale>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cstddef>
#include <stdexcept>
#include <string>

/*
  Лабораторная работа №6. Вариант 43.
*/

inline bool finiteVal(double v) {
    return std::isfinite(v) && !std::isnan(v);
}

double f_cpp(double x) {
    if (x <= 0.0)
        throw std::domain_error("f(x): аргумент x должен быть > 0");
    double sq = std::sqrt(x);
    if (sq == 0.0)
        throw std::domain_error("f(x): деление на ноль (sqrt(x) = 0)");
    double out = 2.0 * x + 3.0 / sq;
    if (!finiteVal(out))
        throw std::runtime_error("f(x): результат не является конечным числом");
    return out;
}

double f_asm(double x) {
    double result = 0.0;
    int err = 0;
    int sw = 0;
    static const double c2 = 2.0;
    static const double c3 = 3.0;

    __asm {
        finit; init FPU

        fld qword ptr [x]
        ftst; x <= 0 ?
        fstsw word ptr [sw]
        mov ah, byte ptr [sw+1]
        sahf
        jbe _fx_err

        fsqrt; sqrt(x)
        ftst; sqrt == 0 ?
        fstsw word ptr [sw]
        mov ah, byte ptr [sw+1]
        sahf
        je _fx_err

        fld qword ptr [c3]; 3 / sqrt(x)
        fdiv st(0), st(1)
        fxch st(1)
        fstp st(0)

        fld qword ptr [c2]; 2*x
        fmul qword ptr [x]

        faddp st(1), st(0); fi
        fstp qword ptr [result]
        jmp _fx_ok

    _fx_err:
        mov dword ptr [err], 1
        finit
    _fx_ok:
    }

    if (err)
        throw std::domain_error("f(x): область определения (x > 0, sqrt(x) != 0)");
    if (!finiteVal(result))
        throw std::runtime_error("f(x): результат не является конечным числом");
    return result;
}

double simpson_cpp(double a, double b, int n) {
    if (n <= 0)
        throw std::invalid_argument("число интервалов N должно быть > 0");
    int m = 2 * n;
    double h = (b - a) / m;
    double sum = 0.0;

    for (int i = 0; i <= m; i++) {
        double xi = a + i * h;
        double fi = f_cpp(xi);
        double c = (i == 0 || i == m) ? 1.0 : (i % 2 ? 4.0 : 2.0);
        sum += c * fi;
    }

    double out = (h / 3.0) * sum;
    if (!finiteVal(out))
        throw std::runtime_error("C++: результат не является конечным числом");
    return out;
}

double simpson_asm(double a, double b, int n,
                   double* vx,
                   double* vf,
                   int* vc) {
    if (n <= 0)
        throw std::invalid_argument("число интервалов N должно быть > 0");

    int m = 2 * n;

    double* pvx = vx;
    double* pvf = vf;
    int* pvc = vc;

    double h = 0.0;
    double xi = 0.0;
    double fi = 0.0;
    double s_ends = 0.0;
    double s_odd = 0.0;
    double s_even = 0.0;
    double result = 0.0;
    double dm = static_cast<double>(m);

    int err = 0;
    int sw = 0;
    int i = 0;

    static const double c2 = 2.0;
    static const double c3 = 3.0;
    static const double c4 = 4.0;

    __asm {
        finit; init FPU
        fldz
        fstp qword ptr [s_ends]
        fldz
        fstp qword ptr [s_odd]
        fldz
        fstp qword ptr [s_even]

        fld qword ptr [b]; h = (b-a)/(2n)
        fsub qword ptr [a]
        fdiv qword ptr [dm]
        fstp qword ptr [h]

        mov eax, 0; i = 0
    _s_loop:
        cmp eax, dword ptr [m]
        ja _s_after_loop

        mov dword ptr [i], eax; i for fild

        finit; xi = a + i*h
        fild dword ptr [i]
        fmul qword ptr [h]
        fadd qword ptr [a]
        fstp qword ptr [xi]

        mov edx, eax
        shl edx, 3; edx = i*8
        mov ecx, pvx
        fld qword ptr [xi]
        fstp qword ptr [ecx+edx]; vx[i]

        finit; f(xi) = 2*xi + 3/sqrt(xi)
        fld qword ptr [xi]
        ftst; xi <= 0 ?
        fstsw word ptr [sw]
        mov ah, byte ptr [sw+1]
        sahf
        jbe _s_f_err

        fsqrt
        ftst; div by zero ?
        fstsw word ptr [sw]
        mov ah, byte ptr [sw+1]
        sahf
        je _s_f_err

        fld qword ptr [c3]
        fdiv st(0), st(1)
        fxch st(1)
        fstp st(0)
        fld qword ptr [c2]
        fmul qword ptr [xi]
        faddp st(1), st(0)
        fstp qword ptr [fi]
        jmp _s_f_ok

    _s_f_err:
        mov dword ptr [err], 1
        finit
        jmp _s_asm_end

    _s_f_ok:
        mov ecx, pvf
        fld qword ptr [fi]
        fstp qword ptr [ecx+edx]; vf[i]

        mov eax, dword ptr [i]; restore i: mov ah corrupts EAX via AH bits
        cmp eax, 0; ends ?
        je _s_c1
        cmp eax, dword ptr [m]
        je _s_c1

        test eax, 1; odd index ?
        jnz _s_c4

        mov ecx, pvc; coeff 2
        mov dword ptr [ecx+eax*4], 2
        finit
        fld qword ptr [s_even]
        fadd qword ptr [fi]
        fstp qword ptr [s_even]
        jmp _s_next

    _s_c1:
        mov ecx, pvc; coeff 1
        mov dword ptr [ecx+eax*4], 1
        finit
        fld qword ptr [s_ends]
        fadd qword ptr [fi]
        fstp qword ptr [s_ends]
        jmp _s_next

    _s_c4:
        mov ecx, pvc; coeff 4
        mov dword ptr [ecx+eax*4], 4
        finit
        fld qword ptr [s_odd]
        fadd qword ptr [fi]
        fstp qword ptr [s_odd]

    _s_next:
        inc eax
        jmp _s_loop

    _s_after_loop:
        finit; (h/3)*(s_ends + 4*s_odd + 2*s_even)
        fld qword ptr [h]
        fdiv qword ptr [c3]
        fld qword ptr [s_ends]
        fld qword ptr [c4]
        fmul qword ptr [s_odd]
        faddp st(1), st(0)
        fld qword ptr [c2]
        fmul qword ptr [s_even]
        faddp st(1), st(0)
        fmulp st(1), st(0)
        fstp qword ptr [result]

    _s_asm_end:
    }

    if (err)
        throw std::domain_error(
            "подынтегральная функция не определена в некоторых точках разбиения "
            "(деление на ноль или корень из отрицательного числа)");
    if (!finiteVal(result))
        throw std::runtime_error("ASM: результат не является конечным числом");
    return result;
}

double F_cpp(double x) {
    if (x < 0.0)
        throw std::domain_error("F(x): аргумент x должен быть >= 0");
    const double out = x * x + 6.0 * std::sqrt(x);
    if (!finiteVal(out))
        throw std::runtime_error("F(x): результат не является конечным числом");
    return out;
}

void printHeader() {
    std::setlocale(LC_ALL, "ru_RU");
    std::cout << "\033[33m";
    std::cout << "     Лабораторная работа ";
    std::cout << "\033[1;31m06\033[33m\n";
    std::cout << "\033[0m";

    std::cout << "Группа: ";
    std::cout << "\033[36m6102-020302D\033[0m\n";

    std::cout << "Студент: ";
    std::cout << "\033[32mДудкин Егор Денисович\033[0m\n";

    std::cout << "Вариант задания: ";
    std::cout << "\033[1;31m43\033[0m\n\n";

    std::cout << "\033[90m------------------------------------------------------------\033[0m\n";
    std::cout << "\033[1mЗадание\033[0m\n\n";
    std::cout << "  Определённый интеграл:\n\n";
    std::cout << "              4\n";
    std::cout << "        integral f(x) dx\n";
    std::cout << "              1\n\n";
    std::cout << "                3\n";
    std::cout << "  f(x) = 2x + -------\n";
    std::cout << "              sqrt(x)\n\n";
    std::cout << "\033[90m------------------------------------------------------------\033[0m\n\n";
}

void readDouble(double& v, const char* prompt) {
    while (true) {
        std::cout << prompt;
        if (std::cin >> v)
            return;
        std::cout << "Ошибка ввода.\n";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
}

void readInt(int& v, const char* prompt) {
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v > 0)
            return;
        std::cout << "Ошибка: N должно быть целым числом > 0.\n";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
}

static void printBorder(int wi, int wx, int wf, int wc) {
    auto bar = [](int w) { for (int j = 0; j < w + 2; j++) std::cout << '-'; };
    std::cout << '+'; bar(wi);
    std::cout << '+'; bar(wx);
    std::cout << '+'; bar(wf);
    std::cout << '+'; bar(wc);
    std::cout << "+\n";
}

void printSimpsonStep(const char* title,
                      double h, int m,
                      const double* fvals,
                      double result) {
    std::cout << "\nПошаговый расчет интеграла по формуле Симпсона (" << title << "):\n";
    std::cout << "I = (h / 3) * [ (f(a) + f(b)) + 4 * (";

    bool firstOdd = true;
    bool firstEven = true;
    double sumOdd = 0.0;
    double sumEven = 0.0;

    for (int i = 1; i < m; i += 2) {
        if (!firstOdd) std::cout << " + ";
        std::cout << fvals[i];
        firstOdd = false;
        sumOdd += fvals[i];
    }

    std::cout << ") + 2 * (";

    for (int i = 2; i < m; i += 2) {
        if (!firstEven) std::cout << " + ";
        std::cout << fvals[i];
        firstEven = false;
        sumEven += fvals[i];
    }

    std::cout << ") ]\n";
    std::cout << "  = (" << h << " / 3) * [ (" << fvals[0] << " + " << fvals[m]
              << ") + 4 * (" << sumOdd << ") + 2 * (" << sumEven << ") ]\n";
    std::cout << "  = " << result << "\n";
}

int main() {
    std::cout << std::fixed;
    std::cout.precision(15);

    printHeader();

    double a, b;
    int n;
    readDouble(a, "Нижний предел  a = ");
    readDouble(b, "Верхний предел b = ");
    readInt(n,    "Число интервалов разбиения N = ");

    double* vx = nullptr;
    double* vf_asm = nullptr;
    double* vf_cpp = nullptr;
    int* vc = nullptr;

    try {
        if (b <= a)
            throw std::invalid_argument("b должно быть больше a.");
        if (a <= 0.0)
            throw std::invalid_argument("a должно быть > 0 (деление на sqrt(x)).");

        const int m = 2 * n;

        vx = new double[m + 1];
        vf_asm = new double[m + 1];
        vf_cpp = new double[m + 1];
        vc = new int[m + 1];

        const double result_asm = simpson_asm(a, b, n, vx, vf_asm, vc);
        const double h = (b - a) / static_cast<double>(m);
        for (int j = 0; j <= m; j++) {
            vf_cpp[j] = f_cpp(vx[j]);
        }
        const double result_cpp = simpson_cpp(a, b, n);

        const int wi = 4, wx = 16, wf = 18;

        std::cout << "\n\033[1mТаблица расчётов:\033[0m\n";
        printBorder(wi, wx, wf, wf);
        std::cout << "| " << std::setw(wi) << "i"
                  << " | " << std::setw(wx) << "x_i"
                  << " | " << std::setw(wf) << "f_asm(x_i)"
                  << " | " << std::setw(wf) << "f_cpp(x_i)"
                  << " |\n";
        printBorder(wi, wx, wf, wf);

        for (int j = 0; j <= m; j++) {
            std::cout << "| " << std::setw(wi) << j
                      << " | " << std::setw(wx) << vx[j]
                      << " | " << std::setw(wf) << vf_asm[j]
                      << " | " << std::setw(wf) << vf_cpp[j]
                      << " |\n";
        }
        printBorder(wi, wx, wf, wf);

        printSimpsonStep("ASM", h, m, vf_asm, result_asm);
        printSimpsonStep("C++", h, m, vf_cpp, result_cpp);

        const double exact = std::fabs(F_cpp(b) - F_cpp(a));
        const double err_asm = std::fabs(exact - result_asm);
        const double err_cpp = std::fabs(exact - result_cpp);
        const double diff_asm_cpp = std::fabs(result_asm - result_cpp);

        std::cout << "\n=== Сравнение результатов ===\n";
        std::cout << "Asm результат        : " << result_asm << "\n";
        std::cout << "C++ результат        : " << result_cpp << "\n";
        std::cout << "Точное значение      : " << exact << "\n\n";
        std::cout << "Разность (Asm - C++) : " << diff_asm_cpp << "\n";
        std::cout << "Погрешность Asm      : " << err_asm << "\n";
        std::cout << "Погрешность C++      : " << err_cpp << "\n";

        std::cout << '\n';
        delete[] vc; vc = nullptr;
        delete[] vf_cpp; vf_cpp = nullptr;
        delete[] vf_asm; vf_asm = nullptr;
        delete[] vx; vx = nullptr;
    } catch (const std::exception& e) {
        delete[] vc;
        delete[] vf_cpp;
        delete[] vf_asm;
        delete[] vx;
        std::cout << "\n\033[1;31mОшибка:\033[0m " << e.what() << "\n";
        system("pause");
        return 1;
    }

    system("pause");
    return 0;
}
