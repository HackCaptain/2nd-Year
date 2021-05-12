#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"

using std::cerr;
using std::cout;
using std::ifstream;
using std::string;
using std::vector;

void die(const string& msg) {
	cerr << "Error: " << msg << "\n";
#ifdef _DEBUG
	abort();
#else
	exit(1);
#endif
}

void load_file(const string& filename, string& str) {

	// To make this program less fussy about where exactly it's run
	// from relative to the file, try looking in parent directories too.

	std::string directory = "";
	for (int i = 0; i < 6; i++) {
		ifstream f(directory + filename, std::ios_base::binary);
		if (!f.good()) {
			directory = "../" + directory;
			continue;
		}

		// Seek to the end of the file to find its length.
		f.seekg(0, std::ios_base::end);
		const size_t length = f.tellg();

		// Seek back to the start of the file and read the data.
		vector<char> buf(length);
		f.seekg(0);
		f.read(buf.data(), length);
		str.assign(buf.begin(), buf.end());

		return;
	}

	die("Unable to find " + filename);
}

void show_context(const string& str, Position pos) {
	const int width = 76;
	Position left = pos - (width / 2);
	Position right = pos + (width / 2);
	Position len = str.size();
	for (Position i = left; i < right; ++i) {
		if (i < 0 || i >= len) {
			cout << ' ';
			continue;
		}
		char c = str[i];
		if (c >= 32 && c < 128) {
			cout << c;
		}
		else {
			// Show control characters as @s.
			cout << '@';
		}
	}
	cout << '\n';
	for (Position i = left; i < right; ++i) {
		if (i < pos) {
			cout << ' ';
		}
		else if (i == pos) {
			cout << "^ " << pos;
		}
	}
	cout << '\n';
}