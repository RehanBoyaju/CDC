#include <iostream>
#include <cctype>
using namespace std;

int main() {
    string code = "int x = 10 + 5;";
    
    cout << "Input: " << code << "\nTokens:\n";
    
    for (int i = 0; i < code.length(); i++) {
        // Skip spaces
        if (isspace(code[i])) continue;
        
        // Check for keywords/identifiers
        if (isalpha(code[i])) {
            string token = "";
            while (i < code.length() && isalnum(code[i])) {
                token += code[i++];
            }
            i--;
            
            // Check if it's a keyword
            if (token == "int" || token == "float" || token == "if" || 
                token == "else" || token == "while" || token == "return") {
                cout << "Keyword: " << token << endl;
            } else {
                cout << "Identifier: " << token << endl;
            }
        }
        // Check for numbers
        else if (isdigit(code[i])) {
            string token = "";
            while (i < code.length() && isdigit(code[i])) {
                token += code[i++];
            }
            i--;
            cout << "Number: " << token << endl;
        }
        // Check for operators
        else if (code[i] == '+' || code[i] == '-' || code[i] == '*' || 
                 code[i] == '/' || code[i] == '=') {
            cout << "Operator: " << code[i] << endl;
        }
        // Check for separators
        else if (code[i] == ';' || code[i] == ',' || code[i] == '(' || code[i] == ')') {
            cout << "Separator: " << code[i] << endl;
        }
    }
    
    return 0;
}