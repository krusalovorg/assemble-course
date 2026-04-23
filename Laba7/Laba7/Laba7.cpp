#include <iostream>
#include <iomanip>
#include <cmath>
#include <windows.h>

using namespace std;

void f_cpp(double x, double eps, int N, double& sum_out, int& iterations_out, double& delta_out, int& reason_out) {
    if (N <= 0) {
        sum_out = 0.0;
        iterations_out = 0;
        delta_out = 0.0;
        reason_out = 1;
        return;
    }

    const double x_plus_10 = x + 10.0;
    double term = x_plus_10; // n=1
    double sum = term;
    double prevSum = 0.0;
    int iterations = 1;
    int reason = 1;

    if (fabs(sum - prevSum) <= eps) {
        reason = 0;
    } else {
        for (int n = 1; n < N; ++n) {
            const double ratio = static_cast<double>(n) / (n + 1);
            const double power = pow(ratio, n);
            term = term * x_plus_10 * power; // переход от n к n+1
            prevSum = sum;
            sum += term;
            ++iterations;

            if (fabs(sum - prevSum) <= eps) {
                reason = 0;
                break;
            }
        }
    }

    sum_out = sum;
    iterations_out = iterations;
    delta_out = fabs(sum - prevSum);
    reason_out = reason;
}

// ASM-версия: вычисление ряда, передача x, eps, N как аргументов.
void f_asm(double x, double eps, int N, double& sum_out, int& iterations_out, double& delta_out, int& reason_out) {
    if (N <= 0) {
        sum_out = 0.0;
        iterations_out = 0;
        delta_out = 0.0;
        reason_out = 1;
        return;
    }

    double x_plus_10 = x + 10.0;
    double term = x_plus_10;   // a1
    double sum = term;         // S1
    double prevSum = 0.0;      // S0
    double ratio = 0.0;
    int n_local = 0;
    int iterations = 1;
    int reason = 1;

    if (fabs(sum - prevSum) <= eps) {
        reason = 0;
    } else {
        for (int n = 1; n < N; ++n) {
            n_local = n;

            __asm {
                finit

                // st(0) = n
                fild dword ptr [n_local]

                // st(0) = n+1, st(1) = n
                mov eax, n_local
                inc eax
                mov n_local, eax
                fild dword ptr [n_local]

                // st(0) = n/(n+1)
                fdivp st(1), st
                fstp ratio
            }

            const double power = pow(ratio, n);
            term = term * x_plus_10 * power;
            prevSum = sum;
            sum += term;
            ++iterations;

            if (fabs(sum - prevSum) <= eps) {
                reason = 0;
                break;
            }
        }
    }

    sum_out = sum;
    iterations_out = iterations;
    delta_out = fabs(sum - prevSum);
    reason_out = reason;
}

double control_value(double x) {
    const double x_plus_10 = x + 10.0;
    if (fabs(x_plus_10) >= 2.718281828459045) {
        return NAN;
    }

    double sum = 0.0;
    int iterations = 0;
    double delta = 0.0;
    int reason = 1;
    f_cpp(x, 1e-15, 2000, sum, iterations, delta, reason);
    return sum;
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    cout << "================================================================================\n";
    cout << "Лабораторная работа №7\n";
    cout << "Вариант 49\n";
    cout << "================================================================================\n\n";

    cout << "Исследуемый ряд:\n";
    cout << "∞\n";
    cout << "---\n";
    cout << "\\         n!        n\n";
    cout << " >     ------ * (x+10)\n";
    cout << "/          n\n";
    cout << "---       n\n";
    cout << "n=1\n\n";

    cout << "Рекуррентная формула: a(n+1) = a(n) * (x+10) * (n/(n+1))^n\n";
    cout << "Радиус сходимости: R = e ≈ 2.71828\n";
    cout << "Условие сходимости: |x+10| < e\n";
    cout << "Условие остановки: |S(k+1)-S(k)| <= eps, либо k >= N\n\n";

    double x = 0.0;
    double eps = 0.0;
    int N = 0;

    cout << "Введите x: ";
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Ошибка! Введите число: ";
    }

    cout << "Введите eps (>0): ";
    while (!(cin >> eps) || eps <= 0.0) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Ошибка! Введите положительное eps: ";
    }

    cout << "Введите ограничение по числу членов N (>0): ";
    while (!(cin >> N) || N <= 0) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Ошибка! Введите целое число N > 0: ";
    }

    const double x_plus_10 = x + 10.0;
    const bool converges = (fabs(x_plus_10) < 2.718281828459045);
    if (!converges) {
        cout << "\nВНИМАНИЕ: |x+10| = " << fabs(x_plus_10) << " >= e.\n";
        cout << "Ряд расходится, оценка погрешности относительно контрольного значения недоступна.\n\n";
    }

    double cppSum = 0.0;
    int cppIter = 0;
    double cppDelta = 0.0;
    int cppReason = 1;

    double asmSum = 0.0;
    int asmIter = 0;
    double asmDelta = 0.0;
    int asmReason = 1;

    f_cpp(x, eps, N, cppSum, cppIter, cppDelta, cppReason);
    f_asm(x, eps, N, asmSum, asmIter, asmDelta, asmReason);
    double exact = converges ? control_value(x) : NAN;

    cout << "\n================================================================================\n";
    cout << "Результаты вычислений\n";
    cout << "================================================================================\n";
    cout << fixed << setprecision(12);
    cout << "C++: сумма = " << cppSum
         << ", итерации = " << cppIter
         << ", |Sk+1-Sk| = " << cppDelta
         << ", причина остановки = " << (cppReason == 0 ? "eps" : "N") << "\n";
    cout << "ASM: сумма = " << asmSum
         << ", итерации = " << asmIter
         << ", |Sk+1-Sk| = " << asmDelta
         << ", причина остановки = " << (asmReason == 0 ? "eps" : "N") << "\n";

    cout << scientific << setprecision(6);
    cout << "|C++ - ASM| = " << fabs(cppSum - asmSum) << "\n";

    if (converges) {
        cout << "Контрольное значение (высокоточная сумма ряда): " << fixed << setprecision(12) << exact << "\n";
        cout << scientific << setprecision(6);
        cout << "|Control - C++| = " << fabs(exact - cppSum) << "\n";
        cout << "|Control - ASM| = " << fabs(exact - asmSum) << "\n";
    } else {
        cout << "Контрольная погрешность не выводится (ряд расходится).\n";
    }

    cout << "================================================================================\n";

    system("pause");
    return 0;
}