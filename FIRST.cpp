#include <bits/stdc++.h>
using namespace std;

map<char, vector<string>> grammar;
map<char, set<char>> firstSet;
set<char> visited;

bool isTerminal(char c) {
    return !(c >= 'A' && c <= 'Z');
}

set<char> findFirst(string production) {
    set<char> res;
    
    if (production == "#") {
        res.insert('#');
        return res;
    }
    
    for (int i = 0; i < production.size(); i++) {
        char curr = production[i];
        
        if (isTerminal(curr)) {
            res.insert(curr);
            break;
        }
        
        bool hasEpsilon = false;
        for (char ch : firstSet[curr]) {
            if (ch == '#') {
                hasEpsilon = true;
            } else {
                res.insert(ch);
            }
        }
        
        if (!hasEpsilon) {
            break;
        }
        
        if (i == production.size() - 1) {
            res.insert('#');
        }
    }
    
    return res;
}

bool findFirstSets() {
    bool isChanged = false;
    
    for (auto p : grammar) {
        char symbol = p.first;
        
        for (string production : grammar[symbol]) {
            auto res = findFirst(production);
            size_t before = firstSet[symbol].size();
            firstSet[symbol].insert(res.begin(), res.end());
            
            if (firstSet[symbol].size() > before) {
                isChanged = true;
            }
        }
    }
    
    return isChanged;
}

string modifyInput(string input) {
    size_t pos = 0;
    
    while ((pos = input.find("ε", pos)) != string::npos) {
        input.replace(pos, string("ε").length(), "#");
        pos += 1;
    }
    
    return input;
}

string modifyOutput(string output) {
    size_t pos = 0;
    
    while ((pos = output.find("#", pos)) != string::npos) {
        output.replace(pos, string("#").length(), "ε");
    }
    
    return output;
}

int main() {
    int n;
    char startSymbol;
    
    cout << "Enter the number of productions: ";
    cin >> n;
    cin.ignore();
    
    for (int i = 0; i < n; i++) {
        cout << "Enter the production " << i + 1 << ": ";
        string production;
        getline(cin, production);
        production = modifyInput(production);
        
        char lhs = production[0];
        string rhs = production.substr(3);
        
        if (i == 0) {
            startSymbol = lhs;
        }
        
        string temp = "";
        for (char c : rhs) {
            if (c == '|') {
                grammar[lhs].push_back(temp);
                temp = "";
            } else {
                temp += c;
            }
        }
        grammar[lhs].push_back(temp);
    }
    
    // Calculate FIRST sets
    while (findFirstSets());

    // Display FIRST sets
    cout << "\nFIRST sets:\n";
    for (auto p : grammar) {
        cout << "FIRST(" << p.first << ") = { ";
        for (char s : firstSet[p.first])
            cout << modifyOutput(string(1, s)) << " ";
        cout << "}\n";
    }
    return 0;
}
