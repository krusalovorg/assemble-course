#include <iostream>
#include <string>

void printMenu() {
    setlocale(LC_ALL, "ru-RU");
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
        je error; если tg(a) == 0, то ошибка

        fld1
        fdivrp st(1), st; 1/tg(a) | ...
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
        je error; если denom == 0, то ошибка


        ; (42 * b + a)/(42 * a - b)
        fdivp st(1), st; (42 * b + a) / (42 * a - b) | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)

        ; res
        fldpi; pi | (42 * b + a) / (42 * a - b) | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fmulp st(1), st; pi * (42 * b + a) / (42 * a - b) | 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a)
        fsubp st(1), st; 42 * pi - sin(b) - cos(b) - tg(a) - ctg(a) - pi * (42 * b + a) / (42 * a - b)

        fstp qword ptr[res];
        jmp done
    error:
        fldz;res=0.0
        fstp qword ptr[res]
    done:
    }
    return res;
}

int main()
{
    printMenu();

    double a, b;
    readDouble(a, "a");
    readDouble(b, "b");


    std::cout << "Результат выполенения функции x = " << calc(a,b) << std::endl;

    system("pause");
}