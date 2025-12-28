#include <bits/stdc++.h>

using namespace std;

// Run chcp 65001 to enable unicode input

map<char, vector<string>> grammar;
set<char> terminals;
map<char, set<char>> followSet;
map<char, set<char>> firstSet;
// map<pair<char, char>, set<int>> parseTable;

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
        // production = modifyInput(production);
        
        char lhs = production[0];
        string rhs = production.substr(3);
        
         
        
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

    // Display FOLLOW sets
    cout << "\nFOLLOW sets:\n";
    for (auto p : grammar) {
        cout << "FOLLOW(" << p.first << ") = { ";
        for (char s : followSet[p.first])
            cout << s << " ";
        cout << "}\n";
    }

    return 0;
}
