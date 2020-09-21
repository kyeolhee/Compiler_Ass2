#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

#define NUMBER 256
#define PLUS 257
#define STAR 258
#define LPAREN 259
#define RPAREN 260
#define END 261
#define EXPRESSION 0
#define TERM 1
#define FACTOR 2
#define ACC 999

int action[12][6] = {
	{5, 0, 0, 4, 0, 0}, {0, 6, 0, 0, 0, ACC}, {0, -2, 7, 0, -2, -2},
	{0, -4, -4, 0, -4, -4}, {5,0, 0, 4, 0, 0}, {0, -6, -6, 0, -6, -6},
	{5, 0, 0, 4, 0, 0}, {5, 0, 0, 4, 0, 0}, {0, 6, 0, 0, 11, 0},
	{0, -1,7, 0, -1, -1}, {0, -3, -3, 0, -3, -3}, {0, -5, -5, 0, -5, -5} };

int go_to[12][3] = {
	{1, 2, 3}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {8, 2, 3}, {0, 0, 0},
	{0, 9, 3}, {0, 0, 10}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };

int prod_left[7] = { 0, EXPRESSION, EXPRESSION, TERM, TERM, FACTOR, FACTOR };
int prod_length[7] = { 0, 3, 1, 3, 1, 3, 1 };

typedef struct _Type {
	// DataType = 0 is int, DataType = 1 is float;
	int DataType;
	int i_num;
	float f_num;
}Type;

int stack[1000];
Type value[1000];
int top = -1;
int sym;
char yytext[32];
Type yyIval;
// time = Position of the expression
int time = 0;
// warning = true is int+float , warning = false is only int or float
bool warning = false;

void push(int);
void reduce(int);
void shift(int i);
void yyerror();
void lex_error();
int yyparse();
int yylex();

int main() {
	printf("Input : ");
	yyparse();
	return 0;
}

int yyparse() {
	int i;
	stack[++top] = 0;
	sym = yylex();
	do {
		i = action[stack[top]][sym - 256];
		if (i == ACC) {
			if (value[top].DataType == 0)
				printf("Result :  %d\n ", value[top].i_num);
			else
				printf("Result :  %f\n ", value[top].f_num);

			if (warning == true)
				printf("It is a mixture of int and float. (at %d)\n", time);
		}
		else if (i > 0)
			shift(i);
		else if (i < 0)
			reduce(-i);
		else
			yyerror();
	} while (i != ACC);
}

int yylex() {
	static char ch = ' ';
	int i = 0;

	for (i = 0; i < 32; i++) {
		yytext[i] = ' ';
	}

	i = 0;
	yyIval.DataType = 0;
	yyIval.i_num = 0;
	yyIval.f_num = 0;
	while (ch == ' ' || ch == '\t')
		ch = getchar();
	if (isdigit(ch)) {
		do {
			yytext[i++] = ch;
			ch = getchar();
		} while (isdigit(ch));
		if (ch == '.') {
			do {
				yytext[i++] = ch;
				ch = getchar();
			} while (isdigit(ch));
			yyIval.DataType = 1;
			yyIval.f_num = atof(yytext);
			if (warning == false)
				time++;
			return (NUMBER);
		}
		yyIval.DataType = 0;
		yyIval.i_num = atoi(yytext);
		if (warning == false)
			time++;
		return (NUMBER);
	}
	else if (ch == '+') {
		ch = getchar();
		return(PLUS);
	}
	else if (ch == '*') {
		ch = getchar();
		return(STAR);
	}
	else if (ch == '(') {
		ch = getchar();
		return(LPAREN);
	}
	else if (ch == ')') {
		ch = getchar();
		return(RPAREN);
	}
	else if (ch == '\n') {
		return(END);
	}
	else
		lex_error();
}

void push(int i) {
	top++;
	stack[top] = i;
}

void shift(int i) {
	push(i);
	value[top] = yyIval;
	sym = yylex();
}

void reduce(int i) {
	int old_top;
	top -= prod_length[i];
	old_top = top;
	push(go_to[stack[old_top]][prod_left[i]]);
	switch (i) {
	case 1:
		if (value[old_top + 1].DataType == 0) {
			if (value[old_top + 3].DataType == 0) {
				value[top].DataType = 0;
				value[top].i_num = value[old_top + 1].i_num + value[old_top + 3].i_num;
			}
			else {
				warning = true;
				value[top].DataType = 1;
				value[top].f_num = value[old_top + 1].i_num + value[old_top + 3].f_num;
			}
		}
		else {
			if (value[old_top + 3].DataType == 1) {
				value[top].DataType = 1;
				value[top].f_num = value[old_top + 1].f_num + value[old_top + 3].f_num;
			}
			else {
				warning = true;
				value[top].DataType = 1;
				value[top].f_num = value[old_top + 1].f_num + value[old_top + 3].i_num;
			}
		}
		break;
	case 2: value[top] = value[old_top + 1];
		break;
	case 3:
		if (value[old_top + 1].DataType == 0) {
			if (value[old_top + 3].DataType == 0) {
				value[top].DataType = 0;
				value[top].i_num = value[old_top + 1].i_num * value[old_top + 3].i_num;
			}
			else {
				warning = true;
				value[top].DataType = 1;
				value[top].f_num = value[old_top + 1].i_num * value[old_top + 3].f_num;
			}
		}
		else {
			if (value[old_top + 3].DataType == 1) {
				value[top].DataType = 1;
				value[top].f_num = value[old_top + 1].f_num * value[old_top + 3].f_num;
			}
			else {
				warning = true;
				value[top].DataType = 1;
				value[top].f_num = value[old_top + 1].f_num * value[old_top + 3].i_num;
			}
		}
		break;
	case 4: value[top] = value[old_top + 1];
		break;
	case 5: value[top] = value[old_top + 2];
		break;
	case 6: value[top] = value[old_top + 1];
		break;
	default: yyerror("parsing table error");
		break;
	}
}

void yyerror() {
	printf("syntax error\n");
	exit(1);
}

void lex_error() {
	printf("illegal token\n");
	exit(1);
}
