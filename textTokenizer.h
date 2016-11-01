#ifndef TEXTTOKENIZER_H
#define TEXTTOKENIZER_H

#include <string>
#include <vector>
#include <utility>


class Tokenizer {
public:
	Tokenizer()=default;
	Tokenizer(std::string text)
		: text(text) {
		gen_span();
	}
	// Tokenizer& operator=(Tokenizer&&)=default;
	const std::vector<std::pair<int, int>> & get_spans() const {
		return spans;
	}
	int get_pos(std::pair<int, int> span) const {
		int min = 0, max = spans.size()-1, mid = (min + max) / 2;
		while(min <= max) {
			if(span.second == spans[mid].second) {
				return mid;
			} else if(span.second < spans[mid].second) {
				max = mid -1;
			} else {
				min = mid + 1;
			}
			mid = (max+min) / 2;
		}
		return -1;
	}
	int get_pos_prev(std::pair<int, int> span) const {
		int min = 0, max = spans.size()-1, mid = (min + max) / 2;
		while(min <= max) {
			if(span.first == spans[mid].first) {
				return mid;
			} else if(span.first < spans[mid].first) {
				max = mid -1;
			} else {
				min = mid + 1;
			}
			mid = (max+min) / 2;
		}
		return -1;
	}
	std::string text;
private:
	void gen_span();
	std::vector<std::pair<int, int>> spans;
};
#endif
