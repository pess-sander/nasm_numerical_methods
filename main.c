#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include <string.h>

// Максимальное кол-во операций, которое будет выполняться
// в цикле поиска корня до аварийной остановки
#define MAX_STEPS 1000000

// Функции описаны в funcs.asm
extern double f1(double x);
extern double f2(double x);
extern double f3(double x);

// Количество итераций, необходимое для вычисления корня
int iter = 0;

double root(double (*f)(double x), double (*g)(double x), double a, double b, double eps1) {
	// Условие: ищем изолированный корень на отрезке [a, b]
	// phi(x) = f(x) - g(x)
	double phi_a = (*f)(a) - (*g)(a);
	double phi_b = (*f)(b) - (*g)(b);
	// Корень находится в концах отрезка
	if (phi_a == 0) return a;
	if (phi_b == 0) return b;
	// Уравнение не соответствует условию
	if (phi_a*phi_b > 0) {
		printf("root: Корень уравнения f(x)=g(x) на отрезке [%.2lf, %.2lf] не существует или не изолирован.\n", a, b);
		return 0.0 / 0.0; // return NaN
	}

	// Счётчик кол-ва итераций
	int k = 0;
	// Начальные значения a и b
	double init_a = a;
	double init_b = b;

	// Выбор метода решения уравнения определяется константой SECANTS
	#ifdef SECANTS
	// Метод хорд
	// Считаем, что функция не имеет точек перегиба на отрезке
	// На каждой итерации будем сдвигать а
	// Считаем абсциссу точки пересечения хорды с осью Ox
	double c = a - (b - a)*phi_a/(phi_b - phi_a);

	// Значении phi в точке c
	double phi = (*f)(c) - (*g)(c);
	// В зависимости от характера выпуклости функции меняем местами а и b
	if (phi*phi_a < 0) {
		double tmp = a;
		a = b;
		b = tmp;

		tmp = phi_a;
		phi_a = phi_b;
		phi_b = tmp;

		c = a - (b - a)*phi_a/(phi_b - phi_a);
	}

	// Пока значение phi достаточно большое
	while (fabs(phi) > eps1/2 && k <= MAX_STEPS) {
		// Сдвигаем а
		a = c;
		phi_a = (*f)(a) - (*g)(a);

		// Обновляем с
		c = a - (b - a)*phi_a/(phi_b - phi_a);
		phi = (*f)(c) - (*g)(c);

		k++;
	}
	#else
	// Метод деления отрезка пополам
	// Середина отрезка
	double c = (b + a) * 0.5;

	// Значение phi в середине отрезка
	double phi = (*f)(c) - (*g)(c);

	while (fabs(phi) > eps1/2 && k <= MAX_STEPS) {
		// Корень находится слева от точки с
		if (phi_a*phi < 0) {
			b = c;
			c = (a + c) * 0.5;
		// Корень находится справа от точки с
		} else {
			a = c;
			c = (c + b) * 0.5;
		}
		
		phi = (*f)(c) - (*g)(c);

		k++;
	}
	#endif

	// Если потребовалось слишком большое кол-во итераций, то
	// задан неоптимальный отрезок
	if (k >= MAX_STEPS) {
		printf("root: Поиск корня уравнения f(x) - g(x) = 0 на отрезке ");
		printf("[%.2lf, %.2lf] занял слишком много времени.\n", init_a, init_b);
		return 0.0/0.0;
	}

	iter = k;
	return c;
}


double integral(double (*f)(double x), double a, double b, double eps2) {
	// Кол-во отрезков разбиения
	int n = 1;
	// Величина шага
	double dx = (b - a);
	// Формула трапеций для двух точек
	double ans = ((*f)(b) - (*f)(a)) * 0.5 * dx;
	// Величина ошибки
	double err = eps2 + 1.0;

	while (err > eps2/2) {
		// Запоминаем прошлое приближение
		double old_ans = ans;

		// Считаем точки в серединах отрезков разбиения
		double middle_points = 0;
		for (int i = 0; i < n; i++)
			middle_points += (*f)(a + dx*i + dx*0.5);
		// Пересчитываем формулу трапеций
		ans = 0.5 * (ans + middle_points*dx);

		// Теперь в два раза больше отрезков разбиения
		n *= 2;
		dx *= 0.5;
		err = fabs(old_ans - ans);
	}

	return ans;
}

double area(int flag_inter, int flag_iter, double eps) {
	double eps1 = eps/10.0;
	// Абсциссы точек пересечения интересуемых функций
	double r13 = root(f1, f3, -3.0, -2.0, eps1);
	int it1 = iter;
	double r12 = root(f1, f2, 1.0, 2.0, eps1);
	int it2 = iter;
	double r23 = root(f2, f3, 0.6, 1.0, eps1);
	int it3 = iter;

	if (flag_inter)
		printf("Абсциссы точек пересечения кривых:\n\tf1 и f2: %.5lf,\n\tf1 и f3: %.5lf,\n\tf2 и f3: %.5lf\n", r12, r13, r23);
	if (flag_iter) {
		// Выводим, каким методом пользуемся
		#ifdef SECANTS
		const char *method_name = "хорд";
		#else
		const char *method_name = "деления отрезка пополам";
		#endif
		printf("Для вычисления абсцисс точек пересечения кривых методом %s потребовалось следующее кол-во итераций:\n\tf1 и f2: %d\n\tf1 и f3: %d\n\tf2 и f3: %d\n", method_name, it2, it1, it3);
	}

	// Значения интегралом ф-ий на нужных отрезках
	double intf1 = integral(f1, r13, r12, eps1);
	double intf2 = integral(f2, r23, r12, eps1);
	double intf3 = integral(f3, r13, r23, eps1);

	double area = intf1 - (intf2 + intf3);
	return area;
}

// Вывод способа использования программы
void print_usage(char *arg) {
	printf("Использование:\n\t%s --help или\n\t%s --mode=area [--epsilon [eps] --vertices --iterations] или\n", arg, arg);
	printf("\t%s --mode=root --first 1|2|3 --second 1|2|3 --left [A] --right [B] --epsilon [eps] или\n", arg);
	printf("\t%s --mode=integral --first 1|2|3 --left [A] --right [B] --epsilon [eps]\n", arg);
}

// Вывод справки
void print_help(char *arg) {
	printf("Данная программа приближённо вычисляет площадь фигуры, ограниченной тремя функциями:\n");
	printf("f1(x) = 2^x + 1,\n");
	printf("f2(x) = x^5,\n");
	printf("f3(x) = (1 - x) / 3.\n");
	printf("Кроме того, программа способна выполнять приближённый поиск корня уравнения методами хорд и деления отрезка пополам, а также приближённо вычислять определённый интеграл методом трапеций.\n");
	printf("Автор: Космынин Иван, 105 группа.\n");
	print_usage(arg);
	printf("Параметры:\n\t--help -H справка;\n\t--mode -M [mode] выбор режима работы программы (area - вычисление площади фигуры, root - поиск корня уравнения first - second = 0 на отрезке [A, B], integral - вычисление определённого интеграла функции first на отрезке [A, B]);\n");
	printf("\t--first -F 1|2|3 выбор первой функции из набора f1, f2, f3;\n");
	printf("\t--second -S 1|2|3 выбор второй функции из набора f1, f2, f3;\n");
	printf("\t--left -A [A] задание левой границы отрезка (вещественное число);\n");
	printf("\t--right -B [B] задание правой границы отрекза (вещественное число);\n");
	printf("\t--epsilon -E [eps] задание точности, с которой происходят вычисления (при вычислении площади изначально считается равной 0.001, но может быть изменена);\n");
	printf("\t--vertices -V вывод абсцисс точек пересечения кривых, заданных функциями;\n");
	printf("\t--iterations -I вывод числа итераций, необходимых для вычисления корней уравнений.\n");

	printf("Примеры запуска программы:\n");
	printf("\t./main_prog -M area -- вычисление площади фигуры со стандартной точностью eps=0.001\n");
	printf("\t./main_prog -M area -E 0.01 -V -I -- вычисление площади фигуры с точностью eps=0.01, выводом абсцисс точек пересечения и кол-ва потребовавшихся итераций в цикле поиска корня\n");
	printf("\t./main_prog -M root -F 1 -S 2 -A -10.0 -B 10.0 -E 0.1 -- вычисление корня уравнения f1(x) - f2(x) = 0 на отрезке [-10, 10] с точностью eps=0.1\n");
	printf("\t./main_prog -M integral -F 3 -A 0.0 -B 20.0 -E 0.0001 -- вычисление определённого интеграла от функции f3(x) на отрезке [0, 20] с точностью eps=0.0001\n");
}

int main(int argc, char **argv) {
	// Для корректного вывода кириллицы в консоль
	setlocale(LC_ALL, ".UTF-8");

	// Для обработки аргументов командной строки используется
	// утилита getopt

	// Программа вызвана без параметров
	if (argc == 1) {
		print_usage(argv[0]);
		return -1;
	}
	
	// Режим работы программы (area - 0, root - 1, integral - 2)
	int mode = -1;
	char *mode_str;
	// Номера используемых функций
	int f_num = -1;
	int g_num = -1;
	// Указатели на используемые функции
	double (*f)(double) = NULL;
	double (*g)(double) = NULL;
	// Границы отрезка
	double a = 0.0;
	double b = 0.0;

	// Если границы отрезка были указаны среди аргументов, флаги поднимутся
	int flag_a = 0;
	int flag_b = 0;

	// Точность, с которой нужно выполнять вычисления
	double eps = 0.001;
	int flag_eps = 0;

	// Флаг вывода абсцисс точек пересечения кривых
	int flag_inter = 0;
	// Флаг вывода кол-ва итераций, потребовавшихся для вычисления корня
	int flag_iter = 0;

	if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-H")) {
		print_help(argv[0]);
		return 0;
	}

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--mode") || !strcmp(argv[i], "-M")) {
			if (i+1 > argc-1 || !(!strcmp(argv[i+1], "area") || !strcmp(argv[i+1], "root") || !strcmp(argv[i+1], "integral"))) {
				printf("Параметр '%s' требует аргумент area|root|integral\n", argv[i]);
				return -1;
			}

			if (!strcmp(argv[i+1], "area")) mode = 0;
			if (!strcmp(argv[i+1], "root")) mode = 1;
			if (!strcmp(argv[i+1], "integral")) mode = 2;
			i++;
		} else if (!strcmp(argv[i], "--first") || !strcmp(argv[i], "-F")) {
			if (i+1 > argc-1 || sscanf(argv[i+1], "%d", &f_num) != 1 || f_num < 1 || f_num > 3) {
				printf("Параметр '%s' требует аргумент 1|2|3\n", argv[i]);
				return -1;
			}

			if (f_num == 1) f = (&f1);
			if (f_num == 2) f = (&f2);
			if (f_num == 3) f = (&f3);
			i++;
		} else if (!strcmp(argv[i], "--second") || !strcmp(argv[i], "-S")) {
			if (i+1 > argc-1 || sscanf(argv[i+1], "%d", &g_num) != 1 || g_num < 1 || g_num > 3) {
				printf("Параметр '%s' требует аргумент 1|2|3\n", argv[i]);
				return -1;
			}

			if (g_num == 1) g = (&f1);
			if (g_num == 2) g = (&f2);
			if (g_num == 3) g = (&f3);
			i++;
		} else if (!strcmp(argv[i], "--left") || !strcmp(argv[i], "-A")) {
			if (i+1 > argc-1 || sscanf(argv[i+1], "%lf", &a) != 1) {
				printf("Параметр '%s' требует вещественный аргумент\n", argv[i]);
				return -1;
			}

			flag_a = 1;
			i++;
		} else if (!strcmp(argv[i], "--right") || !strcmp(argv[i], "-B")) {
			if (i+1 > argc-1 || sscanf(argv[i+1], "%lf", &b) != 1) {
				printf("Параметр '%s' требует вещественный аргумент\n", argv[i]);
				return -1;
			}

			flag_b = 1;
			i++;
		} else if (!strcmp(argv[i], "--epsilon") || !strcmp(argv[i], "-E")) {
			if (i+1 > argc-1 || sscanf(argv[i+1], "%lf", &eps) != 1) {
				printf("Параметр '%s' требует вещественный аргумент\n", argv[i]);
				return -1;
			}

			flag_eps = 1;
			i++;
		} else if (!strcmp(argv[i], "--vertices") || !strcmp(argv[i], "-V")) {
			flag_inter = 1;
		} else if (!strcmp(argv[i], "--iterations") || !strcmp(argv[i], "-I")) {
			flag_iter = 1;
		} else {
			printf("Неизвестный параметр: %s\n", argv[i]);
			return -1;
		}
	}
	
	// Обработка случая некорректного соотношения параметров
	if (mode == -1) {
		printf("Требуется указать параметр --mode\n");
		print_usage(argv[0]);
		return -1;
	}
	
	// Не указаны границы отрезка
	if ((mode == 1 || mode == 2) && (flag_a == 0 || flag_b == 0)) {
		printf("Требуется указать границы отрезка.\n");
		print_usage(argv[0]);
		return -1;
	}
	if ((mode == 1 || mode == 2) && a >= b) {
		printf("Левая точка отрезка должна быть меньше правой.");
		return -1;
	}

	// Не указана первая функция
	if ((mode == 1 || mode == 2) && f == NULL) {
		printf("Требуется указать параметр --first\n");
		print_usage(argv[0]);
		return -1;
	}
	// Не указана вторая функция
	if (mode == 1 && g == NULL) {
		printf("Требуется указать параметр --second\n");
		print_usage(argv[0]);
		return -1;
	}
	// Первая и вторая функции совпадают
	if (f != NULL && f == g) {
		printf("Параметры --first и --second должны различаться\n");
		return -1;
	}

	// Не указана точность
	if ((mode == 1 || mode == 2) && !flag_eps) {
		printf("Требуется указать параметр --epsilon\n");
		return -1;
	}

	// Конец обработки параметров

	// Считаем площадь
	if (mode == 0) {
		printf("Площадь под графиком с точностью eps=%lf равна %lf\n", eps, area(flag_inter, flag_iter, eps));
	} else if (mode == 1) {
		double r = root(f, g, a, b, eps);
		// r == NaN
		if (r != r)
			return -1;
		else
			printf("Корень уравнения f%d - f%d = 0 на отрезке [%.2lf, %.2lf] с точностью eps=%lf равен %lf\n", f_num, g_num, a, b, eps, r);

	} else if (mode == 2) {
		double i = integral(f, a, b, eps);
		// i == NaN
		if (i != i)
			return -1;
		else
			printf("Значение определённого интеграла от f%d на отрезке [%.2lf, %.2lf] с точность eps=%lf равно %lf\n", f_num, a, b, eps, i);
	}

	return 0;
}
