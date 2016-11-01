#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <string>
#include <stack>
#include <vector>
#include <map>

#include "lexer.h"

class Node {
public:
	Node() {};
	Node(std::string name)
		: name(name) {}

	void (*process)(void *arg);
	std::string name;
	Terminal terminal;
	std::vector<Node *> children;
};

class Grammar {
public:
    explicit Grammar(const std::string &statement)
        : lexer(statement), index(0), root(nullptr),
        terminals(lexer.get_terminals()), translate(lexer.get_translate()) {
       	gen_productions();
        build_tree();
    }
    ~Grammar();
    Node * get_tree() {
    	return root;
    }
    #ifdef DEBUG
    void print_productions();
    void print_prodStk();
    void print_tree(Node *node);
    #endif
private:
	void build_tree();
	void gen_productions();
	bool check(std::string str);
    Lexer lexer;
    const std::vector<Terminal> &terminals;
    const std::map<std::string, Terminal::Type> &translate;
    Node *root;
    std::vector<std::pair<std::string, std::vector<std::string>>> prodStk;
    std::map<std::string, std::vector<std::vector<std::string>>> productions;
    int index;
};

#endif
