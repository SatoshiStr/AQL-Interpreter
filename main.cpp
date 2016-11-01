#include "lexer.h"
#include "grammar.h"
#include "textTokenizer.h"
#include "parser.h"
#include <string>
#include <iostream>
using namespace std;


int main() {
    MultiStmt ms("dataset/PerLoc.aql", "dataset/perloc/PerLoc.input");
    ms.process();
}
