#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <map>
#include "lexer.h"
#include "grammar.h"
#include "textTokenizer.h"

class Parser {
public:
    Parser(const std::string &statement, const Tokenizer &tokenizer,
        std::map<std::string, std::map<int, std::string>> &viewHeaders,
        std::map<std::string, std::vector<std::vector<std::pair<int, int>>>> &viewRecords)
            : grammar(statement), tokenizer(tokenizer), viewHeaders(viewHeaders),
              viewRecords(viewRecords) {}
    void parse();
    #ifdef DEBUG
    void print_view(const std::vector<std::vector<std::pair<int, int>>> &records);
    #endif
private:
    void record_from_alias(Node *node);
    void solve_regex_spec(Node *node);
    void record_field_name(Node *node);
    void solve_pattern_spec(Node *node);
    void solve_pattern_group(Node *patternPkg, std::vector<std::vector<std::pair<int, int>>> &records,
            std::vector<bool> &caughts);
    void solve_atom_column(Node *column, std::vector<std::vector<std::pair<int, int>>> &tempRecords);
    void solve_atom_reg(Node *reg, std::vector<std::vector<std::pair<int, int>>> &tempRecords);
    void solve_atom_token(int min, int max, std::vector<std::vector<std::pair<int, int>>> &records);
    void merge_view(std::vector<std::vector<std::pair<int, int>>> &records,
            const std::vector<std::vector<std::pair<int, int>>> &tempRecords);
    void solve_select_stmt(Node *node);
    void output();
    Grammar grammar;
    const Tokenizer &tokenizer;
    std::string viewName;
    std::map<int, std::string> groupName;
    std::map<std::string, std::string> alias;
    std::map<std::string, std::map<int, std::string>> &viewHeaders;
    std::map<std::string, std::vector<std::vector<std::pair<int, int>>>> &viewRecords;
};


class MultiStmt {
public:
    MultiStmt(std::string aqlFile, std::string dataFile);
    void process();
private:
    Tokenizer tokenizer;
    std::string aqlFile;
    std::string dataFile;
    std::vector<std::string> statements;
    std::map<std::string, std::map<int, std::string>> viewHeaders;
    std::map<std::string, std::vector<std::vector<std::pair<int, int>>>> viewRecords;
};
#endif
