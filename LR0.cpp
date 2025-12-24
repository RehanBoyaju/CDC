    #include <iostream>
    #include <vector>
    #include <string>
    #include <map>
    #include <set>
    #include <algorithm>
    #include <iomanip>

    using namespace std;

    // Structure to represent a pr rule
    struct Production {
        char lhs;  // Left-hand side
        string rhs; // Right-hand side
        
        Production(char l, string r) : lhs(l), rhs(r) {}
    };

    // Structure to represent an LR(0) item
    struct Item {
        Production pr;
        int dot;
        
        Item(Production p, int pos) : pr(p), dot(pos) {}
        
        bool operator<(const Item& other) const {
            if (pr.lhs != other.pr.lhs)
                return pr.lhs < other.pr.lhs;
            if (pr.rhs != other.pr.rhs)
                return pr.rhs < other.pr.rhs;
            return dot < other.dot;
        }
        
        bool operator==(const Item& other) const {
            return pr.lhs == other.pr.lhs &&
                pr.rhs == other.pr.rhs &&
                dot == other.dot;
        }
    };

    class Parser {
    private:
        vector<Production> productions;
        char startSymbol;
        set<char> terminals;
        set<char> nonTerminals;
        vector<set<Item>> states;
        map<pair<int, char>, string> parsingTable;
        
    public:
        Parser(vector<Production> prods, char start) : productions(prods), startSymbol(start) {
            // Extract terminals and non-terminals
            for (const auto& prod : productions) {
                nonTerminals.insert(prod.lhs);
                for (char c : prod.rhs) {
                    if (isupper(c)) {
                        nonTerminals.insert(c);
                    } else if (c != '#') {  // # represents epsilon
                        terminals.insert(c);
                    }
                }
            }
            terminals.insert('$');  // End marker
            
            // Generate canonical collection of LR(0) items
            generateStates();
            
            // Build parsing table
            buildParsingTable();
        }
        
        // Closure operation for LR(0) items
        set<Item> closure(set<Item> I) {
            set<Item> closureSet = I;
            bool changed;
            
            do {
                changed = false;
                set<Item> newItems;
                
                for (const auto& item : closureSet) {
                    if (item.dot < item.pr.rhs.length()) {
                        char nextSymbol = item.pr.rhs[item.dot];
                        
                        if (nonTerminals.find(nextSymbol) != nonTerminals.end()) {
                            // Add all productions for this non-terminal
                            for (const auto& prod : productions) {
                                if (prod.lhs == nextSymbol) {
                                    Item newItem(prod, 0);
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
        set<Item> gotoSet(const set<Item>& I, char X) {
            set<Item> gotoSet;
            
            for (const auto& item : I) {
                if (item.dot < item.pr.rhs.length() && 
                    item.pr.rhs[item.dot] == X) {
                    gotoSet.insert(Item(item.pr, item.dot + 1));
                }
            }
            
            return closure(gotoSet);
        }
        
        // Generate all states (canonical collection)
        void generateStates() {
            // Augmented grammar: add S' -> S
            Production augmentedProd('S', string(1, startSymbol));
            set<Item> startItems = {Item(augmentedProd, 0)};
            
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
        
        // Build the parsing table
        void buildParsingTable() {
            // Initialize table with "error"
            for (size_t i = 0; i < states.size(); i++) {
                for (char term : terminals) {
                    parsingTable[{i, term}] = "error";
                }
                for (char nonTerm : nonTerminals) {
                    parsingTable[{i, nonTerm}] = "error";
                }
            }
            
            // Fill the table
            for (size_t i = 0; i < states.size(); i++) {
                // Check for shift actions
                for (char term : terminals) {
                    if (term == '$') continue;
                    
                    auto newState = gotoSet(states[i], term);
                    if (!newState.empty()) {
                        auto it = find(states.begin(), states.end(), newState);
                        if (it != states.end()) {
                            size_t j = distance(states.begin(), it);
                            parsingTable[{i, term}] = "s" + to_string(j);
                        }
                    }
                }
                
                // Check for reduce actions
                for (const auto& item : states[i]) {
                    if (item.dot == item.pr.rhs.length()) {
                        // Reduction item
                        if (item.pr.lhs == 'S' && item.pr.rhs == string(1, startSymbol)) {
                            // Accept action
                            parsingTable[{i, '$'}] = "acc";
                        } else {
                            // Find pr number
                            int prodNum = -1;
                            for (size_t k = 0; k < productions.size(); k++) {
                                if (productions[k].lhs == item.pr.lhs && 
                                    productions[k].rhs == item.pr.rhs) {
                                    prodNum = k;
                                    break;
                                }
                            }
                            
                            if (prodNum != -1) {
                                for (char term : terminals) {
                                    parsingTable[{i, term}] = "r" + to_string(prodNum);
                                }
                            }
                        }
                    }
                }
                
                // Check for goto actions
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
        
        // Display the parsing table
        void displayParsingTable() {
            cout << "\nLR(0) Parsing Table\n";
            // cout << "===================\n\n";
            
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
        
        // Display all states
        void displayStates() {
            cout << "CANONICAL LR(0) ITEMS:\n";
            // cout << "====================================\n";
            for (size_t i = 0; i < states.size(); i++) {
                cout << "I" << i << ":\n";
                for (const auto& item : states[i]) {
                    cout << "  " << item.pr.lhs << " -> ";
                    for (int j = 0; j < item.pr.rhs.length(); j++) {
                        if (j == item.dot) cout << ".";
                        cout << item.pr.rhs[j];
                    }
                    if (item.dot == item.pr.rhs.length()) cout << ".";
                    cout << endl;
                }
                cout << endl;
            }
        }
    };

    int main() {
        // Example grammar: E -> E + T | T, T -> T * F | F, F -> (E) | id
        // vector<Production> productions = {
        //     Production('E', "E+T"),
        //     Production('E', "T"),
        //     Production('T', "T*F"),
        //     Production('T', "F"),
        //     Production('F', "(E)"),
        //     Production('F', "id")
        // };
        vector<Production> productions = {
            Production('S', "AB"),
            Production('A', "a"),
            Production('B', "b")           
        };
        
        char startSymbol = 'S';
        
        Parser parser(productions, startSymbol);
        
        // Display information
        parser.displayGrammar();
        parser.displayStates();
        parser.displayParsingTable();
        
        return 0;
    }