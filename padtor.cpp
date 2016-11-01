#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

int main(int argc, char **argv) {
	if(argc != 2) {
		cout << "usage error" << endl;
		return 0;
	}
	char *filename = argv[1];
	ifstream file(filename);
	string line;
	vector<string> article;
	while(getline(file, line)) {
		article.push_back(line);
	}
	file.close();
	//
	int path = 4, pad = 0;
	ofstream ofs(filename);
	for(auto &line : article) {
		string padding;
		for(int i = 0; i < pad; ++i)
			padding += ' ';
		line = padding + line;
		if(line[line.size()-1] == '{')
			pad += path;
		if(line[line.size()-1] == '}') {
			line = line.substr(4, line.size());
			pad -= path;
		}
		ofs << line << endl;
	}
	return 0;
}
