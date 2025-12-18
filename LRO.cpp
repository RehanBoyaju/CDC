#include<bits/stdc++.h>
using namespace std;


class Production{
    public:
    char lhs;
    string rhs;

    Production(char lhs,string rhs){
        this->lhs = lhs;
        this->rhs = rhs;
    }

};


bool isTerminal(char c){
    return !(c>='A' && c<='Z');
}

void addProduction(char lhs,string rhs){

    grammar.push_back({lhs,rhs});

    nonTerminals.insert(lhs);

    for(char c:rhs){
        if(isTerminal(c) && c!='#'){
            terminals.insert(c);
        }
    }
}
class Item{
    Production* prod;
    int dot;

    Item(Production* prod,int dot){
        this->prod = prod;
        this->dot = dot;
    }

    bool operator<(const Item &other) const{
        if(prod->lhs != other.prod->lhs) return prod->lhs<other.prod->lhs;
        else if(prod->rhs != other.prod->rhs) return prod->rhs<other.prod->rhs;
        else{
            return dot<other.dot;
        }
    }
    bool operator==(const Item &other) const{
        return prod->lhs == other.prod->lhs && prod->rhs == other.prod->rhs && dot == other.dot;
    }

};




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

set<Item> state;
vector<Production> grammar;
set<char> terminals;
set<char> nonTerminals;

int main(){
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
                addProduction(lhs,temp);
                temp = "";
            } else {
                temp += c;
            }
        }
        addProduction(lhs,temp);
    }
    

    return 0;
}