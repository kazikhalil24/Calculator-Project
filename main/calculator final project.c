#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define ID_EDIT 1
#define ID_BTN_BASE 100

char expression[512] = "";
HWND hEdit;

typedef struct
{
    double nums[256];
    int top;
} NumStack;

typedef struct
{
    char ops[256];
    int top;
} OpStack;

void push_num(NumStack *s, double val)
{
    s->nums[++s->top] = val;
}
double pop_num(NumStack *s)
{
    return s->nums[s->top--];
}

void push_op(OpStack *s, char val)
{
    s->ops[++s->top] = val;
}
char pop_op(OpStack *s)
{
    return s->ops[s->top--];
}
char peek_op(OpStack *s)
{
    return s->ops[s->top];
}

int precedence(char op)
{
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

double apply_op(double a, double b, char op)
{
    switch (op)
    {
    case '+':
        return a + b;
    case '-':
        return a - b;
    case '*':
        return a * b;
    case '/':
        return (b != 0) ? a / b : 0;
    }
    return 0;
}

double evaluate(const char *expr)
{
    NumStack nums = {.top = -1};
    OpStack ops = {.top = -1};

    int i = 0;
    while (expr[i])
    {
        if (isspace(expr[i]))
        {
            i++;
            continue;
        }

        if (isdigit(expr[i]) || expr[i] == '.')
        {
            char numStr[64];
            int j = 0;
            while (isdigit(expr[i]) || expr[i] == '.')
            {
                numStr[j++] = expr[i++];
            }
            numStr[j] = '\0';
            push_num(&nums, atof(numStr));
            continue;
        }

        if (expr[i] == '(')
        {
            push_op(&ops, expr[i]);
        }
        else if (expr[i] == ')')
        {
            while (ops.top != -1 && peek_op(&ops) != '(')
            {
                double b = pop_num(&nums);
                double a = pop_num(&nums);
                char op = pop_op(&ops);
                push_num(&nums, apply_op(a, b, op));
            }
            pop_op(&ops);
        }
        else if (strchr("+-*/", expr[i]))
        {
            while (ops.top != -1 && precedence(peek_op(&ops)) >= precedence(expr[i]))
            {
                double b = pop_num(&nums);
                double a = pop_num(&nums);
                char op = pop_op(&ops);
                push_num(&nums, apply_op(a, b, op));
            }
            push_op(&ops, expr[i]);
        }
        i++;
    }

    while (ops.top != -1)
    {
        double b = pop_num(&nums);
        double a = pop_num(&nums);
        char op = pop_op(&ops);
        push_num(&nums, apply_op(a, b, op));
    }

    return nums.top >= 0 ? nums.nums[nums.top] : 0;
}

void appendToExpression(const char *text)
{
    strcat(expression, text);
    SetWindowText(hEdit, expression);
}

void clearExpression()
{
    expression[0] = '\0';
    SetWindowText(hEdit, "");
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        hEdit = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
                             10, 10, 260, 30, hwnd, (HMENU)ID_EDIT, NULL, NULL);

        const char *labels[] =
        {
            "7", "8", "9", "/",
            "4", "5", "6", "*",
            "1", "2", "3", "-",
            "0", ".", "(", ")",
            "C", "=", "+"
        };

        int x = 10, y = 50, w = 60, h = 40;
        for (int i = 0; i < 19; i++)
        {
            CreateWindow("BUTTON", labels[i], WS_CHILD | WS_VISIBLE,
                         x, y, w, h, hwnd, (HMENU)(ID_BTN_BASE + i), NULL, NULL);
            x += w + 5;
            if ((i + 1) % 4 == 0)
            {
                x = 10;
                y += h + 5;
            }
        }
        break;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        int index = id - ID_BTN_BASE;

        const char *labels[] =
        {
            "7", "8", "9", "/",
            "4", "5", "6", "*",
            "1", "2", "3", "-",
            "0", ".", "(", ")",
            "C", "=", "+"
        };

        if (index >= 0 && index < sizeof(labels)/sizeof(labels[0]))
        {
            const char *input = labels[index];

            if (strcmp(input, "C") == 0)
            {
                clearExpression();
            }
            else if (strcmp(input, "=") == 0)
            {
                double result = evaluate(expression);
                sprintf(expression, "%.10g", result);
                SetWindowText(hEdit, expression);
            }
            else
            {
                appendToExpression(input);
            }
        }
        break;
    }


    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "CalculatorWindow";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(CLASS_NAME, "Student Calculator",
                             WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, 300, 360,
                             NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
