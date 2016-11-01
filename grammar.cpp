#include "grammar.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
using namespace std;

Grammar::~Grammar() {
	stack<Node *> stk;
	stk.push(root);
	while(!stk.empty()) {
		Node *top = stk.top();
		stk.pop();
		for(Node *child : top->children) {
			stk.push(child);
		}
		delete top;
	}
}

void Grammar::gen_productions() {
	ifstream ifstrm("productions.txt");
	string line, word, left;
	vector<vector<string>> prods;
	vector<string> prod;
	while(getline(ifstrm, line)) {
		istringstream iss(line);
		iss >> left;
		iss >> word; // pass ->
		while(iss >> word) {
			if(word == "|") {
				prods.push_back(prod);
				prod.clear();
			} else {
				prod.push_back(word);
			}
		}
		prods.push_back(prod);
		productions[left] = prods;
		prods.clear();
		prod.clear();
	}
}


void Grammar::build_tree() {
	string startSymbol = "aql_stmt";
	// //debug(cout << endl << endl)
	bool match = check(startSymbol);
	// //debug(cout << "Result: " << match << endl)
	// debug(print_prodStk())
	stack<Node *> stk;
	root = new Node(startSymbol);
	stk.push(root);
	for(int i = prodStk.size()-1; i >= 0; --i) {
		while(stk.top()->name != prodStk[i].first) {
			stk.pop();
		}
		Node *top = stk.top();
		stk.pop();
		for(const string &item : prodStk[i].second) {
			Node *child = new Node(item);
			top->children.push_back(child);
			stk.push(child);
		}
	}
	// bind each leaf with Terminal
	while(!stk.empty()) {
		stk.pop();
	}
	stk.push(root);
	index = 0;
	while(!stk.empty()) {
		Node *top = stk.top();
		stk.pop();
		if(top->children.size() == 0 && top->name != "None") {
			top->terminal = terminals[index];
			index++;
		}
		for(int i = top->children.size()-1; i >= 0; --i) {
			stk.push(top->children[i]);
		}
	}
	//debug(print_tree(root))
}


bool Grammar::check(string str) {
	// debug(cout << "Check(" << str << ")  {" << endl)
	
	if(productions.find(str) == productions.end()) {
		// terminal
		if(translate.at(str) == Terminal::None) {
			// match
			// debug(cout << "MATCH terminal" << endl << "}" << endl)
			return true;
		}
		if(terminals[index].get_type() == translate.at(str)) {
			// match
			// debug(cout << "MATCH terminal" << endl << "}" << endl)
			index++;
			return true;
		}
		// debug(cout << "notMATCH terminal" << endl << "}" << endl)
		return false;
	} else {
		// non terminal
		for(const auto &prod : productions.at(str)) {
			bool match = true;
			int saved_index = index;
			int saved_stk_size = prodStk.size();
			for(const auto &item : prod) {
				// debug(cout << item << " ";)
				if(!check(item)) {
					match = false;
					break;
				}
			}
			if(not match) {
				index = saved_index;
				while(prodStk.size() > saved_stk_size) {
					prodStk.pop_back();
				}
			} else {
				// debug(cout << "MATCH" << endl << "}" << endl)
				prodStk.push_back(make_pair(str, prod));
				return true;
			}
		}
		// debug(cout << "notMATCH" << endl << "}" << endl)
		return false;
	}
}


#ifdef DEBUG
#include <iostream>
void Grammar::print_productions() {
	cout << "print_productions" << endl;
	for(auto & elem : productions) {
		cout << elem.first << " -> ";
		for(auto &item : elem.second) {
			for(auto &word : item) {
				cout << word << " ";
			}
			cout << " | ";
		}
		cout << endl;
	}
}
void Grammar::print_prodStk() {
	cout << "print_prodStk" << endl;
	for(int i = prodStk.size()-1; i >= 0; --i) {
		const auto &prod = prodStk[i];
		cout << prod.first << " -> ";
		for(const auto &elem : prod.second) {
			cout << elem << " ";
		}
		cout << endl;
	}
}
void Grammar::print_tree(Node *node) {
	cout << node->name;
	if(node->children.size()) {
		cout << " {";
	} else {
		Terminal::Type type = node->terminal.get_type();
		if(type == Terminal::ID || type == Terminal::REG) {
			cout << " -> " << node->terminal.get_str();
		} else if(type == Terminal::NUM) {
			cout << " -> " << node->terminal.get_int();
		}
	}
	cout << endl;
	for(Node *child : node->children) {
		print_tree(child);
	}
	if(node->children.size()) {
		cout << "}" << endl;
	}
}
#endif
