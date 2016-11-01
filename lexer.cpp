#include "lexer.h"
#include <cctype>
#include <string>
#include <map>
#include <vector>

#include <iostream>

using namespace std;


void Lexer::process() {
    // Init translate table
    translate["create"] = Terminal::create;
    translate["view"] = Terminal::view;
    translate["as"] = Terminal::as;
    translate["output"] = Terminal::output;
    translate["select"] = Terminal::select;
    translate["from"] = Terminal::from;
    translate["extract"] = Terminal::extract;
    translate["regex"] = Terminal::regex;
    translate["on"] = Terminal::on;
    translate["return"] = Terminal::return_;
    translate["group"] = Terminal::group;
    translate["and"] = Terminal::and_;
    translate["Token"] = Terminal::Token;
    translate["pattern"] = Terminal::pattern;
    translate["("] = Terminal::lParen;
    translate[")"] = Terminal::rParen;
    translate["<"] = Terminal::lAngle;
    translate[">"] = Terminal::rAngle;
    translate["{"] = Terminal::lCurly;
    translate["}"] = Terminal::rCurly;
    translate[","] = Terminal::comma;
    translate[";"] = Terminal::semicolon;
    translate["."] = Terminal::period;
    //
    translate["ID"] = Terminal::ID;
    translate["REG"] = Terminal::REG;
    translate["NUM"] = Terminal::NUM;
    translate["None"] = Terminal::None;
    //
    int i = 0;
    while(i < statement.size()) {
        if(isspace(statement[i])) {
            ++i;
        } else if(isdigit(statement[i])) {
            // a digit
            int num = 0;
            while(isdigit(statement[i])) {
                num = num * 10 + statement[i] - '0';
                ++i;
            }
            terminals.push_back(Terminal(Terminal::NUM, num));
        } else if(isalpha(statement[i])) {
            string str;
            while(isalpha(statement[i]) || isdigit(statement[i])) {
                str += statement[i];
                ++i;
            }
            if(translate.find(str) != translate.end()) {
                // keyword
                terminals.push_back(Terminal(translate[str]));
            } else {
                // name
                terminals.push_back(Terminal(Terminal::ID, str));
            }
        } else if(statement[i] == '/') {
            // regex
            string str;
            ++i;
            while(!(statement[i] == '/' && statement[i-1] != '\\')) {
                str += statement[i];
                ++i;
            }
            terminals.push_back(Terminal(Terminal::REG, str));
            ++i;
        } else if(ispunct(statement[i])) {
            string punct = statement.substr(i, 1);
            if(translate.find(punct) != translate.end()) {
                terminals.push_back(Terminal(translate[punct]));
            }
            ++i;
        }
    }
}


#ifdef DEBUG
void Lexer::print_terminals() {
    map<Terminal::Type, std::string> mp;
    for(const auto &ele : translate) {
        mp[ele.second] = ele.first;
    }
    for(const auto &ele : terminals) {
        if(mp.find(ele.get_type()) != mp.end()) {  // is keyword or punctuation
            printf("%s ", mp[ele.get_type()].c_str());
            if(ele.get_type() == Terminal::semicolon)
                printf("\n");
        } else {
            if(ele.get_type() == Terminal::NUM) {
                printf("int->%d ", ele.get_int());
            } else {
                printf("str->%s ", ele.get_str().c_str());
            }
        }
    }
}
#endif
