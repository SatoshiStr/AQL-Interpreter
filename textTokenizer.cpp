#include <string>
#include <cctype>
#include <utility>
#include <stdexcept>
#include "textTokenizer.h"
using namespace std;


void Tokenizer::gen_span() {
	int i = 0, left;
	while(i < text.size()) {
		if(isalpha(text[i]) || isdigit(text[i])) {
			left = i;
			while(isalpha(text[i]) || isdigit(text[i])) {
				i++;
			}
			spans.push_back(make_pair(left, i));
		} else if(ispunct(text[i])) {
			spans.push_back(make_pair(i, i+1));
			++i;
		} else if(isspace(text[i])) {
			++i;
		} else {
			throw logic_error("Unknown character: " + string(1, text[i]));
		}
	}
}
