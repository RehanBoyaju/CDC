#include <iostream>
#include <cctype>
using namespace std;

bool isIdentifier(const string &s) {
    // NFA start state
    int i = 0;

    // First character must be letter or underscore
    if (!(isalpha(s[i]) || s[i] == '_'))
        return false;

    // Remaining characters
    i++;
    while (i < s.length()) {
        if (!(isalnum(s[i]) || s[i] == '_'))
            return false;
        i++;
    }
    return true; // accepting state
}

bool isConstant(const string &s) {
    int i = 0;

    // At least one digit required
    if (!isdigit(s[i]))
        return false;

    while (i < s.length()) {
        if (!isdigit(s[i]))
            return false;
        i++;
    }
    return true; // accepting state
}

bool isOperator(const string &s) {
    if (s.length() != 1)
        return false;

    char op = s[0];
    return (op == '+' || op == '-' || op == '*' ||
            op == '/' || op == '=' || op == '<' ||
            op == '>');
}

int main() {
    string token;

    cout << "Enter a token: ";
    cin >> token;

    if (isIdentifier(token))
        cout << "Valid Identifier" << endl;
    else if (isConstant(token))
        cout << "Valid Constant" << endl;
    else if (isOperator(token))
        cout << "Valid Operator" << endl;
    else
        cout << "Invalid Token" << endl;

    return 0;
}
