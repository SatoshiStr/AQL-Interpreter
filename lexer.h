#ifndef LEXER_H
#define LEXER_H

#ifdef DEBUG
#define debug(statement) statement;
#else
#define debug(str) ;
#endif

#include <string>
#include <vector>
#include <map>


class Terminal {
public:
    enum Type {create, view, as, output, select, from, extract, regex, on,
        return_, group, and_, Token, pattern,
        ID, NUM, REG,
        lParen, rParen, lCurly, rCurly, lAngle, rAngle, comma, semicolon, period,
        None, End};
    Terminal() {}
    Terminal(Type type)
        : type(type) {}
    Terminal(Type type, int value)
        : type(type), int_v(value) {}
    Terminal(Type type, const std::string &value)
        : type(type), str_v(value) {}
    // TODO: Add type check
    std::string get_str() const {
        return str_v;
    }
    void set_str(std::string val) {
        str_v = val;
    }
    int get_int() const {
        return int_v;
    }
    Type get_type() const {
        return type;
    }
private:
    Type type;
    int int_v;
    std::string str_v;
};


class Lexer {
public:
    // Lexer() {}
    Lexer(const std::string &statement)
        : statement(statement) {
        this->process();
    }
    const std::vector<Terminal>& get_terminals() {
        return terminals;
    }
    const std::map<std::string, Terminal::Type> & get_translate() {
        return translate;
    }
    #ifdef DEBUG
    void print_terminals();
    #endif
private:
    void process();
    std::string statement;
    std::map<std::string, Terminal::Type> translate;  // translate word to keyword
    std::vector<Terminal> terminals;
};

#endif
