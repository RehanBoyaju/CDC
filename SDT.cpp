#include <iostream>
#include <string>
#include <cctype>
using namespace std;

class SDT {
private:
    string input;
    int pos;
    
    // F -> digit { F.val = digit.val }
    int factor() {
        // cout << "factor() called\n";
        if (pos < input.length() && isdigit(input[pos])) {
            int digit_val = input[pos] - '0';
            cout << "Semantic Action: F.val = " << digit_val << endl;
            pos++;
            return digit_val;
        }
        return 0;
    }
    
    // T -> F * T { T.val = F.val * T1.val } | F { T.val = F.val }
    int term() {
        // cout << "term() called\n";
        int F_val = factor();
        
        if (pos < input.length() && input[pos] == '*') {
            pos++;
            int T1_val = term();
            int T_val = F_val * T1_val;
            cout << "Semantic Action: T.val = " << F_val << " * " << T1_val << " = " << T_val << endl;
            return T_val;
        }
        
        cout << "Semantic Action: T.val = F.val = " << F_val << endl;
        return F_val;
    }
    
    // E -> T + E { E.val = T.val + E1.val } | T { E.val = T.val }
    int expr() {
        // cout << "expr() called\n";
        int T_val = term();
        
        if (pos < input.length() && input[pos] == '+') {
            pos++;
            int E1_val = expr();
            int E_val = T_val + E1_val;
            cout << "Semantic Action: E.val = " << T_val << " + " << E1_val << " = " << E_val << endl;
            return E_val;
        }
        
        cout << "Semantic Action: E.val = T.val = " << T_val << endl;
        return T_val;
    }
    
public:
    SDT(string s) : input(s), pos(0) {}
    
    void translate() {
        cout << "Grammar with Semantic Actions:\n";
        cout << "E -> T + E  { E.val = T.val + E1.val }\n";
        cout << "E -> T      { E.val = T.val }\n";
        cout << "T -> F * T  { T.val = F.val * T1.val }\n";
        cout << "T -> F      { T.val = F.val }\n";
        cout << "F -> digit  { F.val = digit.val }\n\n";
        
        cout << "Input Expression: " << input << endl;
        cout << "Parsing and Evaluating...\n\n";
        int result = expr();
        cout << "\nResult: " << result << endl;
    }
};

int main() {
    // cout << "=== SYNTAX DIRECTED TRANSLATION ===\n\n";
    
    string expression = "2+3*4";
    
    SDT translator(expression);
    translator.translate();
    
    return 0;
}