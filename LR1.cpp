#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>

using namespace std;

// Structure to represent a production rule
struct Production {
    char lhs;  // Left-hand side
    string rhs; // Right-hand side
    int id;     // Production ID
    
    Production(char l, string r, int i = -1) : lhs(l), rhs(r), id(i) {}
};

// Structure to represent an LR(0) item
struct LR0Item {
    Production production;
    int dotPosition;
    
    LR0Item(Production p, int pos) : production(p), dotPosition(pos) {}
    
    bool operator<(const LR0Item& other) const {
        if (production.lhs != other.production.lhs)
            return production.lhs < other.production.lhs;
        if (production.rhs != other.production.rhs)
            return production.rhs < other.production.rhs;
        return dotPosition < other.dotPosition;
    }
    
    bool operator==(const LR0Item& other) const {
        return production.lhs == other.production.lhs &&
               production.rhs == other.production.rhs &&
               dotPosition == other.dotPosition;
    }
};

class SLR1Parser {
private:
    vector<Production> productions;
    char startSymbol;
    set<char> terminals;
    set<char> nonTerminals;
    vector<set<LR0Item>> states;
    map<pair<int, char>, string> parsingTable;
    map<char, set<char>> firstSets;
    map<char, set<char>> followSets;
    
public:
    SLR1Parser(vector<Production> prods, char start) : productions(prods), startSymbol(start) {
        // Assign production IDs
        for (size_t i = 0; i < productions.size(); i++) {
            productions[i].id = i;
        }
        
        // Extract terminals and non-terminals
        extractSymbols();
        
        // Compute FIRST and FOLLOW sets
        computeFirstSets();
        computeFollowSets();
        
        // Generate canonical collection of LR(0) items
        generateStates();
        
        // Build SLR(1) parsing table
        buildParsingTable();
    }
    
private:
    void extractSymbols() {
        for (const auto& prod : productions) {
            nonTerminals.insert(prod.lhs);
            for (char c : prod.rhs) {
                if (isupper(c)) {
                    nonTerminals.insert(c);
                } else if (c != 'ε') {  // ε represents epsilon
                    terminals.insert(c);
                }
            }
        }
        terminals.insert('$');  // End marker
    }
    
    void computeFirstSets() {
        // Initialize FIRST sets
        for (char nt : nonTerminals) {
            firstSets[nt] = set<char>();
        }
        
        bool changed;
        do {
            changed = false;
            for (const auto& prod : productions) {
                char lhs = prod.lhs;
                const string& rhs = prod.rhs;
                
                if (rhs == "ε") {
                    // Epsilon production
                    if (firstSets[lhs].find('ε') == firstSets[lhs].end()) {
                        firstSets[lhs].insert('ε');
                        changed = true;
                    }
                } else {
                    for (char symbol : rhs) {
                        if (terminals.find(symbol) != terminals.end()) {
                            // Terminal symbol
                            if (firstSets[lhs].find(symbol) == firstSets[lhs].end()) {
                                firstSets[lhs].insert(symbol);
                                changed = true;
                            }
                            break;
                        } else {
                            // Non-terminal symbol
                            bool epsilonFound = false;
                            for (char firstSym : firstSets[symbol]) {
                                if (firstSym != 'ε') {
                                    if (firstSets[lhs].find(firstSym) == firstSets[lhs].end()) {
                                        firstSets[lhs].insert(firstSym);
                                        changed = true;
                                    }
                                } else {
                                    epsilonFound = true;
                                }
                            }
                            if (!epsilonFound) break;
                        }
                    }
                }
            }
        } while (changed);
    }
    
    void computeFollowSets() {
        // Initialize FOLLOW sets
        for (char nt : nonTerminals) {
            followSets[nt] = set<char>();
        }
        followSets[startSymbol].insert('$');  // $ in FOLLOW of start symbol
        
        bool changed;
        do {
            changed = false;
            for (const auto& prod : productions) {
                char lhs = prod.lhs;
                const string& rhs = prod.rhs;
                
                for (size_t i = 0; i < rhs.length(); i++) {
                    char current = rhs[i];
                    if (nonTerminals.find(current) != nonTerminals.end()) {
                        // For A -> αBβ, add FIRST(β)-{ε} to FOLLOW(B)
                        if (i + 1 < rhs.length()) {
                            char next = rhs[i + 1];
                            if (terminals.find(next) != terminals.end()) {
                                // Next is terminal
                                if (followSets[current].find(next) == followSets[current].end()) {
                                    followSets[current].insert(next);
                                    changed = true;
                                }
                            } else {
                                // Next is non-terminal
                                for (char firstSym : firstSets[next]) {
                                    if (firstSym != 'ε') {
                                        if (followSets[current].find(firstSym) == followSets[current].end()) {
                                            followSets[current].insert(firstSym);
                                            changed = true;
                                        }
                                    }
                                }
                            }
                            
                            // If β can derive ε, add FOLLOW(A) to FOLLOW(B)
                            bool allEpsilon = true;
                            for (size_t j = i + 1; j < rhs.length(); j++) {
                                if (terminals.find(rhs[j]) != terminals.end()) {
                                    allEpsilon = false;
                                    break;
                                }
                                if (firstSets[rhs[j]].find('ε') == firstSets[rhs[j]].end()) {
                                    allEpsilon = false;
                                    break;
                                }
                            }
                            if (allEpsilon) {
                                for (char followSym : followSets[lhs]) {
                                    if (followSets[current].find(followSym) == followSets[current].end()) {
                                        followSets[current].insert(followSym);
                                        changed = true;
                                    }
                                }
                            }
                        } else {
                            // For A -> αB, add FOLLOW(A) to FOLLOW(B)
                            for (char followSym : followSets[lhs]) {
                                if (followSets[current].find(followSym) == followSets[current].end()) {
                                    followSets[current].insert(followSym);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        } while (changed);
    }
    
    // Closure operation for LR(0) items
    set<LR0Item> closure(set<LR0Item> I) {
        set<LR0Item> closureSet = I;
        bool changed;
        
        do {
            changed = false;
            set<LR0Item> newItems;
            
            for (const auto& item : closureSet) {
                if (item.dotPosition < item.production.rhs.length()) {
                    char nextSymbol = item.production.rhs[item.dotPosition];
                    
                    if (nonTerminals.find(nextSymbol) != nonTerminals.end()) {
                        // Add all productions for this non-terminal
                        for (const auto& prod : productions) {
                            if (prod.lhs == nextSymbol) {
                                LR0Item newItem(prod, 0);
                                if (closureSet.find(newItem) == closureSet.end() && 
                                    newItems.find(newItem) == newItems.end()) {
                                    newItems.insert(newItem);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
            
            closureSet.insert(newItems.begin(), newItems.end());
        } while (changed);
        
        return closureSet;
    }
    
    // Goto operation for LR(0) items
    set<LR0Item> gotoSet(const set<LR0Item>& I, char X) {
        set<LR0Item> gotoSet;
        
        for (const auto& item : I) {
            if (item.dotPosition < item.production.rhs.length() && 
                item.production.rhs[item.dotPosition] == X) {
                gotoSet.insert(LR0Item(item.production, item.dotPosition + 1));
            }
        }
        
        return closure(gotoSet);
    }
    
    // Generate all states (canonical collection)
    void generateStates() {
        // Augmented grammar: add S' -> S
        Production augmentedProd('S', string(1, startSymbol), productions.size());
        set<LR0Item> startItems = {LR0Item(augmentedProd, 0)};
        
        states.push_back(closure(startItems));
        
        bool changed;
        do {
            changed = false;
            size_t currentSize = states.size();
            
            for (size_t i = 0; i < currentSize; i++) {
                // Check for transitions on all symbols
                for (char symbol : terminals) {
                    if (symbol == '$') continue; // Skip end marker for goto
                    
                    auto newState = gotoSet(states[i], symbol);
                    if (!newState.empty() && 
                        find(states.begin(), states.end(), newState) == states.end()) {
                        states.push_back(newState);
                        changed = true;
                    }
                }
                
                for (char symbol : nonTerminals) {
                    auto newState = gotoSet(states[i], symbol);
                    if (!newState.empty() && 
                        find(states.begin(), states.end(), newState) == states.end()) {
                        states.push_back(newState);
                        changed = true;
                    }
                }
            }
        } while (changed);
    }
    
    // Build the SLR(1) parsing table
    void buildParsingTable() {
        // Initialize table with empty entries
        for (size_t i = 0; i < states.size(); i++) {
            for (char term : terminals) {
                parsingTable[{i, term}] = "";
            }
            for (char nonTerm : nonTerminals) {
                parsingTable[{i, nonTerm}] = "";
            }
        }
        
        // Fill the table with shift and goto actions
        for (size_t i = 0; i < states.size(); i++) {
            // Shift actions
            for (char term : terminals) {
                if (term == '$') continue;
                
                auto newState = gotoSet(states[i], term);
                if (!newState.empty()) {
                    auto it = find(states.begin(), states.end(), newState);
                    if (it != states.end()) {
                        size_t j = distance(states.begin(), it);
                        string action = "s" + to_string(j);
                        
                        if (!parsingTable[{i, term}].empty() && parsingTable[{i, term}] != action) {
                            cout << "Shift-Reduce conflict at [" << i << "," << term << "]: " 
                                 << parsingTable[{i, term}] << " vs " << action << endl;
                        }
                        parsingTable[{i, term}] = action;
                    }
                }
            }
            
            // Reduce actions (SLR specific - use FOLLOW sets)
            for (const auto& item : states[i]) {
                if (item.dotPosition == item.production.rhs.length()) {
                    // Reduction item
                    if (item.production.lhs == 'S' && item.production.rhs == string(1, startSymbol)) {
                        // Accept action
                        if (!parsingTable[{i, '$'}].empty() && parsingTable[{i, '$'}] != "acc") {
                            cout << "Conflict at [" << i << ",$]: " << parsingTable[{i, '$'}] << " vs acc" << endl;
                        }
                        parsingTable[{i, '$'}] = "acc";
                    } else {
                        // Regular reduce action - use FOLLOW set for reduce
                        int prodNum = item.production.id;
                        for (char term : followSets[item.production.lhs]) {
                            string action = "r" + to_string(prodNum);
                            
                            if (!parsingTable[{i, term}].empty()) {
                                if (parsingTable[{i, term}].substr(0, 1) == "s") {
                                    cout << "Shift-Reduce conflict at [" << i << "," << term << "]: " 
                                         << parsingTable[{i, term}] << " vs " << action << endl;
                                } else if (parsingTable[{i, term}].substr(0, 1) == "r") {
                                    cout << "Reduce-Reduce conflict at [" << i << "," << term << "]: " 
                                         << parsingTable[{i, term}] << " vs " << action << endl;
                                }
                            }
                            parsingTable[{i, term}] = action;
                        }
                    }
                }
            }
            
            // Goto actions
            for (char nonTerm : nonTerminals) {
                auto newState = gotoSet(states[i], nonTerm);
                if (!newState.empty()) {
                    auto it = find(states.begin(), states.end(), newState);
                    if (it != states.end()) {
                        size_t j = distance(states.begin(), it);
                        parsingTable[{i, nonTerm}] = to_string(j);
                    }
                }
            }
        }
    }
    
public:
    // Display the parsing table
    void displayParsingTable() {
        cout << "\nSLR(1) PARSING TABLE\n";
        cout << "====================\n\n";
        
        // Header row
        cout << setw(8) << "State";
        vector<char> allSymbols;
        for (char term : terminals) allSymbols.push_back(term);
        for (char nonTerm : nonTerminals) allSymbols.push_back(nonTerm);
        
        for (char sym : allSymbols) {
            cout << setw(8) << sym;
        }
        cout << endl;
        
        cout << string(8 + allSymbols.size() * 8, '-') << endl;
        
        // Table rows
        for (size_t i = 0; i < states.size(); i++) {
            cout << setw(8) << i;
            
            for (char sym : allSymbols) {
                string action = parsingTable[{i, sym}];
                if (action.empty()) action = "-";
                cout << setw(8) << action;
            }
            cout << endl;
        }
    }
    
    // Display the grammar
    void displayGrammar() {
        cout << "GRAMMAR:\n";
        cout << "========\n";
        for (size_t i = 0; i < productions.size(); i++) {
            cout << "(" << i << ") " << productions[i].lhs << " -> " << productions[i].rhs << endl;
        }
        cout << endl;
    }
    
    // Display FIRST sets
    void displayFirstSets() {
        cout << "FIRST SETS:\n";
        cout << "===========\n";
        for (char nt : nonTerminals) {
            cout << "FIRST(" << nt << ") = { ";
            for (char f : firstSets[nt]) {
                cout << f << " ";
            }
            cout << "}\n";
        }
        cout << endl;
    }
    
    // Display FOLLOW sets
    void displayFollowSets() {
        cout << "FOLLOW SETS:\n";
        cout << "============\n";
        for (char nt : nonTerminals) {
            cout << "FOLLOW(" << nt << ") = { ";
            for (char f : followSets[nt]) {
                cout << f << " ";
            }
            cout << "}\n";
        }
        cout << endl;
    }
    
    // Display all states
    void displayStates() {
        cout << "CANONICAL COLLECTION OF LR(0) ITEMS:\n";
        cout << "====================================\n";
        for (size_t i = 0; i < states.size(); i++) {
            cout << "I" << i << ":\n";
            for (const auto& item : states[i]) {
                cout << "  " << item.production.lhs << " -> ";
                for (int j = 0; j < item.production.rhs.length(); j++) {
                    if (j == item.dotPosition) cout << ".";
                    cout << item.production.rhs[j];
                }
                if (item.dotPosition == item.production.rhs.length()) cout << ".";
                cout << endl;
            }
            cout << endl;
        }
    }
};

int main() {
    // Example grammar: E -> E + T | T, T -> T * F | F, F -> (E) | id
    vector<Production> productions = {
        Production('E', "E+T", 0),
        Production('E', "T", 1),
        Production('T', "T*F", 2),
        Production('T', "F", 3),
        Production('F', "(E)", 4),
        Production('F', "id", 5)
    };
    
    char startSymbol = 'E';
    
    SLR1Parser parser(productions, startSymbol);
    
    // Display information
    parser.displayGrammar();
    parser.displayFirstSets();
    parser.displayFollowSets();
    parser.displayStates();
    parser.displayParsingTable();
    
    return 0;
}