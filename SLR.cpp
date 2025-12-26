#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>

using namespace std;

// Production rule structure
struct Production {
    char lhs;
    string rhs;
    Production(char l, string r) : lhs(l), rhs(r) {}
};

// Item structure for LR(0) items
struct Item {
    int prodIndex;
    int dotPos;
    Item(int p,int d): prodIndex(p),dotPos(d){}
    bool operator<(const Item& other) const {
        if (prodIndex != other.prodIndex) return prodIndex < other.prodIndex;
        return dotPos < other.dotPos;
    }
    
    bool operator==(const Item& other) const {
        return prodIndex == other.prodIndex && dotPos == other.dotPos;
    }
};

class SLRTableBuilder {
private:
    vector<Production> prods;
    map<pair<int, char>, string> actionTable;
    map<pair<int, char>, int> gotoTable;
    
    // Helper function to check if character is non-terminal
    bool isNonTerminal(char c) {
        return c >= 'A' && c <= 'Z';
    }
    
    // Helper function to check if character is terminal
    bool isTerminal(char c) {
        return !isNonTerminal(c) && c != '$';
    }
    
    // Get closure of items
    set<Item> getClosure(const set<Item>& items) {
        set<Item> closure = items;
        bool changed = true;
        
        while (changed) {
            changed = false;
            set<Item> newItems;
            
            for (const auto& item : closure) {
                const Production& prod = prods[item.prodIndex];
                
                // If dot is at the end, skip
                if (item.dotPos >= prod.rhs.length()) continue;
                
                char nextSym = prod.rhs[item.dotPos];
                
                // If next symbol is non-terminal
                if (isNonTerminal(nextSym)) {
                    // Add all productions of this non-terminal
                    for (int i = 0; i < prods.size(); i++) {
                        if (prods[i].lhs == nextSym) {
                            Item newItem(i, 0);
                            if (closure.find(newItem) == closure.end()) {
                                newItems.insert(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
            
            for (const auto& item : newItems) {
                closure.insert(item);
            }
        }
        
        return closure;
    }
    
    // Get goto set
    set<Item> getGoto(const set<Item>& items, char symbol) {
        set<Item> gotoSet;
        
        for (const auto& item : items) {
            const Production& prod = prods[item.prodIndex];
            
            if (item.dotPos < prod.rhs.length() && prod.rhs[item.dotPos] == symbol) {
                gotoSet.insert(Item{item.prodIndex, item.dotPos + 1});
            }
        }
        
        return getClosure(gotoSet);
    }
    
    // Check if two sets of items are equal
    bool areItemsEqual(const set<Item>& a, const set<Item>& b) {
        if (a.size() != b.size()) return false;
        
        auto it1 = a.begin();
        auto it2 = b.begin();
        
        while (it1 != a.end() && it2 != b.end()) {
            if (!(*it1 == *it2)) return false;
            ++it1;
            ++it2;
        }
        
        return true;
    }
    
    // Simplified FOLLOW sets for our simple grammar
    set<char> getFollow(char nonTerm) {
        set<char> follow;
        
        if (nonTerm == 'S') {
            follow.insert('$');
        } else if (nonTerm == 'A') {
            follow.insert('+');
        } else if (nonTerm == 'B') {
            follow.insert('$');
        }
        
        return follow;
    }
    
public:
    SLRTableBuilder() {
        // VERY SIMPLE GRAMMAR:
        // S -> A + B
        // A -> a
        // B -> b
        
        prods.push_back(Production('X', "S"));  // Augmented production (0)
        prods.push_back(Production('S', "A+B")); // Production 1
        prods.push_back(Production('A', "a"));   // Production 2
        prods.push_back(Production('B', "b"));   // Production 3
    }
    
    void buildTable() {
        // Step 1: Build canonical collection of LR(0) items
        vector<set<Item>> states;
        map<pair<int, char>, int> transitions;
        
        // Initial state: closure of {[X -> .S]}
        set<Item> initial;
        initial.insert(Item{0, 0});
        states.push_back(getClosure(initial));
        
        // Build states
        for (int i = 0; i < states.size(); i++) {
            set<Item> currentState = states[i];
            
            // Get all symbols that appear after dots
            set<char> symbols;
            for (const auto& item : currentState) {
                const Production& prod = prods[item.prodIndex];
                if (item.dotPos < prod.rhs.length()) {
                    symbols.insert(prod.rhs[item.dotPos]);
                }
            }
            
            // For each symbol, compute goto
            for (char sym : symbols) {
                set<Item> gotoSet = getGoto(currentState, sym);
                
                if (!gotoSet.empty()) {
                    // Check if this state already exists
                    int stateIndex = -1;
                    for (int j = 0; j < states.size(); j++) {
                        if (areItemsEqual(states[j], gotoSet)) {
                            stateIndex = j;
                            break;
                        }
                    }
                    
                    if (stateIndex == -1) {
                        stateIndex = states.size();
                        states.push_back(gotoSet);
                    }
                    
                    transitions[{i, sym}] = stateIndex;
                }
            }
        }
        
        // Step 2: Build ACTION and GOTO tables
        for (int i = 0; i < states.size(); i++) {
            set<Item> currentState = states[i];
            
            // Check each item in the state
            for (const auto& item : currentState) {
                const Production& prod = prods[item.prodIndex];
                
                // Case 1: Shift
                if (item.dotPos < prod.rhs.length()) {
                    char nextSym = prod.rhs[item.dotPos];
                    if (isTerminal(nextSym)) {
                        auto it = transitions.find({i, nextSym});
                        if (it != transitions.end()) {
                            actionTable[{i, nextSym}] = "s" + to_string(it->second);
                        }
                    }
                }
                // Case 2: Reduce/Accept
                else {
                    if (item.prodIndex == 0) {
                        // X -> S.
                        actionTable[{i, '$'}] = "acc";
                    } else {
                        // Regular reduce
                        set<char> follow = getFollow(prod.lhs);
                        for (char terminal : follow) {
                            actionTable[{i, terminal}] = "r" + to_string(item.prodIndex);
                        }
                    }
                }
            }
            
            // Build GOTO table for non-terminals
            for (char nt = 'A'; nt <= 'Z'; nt++) {
                if (nt == 'X') continue;  // Skip augmented start
                
                auto it = transitions.find({i, nt});
                if (it != transitions.end()) {
                    gotoTable[{i, nt}] = it->second;
                }
            }
        }
        
        // Display results
        displayTables(states);
    }
    
    void displayTables(const vector<set<Item>>& states) {
        // cout << "SLR PARSING TABLE FOR SIMPLE GRAMMAR\n";
        // cout << "====================================\n\n";
        
        cout << "Grammar:\n";
        cout << "0: S' -> S     (augmented start)\n";
        cout << "1: S -> A + B\n";
        cout << "2: A -> a\n";
        cout << "3: B -> b\n\n";
        
        // Create list of terminals and non-terminals
        set<char> terminals = {'a', 'b', '+', '$'};
        set<char> nonTerminals = {'S', 'A', 'B'};
        
        // Display combined ACTION and GOTO table in one row per state
        cout << "SLR(1) Parsing Table\n";
        cout << "State\t";
        
        // First display ACTION columns (terminals)
        for (char t : terminals) {
            cout << t << "\t";
        }
        
        // Then display GOTO columns (non-terminals)
        for (char nt : nonTerminals) {
            if (nt != 'X') {  // Skip augmented start
                cout << nt << "\t";
            }
        }
        cout << endl;
        
        // Display table rows
        for (int i = 0; i < states.size(); i++) {
            cout << i << "\t";
            
            // Display ACTION table entries
            for (char t : terminals) {
                auto it = actionTable.find({i, t});
                if (it != actionTable.end()) {
                    cout << it->second;
                }
                cout << "\t";
            }
            
            // Display GOTO table entries
            for (char nt : nonTerminals) {
                if (nt != 'X') {  // Skip augmented start
                    auto it = gotoTable.find({i, nt});
                    if (it != gotoTable.end()) {
                        cout << it->second;
                    }
                    cout << "\t";
                }
            }
            cout << endl;
        }
        
        // Display legend
        // cout << "\nLEGEND:\n";
        // cout << "------\n";
        // cout << "sX  = Shift and go to state X\n";
        // cout << "rX  = Reduce using production X\n";
        // cout << "acc = Accept\n";
        // cout << "Numbers in GOTO columns = Next state number\n";
        
        // Display states
        // cout << "\nStates (" << states.size() << " total):\n";
        for (int i = 0; i < states.size(); i++) {
            cout << "\nI" << i << ":\n";
            for (const auto& item : states[i]) {
                const Production& prod = prods[item.prodIndex];
                cout << "  " << prod.lhs << " -> ";
                for (int j = 0; j < prod.rhs.length(); j++) {
                    if (j == item.dotPos) cout << ".";
                    cout << prod.rhs[j];
                }
                if (item.dotPos == prod.rhs.length()) cout << ".";
                // cout << endl;
            }
        }
        
        // Display productions separately
        // cout << "\nProductions:\n";
        // for (int i = 0; i < prods.size(); i++) {
        //     cout << i << ": " << prods[i].lhs << " -> " << prods[i].rhs << endl;
        // }
    }
};

int main() {
    SLRTableBuilder parser;
    parser.buildTable();
    return 0;
}