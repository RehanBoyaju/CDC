#include <bits/stdc++.h>
using namespace std;

class Production
{
public:
    char lhs;
    string rhs;

    Production(char lhs, string rhs)
    {
        this->lhs = lhs;
        this->rhs = rhs;
    }
};

bool isTerminal(char c)
{
    return !(c >= 'A' && c <= 'Z');
}

void addProduction(char lhs, string rhs)
{
    Production prod(lhs,rhs);
    grammar.push_back(prod);

    nonTerminals.insert(lhs);


    for (char c : rhs)
    {
        if (isTerminal(c) && c != '#')
        {
            terminals.insert(c);
        }
    }
}

class Item
{
public:
    Production *prod;
    int dot;

    Item(Production *prod, int dot)
    {
        this->prod = prod;
        this->dot = dot;
    }

    bool operator<(const Item &other) const
    {
        if (prod->lhs != other.prod->lhs)
            return prod->lhs < other.prod->lhs;
        else if (prod->rhs != other.prod->rhs)
            return prod->rhs < other.prod->rhs;
        else
        {
            return dot < other.dot;
        }
    }
    bool operator==(const Item &other) const
    {
        return prod->lhs == other.prod->lhs && prod->rhs == other.prod->rhs && dot == other.dot;
    }
};

typedef set<Item> State;
vector<Production> grammar;
set<char> terminals;
set<char> nonTerminals;
map<int,State> stateIds;
vector<State> states;
map<pair<int,char>,int> transitions;//(state,symbol)->next state


set<Item> closure(set<Item> I)
{

    while (true)
    {
        vector<Item> newItems;

        for (auto item : I)
        {
            if (item.dot < item.prod->rhs.size())
            {
                char symbol = item.prod->rhs[item.dot];
                if (nonTerminals.count(symbol))
                {
                    for (auto p : grammar)
                    {
                        if (p.lhs == symbol)
                        {
                            Item newItem(&p, 0);
                            if (I.find(newItem) == I.end())
                            {
                                newItems.push_back(newItem);
                            }
                        }
                    }
                }
            }
        }
        if (newItems.size() == 0)
        {
            break;
        }
        for (auto newItem : newItems)
        {
            I.insert(newItem);
        }
    }
    return I;
}

set<Item> goTo(set<Item> I, char X)
{
    set<Item> newItems;

    for (auto item : I)
    {
        if (item.dot < item.prod->rhs.size() && item.prod->rhs[item.dot] == X)
        {
            Item nextItem = item;
            nextItem.dot++;
            newItems.insert(nextItem);
        }
    }

    return closure(newItems);
}

void generateLR0Items(State state){
    
    states.push_back(state);

    for(auto terminal: terminals){
        auto nextState = goTo(state,terminal);
        if(nextState.size()>0){
            int nextId = stateIds.size();
            stateIds[nextId] = nextState;
            pair<int,char> key = {states.size()-1,terminal};
            transitions[key] = states.size();
            generateLR0Items(nextState);
        }
    } 

    for(auto nonTerminal: nonTerminals){
        auto nextState = goTo(state,nonTerminal);
        if(nextState.size()>0){
            pair<int,char> key = {states.size()-1,nonTerminal};
            transitions[key] = states.size();
            generateLR0Items(nextState);
        }
    } 



}

string modifyInput(string input)
{
    size_t pos = 0;

    while ((pos = input.find("ε", pos)) != string::npos)
    {
        input.replace(pos, string("ε").length(), "#");
        pos += 1;
    }

    return input;
}

string modifyOutput(string output)
{
    size_t pos = 0;

    while ((pos = output.find("#", pos)) != string::npos)
    {
        output.replace(pos, string("#").length(), "ε");
    }

    return output;
}

int main()
{
    int n;
    char startSymbol;

    cout << "Enter the number of productions: ";
    cin >> n;
    cin.ignore();

    for (int i = 0; i < n; i++)
    {
        cout << "Enter the production " << i + 1 << ": ";
        string production;
        getline(cin, production);
        production = modifyInput(production);

        char lhs = production[0];
        string rhs = production.substr(3);

        if (i == 0)
        {
            startSymbol = lhs;
        }

        string temp = "";
        for (char c : rhs)
        {
            if (c == '|')
            {
                addProduction(lhs, temp);
                temp = "";
            }
            else
            {
                temp += c;
            }
        }
        addProduction(lhs, temp);
    }
    
    //Add augmented start symbol
    addProduction('Z',"S");

    Production* startProd = &grammar.back();

    Item startItem(startProd,0);
    State initialState = closure({startItem});
    generateLR0Items(initialState);

    return 0;
}