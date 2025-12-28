#include <bits/stdc++.h>
using namespace std;

map<char, vector<string>> grammar;
map<char, set<char>> firstSet;
map<char, set<char>> followSet;

bool isTerminal(char c) {
    return !(c >= 'A' && c <= 'Z');
}

int main() {
    // Hardcoded simple grammar
    // S -> AB
    // A -> a
    // B -> b
    grammar['S'] = {"AB"};
    grammar['A'] = {"a","Ba"};
    grammar['B'] = {"b"};

    // Initialize FIRST sets for terminals
    for (auto &p : grammar) {
        for (string prod : p.second) {
            for (char c : prod) {
                if (isTerminal(c)) firstSet[c].insert(c);
            }
        }
    }

    // -------- FIX-POINT ITERATION for FIRST sets --------
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &p : grammar) {
            char A = p.first;
            for (string prod : p.second) {
                if (prod.empty()) continue;
                char firstChar = prod[0];
                size_t before = firstSet[A].size();
                firstSet[A].insert(firstSet[firstChar].begin(), firstSet[firstChar].end());
                if (firstSet[A].size() > before) changed = true;
            }
        }
    }

    // -------- FIX-POINT ITERATION for FOLLOW sets --------
    followSet['S'].insert('$'); // Start symbol
    changed = true;
    while (changed) {
        changed = false;
        for (auto &p : grammar) {
            char A = p.first;
            for (string prod : p.second) {
                for (int i = 0; i < prod.size(); i++) {
                    char B = prod[i];
                    if (isTerminal(B)) continue;
                    if (i + 1 < prod.size()) {
                        char next = prod[i + 1];
                        size_t before = followSet[B].size();
                        followSet[B].insert(firstSet[next].begin(), firstSet[next].end());
                        if (followSet[B].size() > before) changed = true;
                    } else { // Last symbol gets FOLLOW of LHS
                        size_t before = followSet[B].size();
                        followSet[B].insert(followSet[A].begin(), followSet[A].end());
                        if (followSet[B].size() > before) changed = true;
                    }
                }
            }
        }
    }

    // Print FIRST sets
    cout << "FIRST sets:\n";
    for (auto &p : firstSet) {
        if (!isTerminal(p.first)) {
            cout << p.first << " : { ";
            for (char c : p.second) cout << c << " ";
            cout << "}\n";
        }
    }

    // Print FOLLOW sets
    cout << "\nFOLLOW sets:\n";
    for (auto &p : followSet) {
        cout << p.first << " : { ";
        for (char c : p.second) cout << c << " ";
        cout << "}\n";
    }

    return 0;
}
