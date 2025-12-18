#include <bits/stdc++.h>

using namespace std;

// Run chcp 65001 to enable unicode input

map<char, vector<string>> grammar;
set<char> terminals;
map<char, set<char>> followSet;
map<char, set<char>> firstSet;
map<pair<char, char>, set<int>> parseTable;

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

bool findFollowSets() {
    bool isChanged = false;
    
    for (auto p : grammar) {
        char symbol = p.first;
        
        for (string production : grammar[symbol]) {
            if (production == "#")
                continue;
            
            // Process all symbols except the last one
            for (int i = 0; i < production.size() - 1; i++) {
                char currSymbol = production[i];
                
                if (isTerminal(production[i])) {
                    continue;
                }
                
                for (int j = i + 1; j < production.size(); j++) {
                    bool hasEpsilon = false;
                    char nextSymbol = production[j];
                    
                    if (isTerminal(production[j])) {
                        auto result = followSet[currSymbol].insert(nextSymbol);
                        if (result.second) {
                            isChanged = true;
                        }
                        break;
                    } else {
                        for (char first : firstSet[production[j]]) {
                            if (first == '#') {
                                hasEpsilon = true;
                            } else {
                                auto result = followSet[currSymbol].insert(first);
                                if (result.second) {
                                    isChanged = true;
                                }
                            }
                        }
                    }
                    
                    if (!hasEpsilon)
                        break;
                    
                    if (j == production.size() - 1) {
                        size_t before = followSet[currSymbol].size();
                        followSet[currSymbol].insert(
                            followSet[symbol].begin(),
                            followSet[symbol].end()
                        );
                        if (followSet[currSymbol].size() > before) {
                            isChanged = true;
                        }
                    }
                }
            }
            
            // Process last symbol
            char lastSymbol = production[production.size() - 1];
            if (!isTerminal(lastSymbol)) {
                size_t before = followSet[lastSymbol].size();
                followSet[lastSymbol].insert(
                    followSet[symbol].begin(),
                    followSet[symbol].end()
                );
                if (followSet[lastSymbol].size() > before) {
                    isChanged = true;
                }
            }
        }
    }
    
    return isChanged;
}

void evaluateLL1ParseTable() {
    int index = 1;
    
    for (auto production : grammar) {
        char lhs = production.first;
        
        for (string rhs : production.second) {
            auto first = findFirst(rhs);
            bool hasEpsilon = false;
            
            for (char symbol : first) {
                if (symbol == '#') {
                    hasEpsilon = true;
                } else {
                    parseTable[pair<char, char>(lhs, symbol)].insert(index);
                }
            }
            
            if (hasEpsilon) {
                for (char symbol : followSet[lhs]) {
                    parseTable[pair<char, char>(lhs, symbol)].insert(index);
                }
            }
            
            index++;
        }
    }
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
                if (isTerminal(c) && c != '#') {
                    terminals.insert(c);
                }
            }
        }
        grammar[lhs].push_back(temp);
    }
    
    // Calculate FIRST sets
    while (findFirstSets());
    
    // Initialize FOLLOW set for start symbol
    followSet[startSymbol].insert('$');
    terminals.insert('$');
    
    // Calculate FOLLOW sets
    while (findFollowSets());
    
    // Build LL(1) parse table
    evaluateLL1ParseTable();
    
    // Check if grammar is LL(1)
    bool isLL1 = true;
    for (auto entry : parseTable) {
        if (entry.second.size() > 1) {
            isLL1 = false;
            break;
        }
    }
    
    if (!isLL1) {
        cout << "\nWARNING: Grammar is NOT LL(1) (multiple entries in parse table)\n";
    }
    
    // Display production mapping
    cout << "\nProduction Mapping:\n";
    int idx = 1;
    for (auto p : grammar) {
        for (string prod : p.second) {
            cout << idx++ << ": " << p.first << " -> " << modifyOutput(prod) << "\n";
        }
    }
    
    // Display FIRST sets
    cout << "\nFIRST sets:\n";
    for (auto p : grammar) {
        cout << "FIRST(" << p.first << ") = { ";
        for (char s : firstSet[p.first])
            cout << modifyOutput(string(1, s)) << " ";
        cout << "}\n";
    }
    
    // Display FOLLOW sets
    cout << "\nFOLLOW sets:\n";
    for (auto p : grammar) {
        cout << "FOLLOW(" << p.first << ") = { ";
        for (char s : followSet[p.first])
            cout << s << " ";
        cout << "}\n";
    }
    
    // Display LL(1) Parsing Table
    cout << "\nLL(1) Parsing Table:\n";
    cout << "\t";
    for (auto terminal : terminals) {
        cout << terminal << "\t";
    }
    cout << endl;
    
    for (auto p : grammar) {
        char row = p.first;
        cout << row << "\t";
        
        for (auto col : terminals) {
            pair<char, char> key(row, col);
            
            if (parseTable.find(key) != parseTable.end()) {
                auto productions = parseTable[key];
                for (auto production : productions) {
                    cout << production << " ";
                }
                cout << "\t";
            } else {
                cout << "\t";
            }
        }
        cout << "\n";
    }
    
    return 0;
}

// Example inputs:
// S->aA|bB
// A->cS|ε
// B->dS|ε
//
// S->aA|aB
// A->b
// B->c