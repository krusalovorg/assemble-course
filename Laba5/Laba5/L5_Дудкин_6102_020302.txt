#define _USE_MATH_DEFINES
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <clocale>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <format>
#include <limits>
#include <cstddef>
#include <string>

/*
  Вариант 43
*/

inline bool finiteVal(double v) {
    return std::isfinite(v) && !std::isnan(v);
}

bool calcCpp(double x, double y, double& out) {
    if (x <= 0.0)
        return false;
    if (y == 0.0)
        return false;

    const double sin_y = std::sin(y);
    if (sin_y == 0.0 || !finiteVal(sin_y))
        return false;

    const double x2 = x * 0.5;
    const double cos_x2 = std::cos(x2);
    if (cos_x2 == 0.0 || !finiteVal(cos_x2))
        return false;
    
    const double sin_x2 = std::sin(x2);
    const double t_x2 = std::tan(x2);
    if (!finiteVal(t_x2))
        return false;

    const double cpp_43_44 = 43.0 / 44.0;
    const double cpp_2_3 = 2.0 / 3.0;
    const double cpp_neg2_3 = -2.0 / 3.0;

    const double t1 = std::pow(0.5, -x / 4.0);
    const double t2 = std::pow(x, x / 5.0);
    const double t3 = std::pow(std::fabs(cos_x2), cpp_neg2_3);
    const double t4 = std::exp(std::sin(x));
    const double t5 = std::pow(std::fabs(sin_x2), cpp_2_3);
    const double t6 = 0.5 * std::log(x * 0.5);
    const double t7 = std::pow(std::fabs(t_x2), cpp_2_3);

    if (!finiteVal(t1) || !finiteVal(t2) || !finiteVal(t3) || !finiteVal(t4) ||
        !finiteVal(t5) || !finiteVal(t6) || !finiteVal(t7))
        return false;

    const double inner = t1 + t2 + t3 + t4 + t5 + t6 - t7;
    if (inner < 0.0 || !finiteVal(inner))
        return false;

    const double y_inv = std::pow(y, cpp_neg2_3);
    if (!finiteVal(y_inv))
        return false;

    const double y_pow = (y >= 0) ? std::pow(y, cpp_2_3) : -std::pow(-y, cpp_2_3);

    out = std::pow(x, cpp_43_44) + std::sqrt(inner) + std::sin(y_inv) +
          5.0 * std::exp(-2.0 * y / 3.0) - std::cos(y_pow) + std::exp(std::cos(y)) +
          1.0 / sin_y;
    return finiteVal(out);
}

double calcAsm(double x, double y) {
    double res = 0.0;
    int error = 0;
    int status = 0;

    double sin_y, neg_x_4, x_div_5, x2, cos_x2, cx_abs;
    double t1, t2, t3, t4, t5, t6, t7, inner, acc;
    double y_neg2_3, ay, yp;

    static const double d4 = 4.0;
    static const double d5 = 5.0;
    static const double dHalf = 0.5;

    if (x <= 0.0 || y == 0.0)
        return std::nan("");
    
    __asm {
        finit; init FPU

        ; sin(y)
        fld qword ptr[y]
        fsin
        fstp qword ptr[sin_y]
        fld qword ptr[sin_y]
        ftst; sin(y) == 0 ?
        fstsw word ptr[status]
        mov ah, byte ptr[status + 1]
        sahf
        je err_done

        ; acc = x^(43/44)
        push 44
        push 43
        fild dword ptr [esp]
        fild dword ptr [esp+4]
        fdivrp st(1), st
        add esp, 8
        fld qword ptr[x]
        fyl2x
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[acc]

        ; t1 = (1/2)^(-x/4) = 2^(x/4)
        fld qword ptr[x]
        fld qword ptr[d4]
        fdivp st(1), st
        fchs; -x/4
        fstp qword ptr[neg_x_4]
        fld qword ptr[neg_x_4]
        fld qword ptr[dHalf]
        fyl2x
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[t1]

        ; t2 = x^(x/5)
        fld qword ptr[x]
        fld qword ptr[d5]
        fdivp st(1), st
        fstp qword ptr[x_div_5]
        fld qword ptr[x_div_5]
        fld qword ptr[x]
        fyl2x
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[t2]

        ; x2 = x/2, cos(x2) != 0
        fld qword ptr[x]
        fld qword ptr[dHalf]
        fmulp st(1), st
        fstp qword ptr[x2]
        finit
        fld qword ptr[x2]
        fcos
        fstp qword ptr[cos_x2]
        fld qword ptr[cos_x2]
        ftst; cos(x/2)==0 ?
        fstsw word ptr[status]
        mov ah, byte ptr[status + 1]
        sahf
        je err_done
        ; t3 = |cos(x/2)|^(-2/3)
        finit
        fld qword ptr[cos_x2]
        fabs
        fstp qword ptr[cx_abs]
        push 3
        push 2
        fild dword ptr [esp]
        fild dword ptr [esp+4]
        fdivrp st(1), st
        fchs
        add esp, 8
        fld qword ptr[cx_abs]
        fyl2x
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[t3]

        ; t4 = e^sin(x)
        finit
        fld qword ptr[x]
        fsin
        fldl2e
        fmulp st(1), st
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[t4]

        ; t5 = |sin(x/2)|^(2/3)
        finit
        fld qword ptr[x2]
        fsin
        fabs
        fstp qword ptr[cx_abs]
        push 3
        push 2
        fild dword ptr [esp]
        fild dword ptr [esp+4]
        fdivrp st(1), st
        add esp, 8
        fld qword ptr[cx_abs]
        fyl2x
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[t5]

        ; t6 = (1/2)*ln(x/2)
        finit
        fld qword ptr[x]
        fld qword ptr[dHalf]
        fmulp st(1), st
        fldln2
        fxch st(1)
        fyl2x
        fld qword ptr[dHalf]
        fmulp st(1), st
        fstp qword ptr[t6]

        ; tg(x/2)
        finit
        fld qword ptr[x2]
        fptan
        fstp st(0)
        fstp qword ptr[cx_abs]
        fld qword ptr[cx_abs]
        ftst
        fstsw word ptr[status]
        mov ah, byte ptr[status + 1]
        sahf
        jp err_done
        ; t7 = |tg(x/2)|^(2/3)
        finit
        fld qword ptr[cx_abs]
        fabs
        fstp qword ptr[cx_abs]
        push 3
        push 2
        fild dword ptr [esp]
        fild dword ptr [esp+4]
        fdivrp st(1), st
        add esp, 8
        fld qword ptr[cx_abs]
        fyl2x
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[t7]

        finit
        fld qword ptr[t1]
        fadd qword ptr[t2]
        fadd qword ptr[t3]
        fadd qword ptr[t4]
        fadd qword ptr[t5]
        fadd qword ptr[t6]
        fsub qword ptr[t7]
        fstp qword ptr[inner]
        fld qword ptr[inner]
        ftst; < 0 ?
        fstsw word ptr[status]
        mov ah, byte ptr[status + 1]
        sahf
        jb err_done
        ; acc += sqrt(inner)
        finit
        fld qword ptr[inner]
        fsqrt
        fadd qword ptr[acc]
        fstp qword ptr[acc]

        ; + sin(y^(-2/3))
        finit
        push 3
        push 2
        fild dword ptr [esp]
        fild dword ptr [esp+4]
        fdivrp st(1), st
        fchs
        add esp, 8
        fld qword ptr[y]
        fyl2x
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[y_neg2_3]
        fld qword ptr[y_neg2_3]
        fsin
        fadd qword ptr[acc]
        fstp qword ptr[acc]

        ; + 5 * e^(-2y/3)
        finit
        push 3
        push 2
        fild dword ptr [esp]
        fild dword ptr [esp+4]
        fdivrp st(1), st
        fchs
        add esp, 8
        fld qword ptr[y]
        fmulp st(1), st
        fldl2e
        fmulp st(1), st
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fld qword ptr[d5]
        fmulp st(1), st
        fadd qword ptr[acc]
        fstp qword ptr[acc]

        ; yp = y^(2/3); acc -= cos(yp)
        finit
        fld qword ptr[y]
        fabs
        fstp qword ptr[ay]
        push 3
        push 2
        fild dword ptr [esp]
        fild dword ptr [esp+4]
        fdivrp st(1), st
        add esp, 8
        fld qword ptr[ay]
        fyl2x
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fstp qword ptr[yp]
        finit
        fld qword ptr[y]
        fldz
        fcompp; y < 0 -> yp = -|y|^(2/3)
        fnstsw ax
        sahf
        jae short yp_ok
        finit
        fld qword ptr[yp]
        fchs
        fstp qword ptr[yp]
    yp_ok:
        finit
        fld qword ptr[acc]
        fld qword ptr[yp]
        fcos
        fsubp st(1), st
        fstp qword ptr[acc]

        ; + e^cos(y)
        finit
        fld qword ptr[y]
        fcos
        fldl2e
        fmulp st(1), st
        fld st
        frndint
        fsub st(1), st
        fxch st(1)
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        fadd qword ptr[acc]
        fstp qword ptr[acc]

        ; + 1/sin(y) = cosec(y)
        finit
        fld1
        fld qword ptr[sin_y]
        fdivp st(1), st
        fadd qword ptr[acc]
        fstp qword ptr[acc]

        fld qword ptr[acc]
        fstp qword ptr[res]
        jmp asm_done

    err_done:
        mov error, 1
        finit
        fldz
        fstp qword ptr[res]
    asm_done:
    }

    if (error)
        return std::numeric_limits<double>::quiet_NaN();
    if (!finiteVal(res))
        return std::numeric_limits<double>::quiet_NaN();
    return res;
}

void printHeader() {
    std::setlocale(LC_ALL, "ru_RU");
    std::cout << "\033[33m";
    std::cout << "     Лабораторная работа ";
    std::cout << "\033[1;31m05\033[33m\n";
    std::cout << "\033[0m";

    std::cout << "Группа: ";
    std::cout << "\033[36m6102-020302D\033[0m\n";

    std::cout << "Студент: ";
    std::cout << "\033[32mДудкин Егор Денисович\033[0m\n";

    std::cout << "Вариант задания: ";
    std::cout << "\033[1;31m43\033[0m\n\n";

    std::cout << "\033[90m------------------------------------------------------------\033[0m\n";
    std::cout << "\033[1mЗадание\033[0m\n\n";
    std::cout << "  f(x,y) = x^(43/44)\n";
    std::cout << "         + sqrt( (1/2)^(-x/4) + x^(x/5) + |cos(x/2)|^(-2/3) + e^sin(x)\n";
    std::cout << "         + |sin(x/2)|^(2/3) + (1/2)ln(x/2) - |tg(x/2)|^(2/3) )\n";
    std::cout << "         + sin(y^(-2/3)) + 5*e^(-2y/3) - cos(y^(2/3)) + e^cos(y) + cosec(y)\n";
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

static constexpr std::size_t kCellMinWidth = 17;

static std::string dashCell() {
    std::string s(1, '-');
    s.insert(0, (kCellMinWidth - 1) / 2, ' ');
    while (s.size() < kCellMinWidth)
        s += ' ';
    return s;
}

static std::string padCell(std::string s) {
    if (s.size() < kCellMinWidth)
        s.append(kCellMinWidth - s.size(), ' ');
    return s;
}

std::string cellCpp(double x, double y) {
    double o;
    if (!calcCpp(x, y, o))
        return dashCell();
    return padCell(std::format("{:.15f}", o));
}

std::string cellAsm(double x, double y) {
    double a = calcAsm(x, y);
    if (!finiteVal(a))
        return dashCell();
    return padCell(std::format("{:.15f}", a));
}

static void printTableBorderLine(int wn, int wcell,
                                 char corner, char fill, char junction) {
    std::cout << corner;
    for (int i = 0; i < wn + 2; ++i) std::cout << fill;
    for (int col = 0; col < 4; ++col) {
        std::cout << junction;
        for (int i = 0; i < wcell + 2; ++i) std::cout << fill;
    }
    std::cout << corner << '\n';
}

void printTable(double x0, double x1, double y0, double y1, double hx, double hy) {
    const int wn = 4;
    const int wcell = static_cast<int>(kCellMinWidth);

    std::cout << std::fixed;
    std::cout.precision(15);
    std::cout << "\nРезультат:\n";
    printTableBorderLine(wn, wcell, '+', '-', '+');
    std::cout << "| " << std::setw(wn) << "№" << " | " << std::setw(wcell) << "x" << " | " << std::setw(wcell) << "y"
              << " | " << std::setw(wcell) << "C++" << " | " << std::setw(wcell) << "ASM" << " |\n";
    printTableBorderLine(wn, wcell, '+', '-', '+');

    int n = 0;
    for (double x = x0; x <= x1 + 1e-12; x += hx) {
        for (double y = y0; y <= y1 + 1e-12; y += hy) {
            ++n;
            std::cout << "| " << std::setw(wn) << n << " | " << std::setw(wcell) << x << " | " << std::setw(wcell) << y
                      << " | " << cellCpp(x, y) << " | " << cellAsm(x, y) << " |\n";
        }
    }
    printTableBorderLine(wn, wcell, '+', '-', '+');
}

int main() {
    std::cout.precision(15);
    std::cout << std::fixed;

    printHeader();

    double x0, x1, y0, y1, hx, hy;
    readDouble(x0, "X0 (начало [X0; X1]) = ");
    readDouble(x1, "X1 (конец  [X0; X1]) = ");
    readDouble(y0, "Y0 (начало [Y0; Y1]) = ");
    readDouble(y1, "Y1 (конец  [Y0; Y1]) = ");
    readDouble(hx, "Шаг по X = ");
    readDouble(hy, "Шаг по Y = ");

    if (hx <= 0 || hy <= 0 || x1 < x0 || y1 < y0) {
        std::cout << "Некорректное значение.\n";
        return 1;
    }

    printTable(x0, x1, y0, y1, hx, hy);
    std::cout << '\n';
    system("pause");
    return 0;
}
