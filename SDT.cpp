#include <iostream>
#include <stack>
#include <string>

using namespace std;

// Super simple tree node
struct Node {
    string value;
    Node(string v) : value(v) {}
};

int main() {
    stack<Node*> stack;
    
    // Example: Process "3 + 4 * 2"
    cout << "Processing: 3 + 4 * 2\n\n";
    
    // Step 1: Push numbers
    stack.push(new Node("3"));
    cout << "Pushed: 3\n";
    
    stack.push(new Node("4"));
    cout << "Pushed: 4\n";
    
    stack.push(new Node("2"));
    cout << "Pushed: 2\n";
    
    // Step 2: Process multiplication (4 * 2)
    Node* right = stack.top(); stack.pop();
    Node* left = stack.top(); stack.pop();
    Node* mul = new Node("*");
    cout << "\nCreated * node (4 * 2)\n";
    
    // Step 3: Process addition (3 + result)
    Node* num3 = stack.top(); stack.pop();
    Node* add = new Node("+");
    cout << "Created + node (3 + result)\n";
    
    // Display tree structure
    cout << "\nSyntax Tree:\n";
    cout << "    +\n";
    cout << "   / \\\n";
    cout << "  3   *\n";
    cout << "     / \\\n";
    cout << "    4   2\n";
    
    return 0;
}
