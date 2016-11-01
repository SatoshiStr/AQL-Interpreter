#include <cctype>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "parser.h"
using namespace std;

vector<vector<int>> findall(const char *regex, const char *content);


void Parser::parse() {
    Node *root = grammar.get_tree();
    if(root->children[0]->name == "create_stmt") {
        Node *node = root->children[0];  // node is create_stmt
        this->viewName = node->children[2]->terminal.get_str();
        node = node->children[4];  // node is view_stmt
        if(node->children[0]->name == "extract_stmt") {
            node = node->children[0];  // node is extract_stmt
            Node *fromList = node->children[3];
            record_from_alias(fromList);
            node = node->children[1]->children[0];
            if(node->name == "regex_spec") {
                solve_regex_spec(node);
            } else if(node->name == "pattern_spec") {
                solve_pattern_spec(node);
            }
        } else if(node->children[0]->name == "select_stmt") {
            solve_select_stmt(node->children[0]);
        }
    } else if(root->children[0]->name == "output_stmt") {
        viewName = root->children[0]->children[2]->terminal.get_str();
        output();
    }
    // debug(output())
}


void Parser::record_from_alias(Node *node) {
    // node is from_list
    // save first 1
    // debug(cout << "from alias " << node->children[0]->children[0]->terminal.get_str()
    //         << " " << node->children[0]->children[1]->terminal.get_str() << endl)
    alias[node->children[0]->children[1]->terminal.get_str()] = 
        node->children[0]->children[0]->terminal.get_str();
    node = node->children[1];  // from_list2
    while(node->children.size() == 3) {
        alias[node->children[1]->children[1]->terminal.get_str()] =
            node->children[1]->children[0]->terminal.get_str();
        node = node->children[2];
    }
}


void Parser::solve_regex_spec(Node *node) {
    // node is regex_spec
    record_field_name(node->children[4]);
    // check column
    Node *column = node->children[3];
    string name = column->children[0]->terminal.get_str();
    string attr = column->children[2]->terminal.get_str();
    if(alias.find(name) != alias.end()) {
        name = alias[name];
    }
    // debug(cout << "name attr: " << name << " " << attr << endl)
    if(name != "Document" || attr != "text") {
        throw logic_error("Extract-regex should be from Document.");
    }
    //
    vector<vector<pair<int, int>>> &records = viewRecords[viewName];
    vector<vector<int>> results = findall(node->children[1]->terminal.get_str().c_str(),
                                          tokenizer.text.c_str());
    // debug(cout << "result size " << results.size() << "  regex "
    //         << node->children[1]->terminal.get_str() << endl)
    for(const auto & result : results) {
        vector<pair<int, int>> record;
        for(int i = 0; i < result.size(); i += 2) {
            if(groupName.find(i/2) != groupName.end()) {
                record.push_back(make_pair(result[i], result[i+1]));
                // debug(cout << "record pair " << result[i] << ", " << result[i+1] << endl)
            }
        }
        records.push_back(record);
    }
}


void Parser::record_field_name(Node *node) {
    // node is name_spec
    if(node->children[0]->terminal.get_type() == Terminal::as) {
        // debug(cout << "as ID :" << node->children[1]->terminal.get_str() << endl)
        groupName[0] = node->children[1]->terminal.get_str();
        viewHeaders[viewName] = groupName;
        return;
    }
    node = node->children[1];  // group_spec
    // save the first 1
    int num = node->children[0]->children[1]->terminal.get_int();
    string name = node->children[0]->children[3]->terminal.get_str();
    groupName[num] = name;
    node = node->children[1];  // group_spec2
    while(node->children.size() == 3) {
        num = node->children[1]->children[1]->terminal.get_int();
        name = node->children[1]->children[3]->terminal.get_str();
        groupName[num] = name;
        // debug(cout << "Field name : " << num << " : " << name << endl)
        node = node->children[2];
    }
    viewHeaders[viewName] = groupName;
}


void Parser::solve_pattern_spec(Node *node) {
    // node is pattern_spec
    record_field_name(node->children[2]);
    //
    vector<vector<pair<int, int>>> records;
    vector<bool> caughts;
    node = node->children[1];  // node is pattern_expr
    while(node->children.size() == 2) {
        Node *patternPkg = node->children[0];
        if(patternPkg->children.size() != 1) {
            // atom { NUM , NUM }
            Node *atom = patternPkg->children[0];
            if(atom->children[1]->terminal.get_type() == Terminal::Token) {
                int min = patternPkg->children[2]->terminal.get_int();
                int max = patternPkg->children[4]->terminal.get_int();
                solve_atom_token(min, max, records);
                caughts.push_back(false);
            }
        } else if(patternPkg->children[0]->name == "atom") {
            Node *atom = patternPkg->children[0];
            vector<vector<pair<int, int>>> tempRecords;
            if(atom->children.size() == 1) {
                solve_atom_reg(atom->children[0], tempRecords);
                if(records.size() == 0) {
                    records = tempRecords;
                } else {
                    merge_view(records, tempRecords);
                }
                caughts.push_back(false);
            }
        } else if(patternPkg->children[0]->name == "pattern_group") {
            solve_pattern_group(patternPkg, records, caughts);
        }
        node = node->children[1];
    }
    //
    vector<vector<pair<int, int>>> &caughtRecord = viewRecords[viewName];
    for(const auto &rec : records) {
        vector<pair<int, int>> vec;
        vec.push_back(make_pair(rec[0].first, rec[rec.size()-1].second));
        for(int i = 0; i < rec.size(); ++i) {
            if(caughts[i]) {
                vec.push_back(rec[i]);                
            }
        }
        caughtRecord.push_back(vec);
    }
}


void Parser::solve_atom_reg(Node *reg, vector<vector<pair<int, int>>> &tempRecords) {
    string regex = reg->terminal.get_str();
    vector<vector<int>> results = findall(regex.c_str(), tokenizer.text.c_str());
    for(const auto &result : results) {
        vector<pair<int, int>> vec;
        for(int i = 0; i < result.size(); i+=2) {
            vec.push_back(make_pair(result[i], result[i+1]));
        }
        tempRecords.push_back(vec);
    }
}


void Parser::solve_atom_column(Node *column, vector<vector<pair<int, int>>> &tempRecords) {
    string name = column->children[0]->terminal.get_str();
    string field = column->children[2]->terminal.get_str();
    if(alias.find(name) != alias.end()) {
        name = alias[name];
    }
    if(viewHeaders.find(name) == viewHeaders.end()) {
        throw logic_error("View " + name + " is not defined.");
    }
    int index = -1;
    for(const auto &item : viewHeaders[name]) {
        if(item.second == field) {
            index = item.first;
            break;
        }
    }
    if(index == -1) {
        throw logic_error("Field "+field+" is not defined.");
    }
    for(const auto &item : viewRecords[name]) {
        vector<pair<int, int>> vec;
        vec.push_back(item[index]);
        tempRecords.push_back(vec);
    }
}


void Parser::solve_atom_token(int min, int max, std::vector<std::vector<std::pair<int, int>>> &records) {
    if(records.size() == 0) {
        throw logic_error("token error");
    }
    vector<std::vector<std::pair<int, int>>> newRecords;
    for(const auto &rec : records) {
        const vector<pair<int, int>> &spans = tokenizer.get_spans();
        int pos = tokenizer.get_pos(rec[rec.size()-1]);
        int upper = pos + max, lower = pos + min;
        if(upper > spans.size() - 1) {
            upper = spans.size() - 1;
        }
        if(lower > spans.size() - 1) {
            continue;
        }
        // debug(cout << "size " << upper << " " << lower << " " << pos << endl)
        int left = spans[pos+1].first;
        for(int i = lower; i <= upper; ++i) {
            int right = spans[i].second;
            vector<pair<int, int>> vec = rec;
            vec.push_back(make_pair(left, right));
            // debug(cout << left << ", " << right << endl)
            newRecords.push_back(vec);
        }
    }
    // debug(print_view(newRecords))
    records = newRecords;
}


void Parser::merge_view(vector<vector<pair<int, int>>> &records, const vector<vector<pair<int, int>>> &tempRecords) {
    vector<vector<pair<int, int>>> newRecords;
    for(const auto &item : records) {
        pair<int, int> lastSpan = item[item.size()-1];
        int pos = tokenizer.get_pos(lastSpan);
        for(const auto &elem : tempRecords) {
            if(tokenizer.get_pos_prev(elem[0]) == pos + 1) {
                vector<pair<int, int>> line = item;
                line.push_back(elem[0]);
                newRecords.push_back(line);
            }
        }
    }
    // debug(cout << "records" << endl)
    // debug(print_view(records))
    // debug(cout << "tempRecords" << endl)
    // debug(print_view(tempRecords))
    // debug(cout << "merge" << endl)
    // debug(print_view(newRecords))
    records = newRecords;
}


void Parser::solve_pattern_group(Node *patternPkg, vector<vector<pair<int, int>>> &records, vector<bool> &caughts) {
    // Assume catch one atom only
    Node *caughtPkg = patternPkg->children[0]->children[1]->children[0];  // pattern_pkg
    vector<vector<pair<int, int>>> tempRecords;
    if(caughtPkg->children.size() == 1 && caughtPkg->children[0]->name == "atom") {
        Node *atom = caughtPkg->children[0];
        if(atom->children.size() == 1) {
            // REG

        } else if(atom->children[1]->name == "column") {
            solve_atom_column(atom->children[1], tempRecords);
        }
        if(records.empty()) {
            records = tempRecords;
        } else {
            merge_view(records, tempRecords);
        }
        caughts.push_back(true);
    }
}


inline int digit_num(int num) {
    int len = 0;
    if(num == 0) return 1;
    while(num) {
        num /= 10;
        len++;
    }
    return len;
}


inline int get_len(const pair<int, int> &span) {
    int len = span.second - span.first;
    return len + digit_num(span.first) + digit_num(span.second) + 4 + 2;
}


inline void print_line(const vector<int> &maxLens) {
    cout << '+';
    for(int len : maxLens) {
        for(int i = 0; i < len; ++i) {
            cout << '-';
        }
        cout << '+';
    }
    cout << endl;
}


void Parser::solve_select_stmt(Node *node) {
    record_from_alias(node->children[3]);
    // Assume select from only one field
    Node *select_item = node->children[1]->children[0];
    vector<vector<pair<int, int>>> records;
    solve_atom_column(select_item, records);
    Node *fieldAlias = select_item->children[3];
    if(fieldAlias->children.size() == 2) {
        groupName[0] = fieldAlias->children[1]->terminal.get_str();
        viewHeaders[viewName] = groupName;
    }
    viewRecords[viewName] = records;
}


void Parser::output() {
    cout << viewName << ":" << endl;
    vector<int> maxLens(viewHeaders[viewName].size(), 0);
    vector<vector<pair<int, int>>> &records = viewRecords[viewName];
    for(const auto &record : records) {
        for(int i = 0; i < record.size(); ++i) {
            int len = get_len(record[i]);
            if(len > maxLens[i]) {
                maxLens[i] = len;
            }
        }
    }
    print_line(maxLens);
    // print field
    cout << '|';
    int cnt = 0;
    for(const auto &field : viewHeaders[viewName]) {
        cout << ' ';
        cout << field.second;
        for(int i = field.second.size(); i < maxLens[cnt]-2; ++i) {
            cout << ' ';
        }
        cnt++;
        cout << " |";
    }
    cout << endl;
    print_line(maxLens);
    // print record
    for(const auto &record : records) {
        cout << '|';
        for(int index = 0; index < record.size(); ++index) {
            cout << ' ';
            const auto &span = record[index];
            cout << tokenizer.text.substr(span.first, span.second-span.first);
            cout << '(' << span.first << ", " << span.second << ')';
            for(int i = get_len(span); i < maxLens[index]; ++i) {
                cout << ' ';
            }
            cout << " |";
        }
        cout << endl;
    }
    print_line(maxLens);
}


MultiStmt::MultiStmt(std::string aqlFile, std::string dataFile)
        : aqlFile(aqlFile), dataFile(dataFile) {
    string line, text;
    ifstream dataStream(dataFile);
    while(getline(dataStream, line)) {
        text += line + "\n";
    }
    tokenizer = Tokenizer(text);
    string statement;
    ifstream aqlStream(aqlFile);
    while(getline(aqlStream, line)) {
        statement += line + '\n';
        int i;
        for(i = line.size()-1; i >= 0; i--) {
            if(!isspace(line[i])) break;
        }
        if(line[i] == ';') {
            statements.push_back(statement);
            statement = "";
        }
    }
}


void MultiStmt::process() {
    int i = 0;
    for(const auto &statement : statements) {
        vector<vector<pair<int, int>>> records;
        Parser parser(statement, tokenizer, viewHeaders, viewRecords);
        parser.parse();
        ++i;
        // if(i == 6)
        //     return;
    }
}


#ifdef DEBUG
void Parser::print_view(const std::vector<std::vector<std::pair<int, int>>> &records) {
    for(auto & rec : records) {
        cout << "| ";
        for(auto &item : rec) {
            cout << " " << tokenizer.text.substr(item.first, item.second-item.first) 
                << " (" << item.first << ", " << item.second << ") |";
        }
        cout << endl;
    }
}
#endif
