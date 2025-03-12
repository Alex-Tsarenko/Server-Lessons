#include <iostream>
#include <stack>
#include <string>
#include <cctype>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cassert>

using namespace std;

// Функция для определения приоритета операторов
int getPrecedence(char op) {
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/')
        return 2;
    if (op == '^')
        return 3;
    return 0;
}

// Функция для проверки, является ли символ оператором
bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

// Структура для хранения информации о функции
struct FunctionInfo {
    string name;
    int argCount;
};

// Реализация алгоритма сортировочной станции с улучшенной поддержкой функций
string shuntingYard(const string& infix) {
    string output;
    stack<char> operatorStack;
    stack<FunctionInfo> functionStack;
    int argCount = 0;

    for (size_t i = 0; i < infix.length(); i++) {
        char c = infix[i];

        // Пропускаем пробелы
        if (isspace(c))
            continue;

        // Обработка чисел
        if (isdigit(c)) {
            string number;
            while (i < infix.length() && isdigit(infix[i])) {
                number += infix[i];
                i++;
            }
            i--; // Возвращаемся на один символ назад
            output += number + " ";
        }
        // Обработка идентификаторов (переменных и функций)
        else if (isalpha(c))
        {
            //
            // extract identifier
            //
            string identifier;
            while (i < infix.length() && (isalnum(infix[i]) || infix[i] == '_'))
            {
                identifier += infix[i];
                i++;
            }
            i--; // Возвращаемся на один символ назад

            //
            // Проверяем, является ли идентификатор функцией
            //
            size_t j = i + 1;
            while (j < infix.length() && isspace(infix[j])) j++; // Пропускаем пробелы

            if (j < infix.length() && infix[j] == '(')
            {
                // Это функция
                FunctionInfo func{identifier, 0};
                functionStack.push(func);
                operatorStack.push('{'); // Добавляем скобку в стек операторов
                i = j; // Пропускаем открывающую скобку
            }
            else
            {
                // Это переменная
                output += identifier + " ";
            }
        }
        // Обработка запятой (разделитель аргументов функции)
        else if (c == ',')
        {
            // Выталкиваем операторы до открывающей скобки текущего аргумента
            while ( !operatorStack.empty() && (operatorStack.top() != '(' && operatorStack.top() != '{') )
            {
                output += operatorStack.top();
                output += " ";
                operatorStack.pop();
            }

            // Увеличиваем счетчик аргументов текущей функции верхнего уровня
            if ( functionStack.empty() )
            {
                cout << "Error: ','";
                exit(0);
            }
            else
            {
                functionStack.top().argCount++;
            }
        }
        // Обработка закрывающей скобки
        else if (c == ')')
        {
            // Выталкиваем операторы до открывающей скобки
            while ( !operatorStack.empty() && (operatorStack.top() != '(' && operatorStack.top() != '{') )
            {
                output += operatorStack.top();
                output += " ";
                operatorStack.pop();
            }

            // Удаляем открывающую скобку из стека
            if( operatorStack.empty() )
            {
                cout << "Error: extra ')'";
                exit(0);
            }

            bool itIsFunc = ( operatorStack.top() == '{' );

            // pop '(/{'
            operatorStack.pop();

            if ( itIsFunc && functionStack.empty() )
            {
                cout << "Error: internal error '{'";
                exit(0);
            }

            // Проверяем, связана ли эта скобка с вызовом функции
            if ( itIsFunc )
            {
                FunctionInfo func = functionStack.top();
                functionStack.pop();

                // Если были аргументы (отмечены запятыми), увеличиваем счетчик
                if (func.argCount > 0)
                {
                    func.argCount++; // Последний аргумент после последней запятой
                }
                else
                {
                    // Проверяем, был ли один аргумент без запятых
                    bool emptyParens = true;

                    for (size_t k = i - 1; k > 0; k--)
                    {
                        if (infix[k] == '(') {
                            emptyParens = true;
                            break;
                        }
                        if ( !isspace(infix[k]) )
                        {
                            emptyParens = false;
                            break;
                        }
                    }
                    func.argCount = emptyParens ? 0 : 1;
                }

                output += func.name + " FUNC_" + to_string(func.argCount) + " ";
            }
        }
        // Обработка открывающей скобки
        else if (c == '(') {
            operatorStack.push(c);
        }
        // Обработка операторов
        else if (isOperator(c)) {
            // Правильная обработка приоритетов операторов
            while (!operatorStack.empty() && operatorStack.top() != '(' &&
                  (getPrecedence(operatorStack.top()) > getPrecedence(c) ||
                   (getPrecedence(operatorStack.top()) == getPrecedence(c) && c != '^'))) {
                output += operatorStack.top();
                output += " ";
                operatorStack.pop();
            }
            operatorStack.push(c);
        }
    }

    // Выталкиваем все оставшиеся операторы из стека
    while (!operatorStack.empty()) {
        if (operatorStack.top() != '(') {
            output += operatorStack.top();
            output += " ";
        }
        operatorStack.pop();
    }

    // Удаляем последний пробел, если он есть
    if (!output.empty() && output.back() == ' ')
        output.pop_back();

    return output;
}

// Определим нужный приоритет скобок для преобразования
int needParentheses(const string& expr, char op) {
    if (expr.length() < 2) return 0;

    // Если выражение уже в скобках, дополнительные не нужны
    if (expr[0] == '(' && expr[expr.length() - 1] == ')') return 0;

    // Ищем оператор верхнего уровня в выражении
    int depth = 0;
    char topOp = 0;

    for (size_t i = 0; i < expr.length(); i++) {
        if (expr[i] == '(') depth++;
        else if (expr[i] == ')') depth--;
        else if (depth == 0 && isOperator(expr[i])) {
            topOp = expr[i];
            break;
        }
    }

    // Если оператор не найден, скобки не нужны
    if (topOp == 0) return 0;

    // Сравниваем приоритеты
    return getPrecedence(topOp) < getPrecedence(op);
}

string postfixToInfix(const string& postfix) {
    vector<string> tokens;
    stringstream ss(postfix);
    string token;

    // Сначала разбиваем строку на токены
    while (ss >> token) {
        tokens.push_back(token);
    }

    stack<string> stack;

    for (const string& token : tokens) {
        // Если это обычный оператор
        if (token.length() == 1 && isOperator(token[0])) {
            if (stack.size() < 2) {
                return "Ошибка: недостаточно операндов для оператора " + token;
            }

            string right = stack.top(); stack.pop();
            string left = stack.top(); stack.pop();

            // Добавляем скобки только при необходимости
            bool needLeftParen = needParentheses(left, token[0]);
            bool needRightParen = needParentheses(right, token[0]);

            if (needLeftParen) {
                left = "(" + left + ")";
            }

            if (needRightParen) {
                right = "(" + right + ")";
            }

            stack.push(left + " " + token + " " + right);
        }
        // Если это вызов функции
        else if (token.find("FUNC_") != string::npos) {
            int numArgs = stoi(token.substr(token.find("_") + 1));

            // Проверяем, что в стеке достаточно элементов
            if (stack.size() < numArgs + 1) {
                return "Ошибка: недостаточно аргументов для функции";
            }

            // Сначала извлекаем аргументы в обратном порядке
            vector<string> args;
            for (int i = 0; i < numArgs; i++) {
                args.push_back(stack.top());
                stack.pop();
            }

            // Затем получаем имя функции
            string funcName = stack.top();
            stack.pop();

            // Разворачиваем массив аргументов, так как мы их извлекали в обратном порядке
            reverse(args.begin(), args.end());

            // Формируем корректную строку вызова
            string call = funcName + "(";
            for (size_t i = 0; i < args.size(); i++) {
                if (i > 0) call += ", ";
                call += args[i];
            }
            call += ")";

            stack.push(call);
        }
        // Операнд (переменная или число)
        else {
            stack.push(token);
        }
    }

    if (stack.size() != 1) {
        return "Ошибка: некорректное выражение (осталось " + to_string(stack.size()) + " элементов в стеке)";
    }

    return stack.top();
}

// Функция для преобразования постфиксной записи обратно в инфиксную
string postfixToInfix2(const string& postfix) {
    vector<string> tokens;
    stringstream ss(postfix);
    string token;

    // Сначала разбиваем строку на токены
    while (ss >> token) {
        tokens.push_back(token);
    }

    stack<string> stack;

    for (const string& token : tokens) {
        // Если это обычный оператор
        if (token.length() == 1 && isOperator(token[0])) {
            if (stack.size() < 2) {
                return "Ошибка: недостаточно операндов для оператора " + token;
            }

            string right = stack.top(); stack.pop();
            string left = stack.top(); stack.pop();

            // Добавляем скобки только при необходимости
            if (needParentheses(left, token[0])) {
                left = "(" + left + ")";
            }

            if (needParentheses(right, token[0])) {
                right = "(" + right + ")";
            }

            stack.push(left + " " + token + " " + right);
        }
        // Если это вызов функции
        else if (token.substr(0, 5) == "FUNC_") {
            int numArgs = stoi(token.substr(5));

            if (stack.size() < numArgs + 1) {
                return "Ошибка: недостаточно аргументов для функции";
            }

            // Получаем имя функции
            string funcName = stack.top();
            stack.pop();

            vector<string> args;
            for (int i = 0; i < numArgs; i++) {
                args.insert(args.begin(), stack.top()); // Вставляем в начало
                stack.pop();
            }

            // Формируем корректную строку вызова
            string call = funcName + "(" + (args.empty() ? "" : args[0]);
            for (size_t i = 1; i < args.size(); i++) {
                call += ", " + args[i];
            }
            call += ")";

            stack.push(call);
        }
        // Операнд (переменная или число)
        else {
            stack.push(token);
        }
    }

    if (stack.size() != 1) {
        return "Ошибка: некорректное выражение (осталось " + to_string(stack.size()) + " элементов в стеке)";
    }

    return stack.top();
}

int main() {
    string infix = "10 + f(4,a*(8+3),x-f12()) + 11";
    cout << infix << endl;

    string postfix = shuntingYard(infix);
    cout << "Постфиксная запись: " << postfix << endl;

    string infix2 = postfixToInfix(postfix);
    cout << "Обратное преобразование: " << infix2 << endl;

    return 0;
}


