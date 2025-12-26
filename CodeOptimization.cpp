#include <iostream>
#include <string>
using namespace std;

string constantFolding(string code) {
    // Optimize: x = 2 + 3 -> x = 5
    size_t pos = code.find(" = ");
    if (pos != string::npos) {
        size_t plus = code.find(" + ", pos);
        if (plus != string::npos) {
            int num1 = code[plus - 1] - '0';
            int num2 = code[plus + 3] - '0';
            int result = num1 + num2;
            
            string optimized = code.substr(0, pos + 3) + to_string(result) + ";";
            cout << "Constant Folding Applied!\n";
            return optimized;
        }
    }
    return code;
}

string algebraicSimplification(string code) {
    size_t pos = code.find(" * 1");
    if (pos != string::npos) {
        string optimized = code.substr(0, pos) + ";";
        cout << "Algebraic Simplification Applied!\n";
        return optimized;
    }
    
    pos = code.find(" + 0");
    if (pos != string::npos) {
        string optimized = code.substr(0, pos) + ";";
        cout << "Algebraic Simplification Applied!\n";
        return optimized;
    }
    
    return code;
}

string strengthReduction(string code) {
    // Optimize: x * 2 -> x << 1 (multiply by 2 using bit shift)
    size_t pos = code.find(" * 2");
    if (pos != string::npos) {
        string optimized = code.substr(0, pos) + " << 1;";
        cout << "Strength Reduction Applied!\n";
        return optimized;
    }
    return code;
}

int main() {
    
    // Test 1: Constant Folding
    string code1 = "int x = 2 + 3;";
    cout << "Original: " << code1 << endl;
    cout << "Optimized: " << constantFolding(code1) << "\n\n";
    
    // Test 2: Algebraic Simplification
    string code2 = "int y = x * 1;";
    cout << "Original: " << code2 << endl;
    cout << "Optimized: " << algebraicSimplification(code2) << "\n\n";
    
    // string code2b = "int z = x + 0;";
    // cout << "Original: " << code2b << endl;
    // cout << "Optimized: " << algebraicSimplification(code2b) << "\n\n";
    
    // Test 3: Strength Reduction
    string code3 = "int y = x * 2;";
    cout << "Original: " << code3 << endl;
    cout << "Optimized: " << strengthReduction(code3) << "\n\n";
    
    return 0;
}