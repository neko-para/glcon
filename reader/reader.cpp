#include "../include/glcon.h"
#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

#define Low6bit(n) ((n) & 63)

inline vector<int> parsestr(const string& utf8) {
	vector<int> utf16;
	const unsigned char* up = (const unsigned char*)utf8.c_str();
	for (size_t p = 0; p < utf8.size(); ++p) {
		if (!(up[p] & (1 << 7))) {
			utf16.push_back(up[p]);
		} else if (((up[p] >> 5) & 7) == 6) {
			utf16.push_back(((1 << 11) - 1) & ((up[p] << 6) | Low6bit(up[p + 1])));
			p += 1;
		} else {
			utf16.push_back(((up[p] << 12) | (Low6bit(up[p + 1]) << 6) | Low6bit(up[p + 2])));
			p += 2;
		}
	}
	return utf16;
}

inline void put16(int val) {
	if (val < (1 << 7)) {
		putchar(val);
	} else if (val < (1 << 11)) {
		putchar((3 << 6) | (val >> 6));
		putchar((1 << 7) | Low6bit(val));
	} else {
		putchar((7 << 5) | (val >> 12));
		putchar((1 << 7) | Low6bit(val >> 6));
		putchar((1 << 7) | Low6bit(val));
	}
}

inline string restorestr(const vector<int>& utf16) {
	string utf8;
	for (size_t p = 0; p < utf16.size(); ++p) {
		if (utf16[p] < (1 << 7)) {
			utf8.push_back(utf16[p]);
		} else if (utf16[p] < (1 << 11)) {
			utf8.push_back((3 << 6) | (utf16[p] >> 6));
			utf8.push_back((1 << 7) | Low6bit(utf16[p]));
		} else {
			utf8.push_back((7 << 5) | (utf16[p] >> 12));
			utf8.push_back((1 << 7) | Low6bit(utf16[p] >> 6));
			utf8.push_back((1 << 7) | Low6bit(utf16[p]));
		}
	}
	return utf8;
}

string dir;
int npsg;
int psg;
int base;
vector<string> file;

int maxi_row;
int maxi_col;
int pad_row = 2;
int pad_col = 5;

bool exist(const string& path) {
	return access(path.c_str(), 4) != -1;
}

string itoa(int v) {
	string s;
	do {
		s += char((v % 10) ^ '0');
		v /= 10;
	} while (v);
	reverse(s.begin(), s.end());
	return s;
}

int testpsg(string dir) {
	for (int i = 0; ; ++i) {
		if (!exist(dir + "/" + itoa(i))) {
			return i;
		}
	}
}

void saveconfig() {
	ofstream fout((dir + "/cfg").c_str());
	fout << npsg << ' ' << psg << ' ' << base;
}

void loadconfig() {
	if (!exist(dir + "/cfg")) {
		npsg = testpsg(dir);
		psg = 0;
		base = 0;
		saveconfig();
	}
	ifstream fin((dir + "/cfg").c_str());
	fin >> npsg >> psg >> base;
}

void loadfile() {
	file.clear();
	ifstream fin((dir + "/" + itoa(psg)).c_str());
	string row;
	vector<int> row16;
	while (getline(fin, row) && row != "") {
		row16 = parsestr(row);
		int s = 0, p = 0;
		int c = 0;
		while (p != row16.size()) {
			if (c >= maxi_col - pad_col * 2) {
				file.push_back(restorestr(vector<int>(row16.begin() + s, row16.begin() + p)) + string(maxi_col - pad_col - c, ' '));
				s = p;
				c = 0;
			} else {
				++c;
				if (row16[p] == '\t') {
					c += 3;
				}
				c += (row16[p++] > 127);
			}
		}
		if (s != p) {
			file.push_back(restorestr(vector<int>(row16.begin() + s, row16.begin() + p)) + string(maxi_col - pad_col - c, ' '));
		}
	}
}

void showfile(int base) {
	int r = min(maxi_row - pad_row * 2, (int)file.size() - base);
	int i;
	for (i = 0; i < r; ++i) {
		gcMove(i + pad_row, 0);
		gcPrintf("%s%s", string(pad_col, ' ').c_str(), file[base + i].c_str());
	}
	while (i < maxi_row - pad_row * 2) {
		gcMove(i + pad_row, 0);
		gcPrintf("%s", string(maxi_col, ' ').c_str());
		++i;
	}
	int cnt;
	cnt = maxi_col * psg / npsg;
	gcMove(0, 0);
	gcAttr(0xFFFFFF, 0xFF0000);
	gcPrintf("%s", string(cnt, ' ').c_str());
	gcAttr(0xFFFFFF, 0);
	gcPrintf("%s", string(maxi_col - cnt, ' ').c_str());
	cnt = maxi_col * base / file.size();
	gcMove(maxi_row - pad_row + 1, 0);
	gcAttr(0xFFFFFF, 0xFF0000);
	gcPrintf("%s", string(cnt, ' ').c_str());
	gcAttr(0xFFFFFF, 0);
	gcPrintf("%s", string(maxi_col - cnt, ' ').c_str());
	fflush(stdout);
}

int main(int argc, char* argv[]) {
	gcInit(24, 80);
	gcAttr(0xFFFFFF, 0);
	maxi_row = 24;
	maxi_col = 80;
	dir = argv[1];
	loadconfig();
	loadfile();
	for (int i = 0; i < pad_row; ++i) {
		gcMove(i, 0);
		gcPrintf("%s", string(maxi_col, ' ').c_str());
		gcMove(maxi_row - i - 1, 0);
		gcPrintf("%s", string(maxi_col, ' ').c_str());
	}
	int act;
	while (true) {
		showfile(base);
		act = gcGetc();
		switch(act) {
			case 'r':
				loadfile();
				break;
			case GC_UP:
			case 'k':
				base = max(base - 1, 0);
				break;
			case GC_DOWN:
			case 'j':
				base = min(base + 1, (int)file.size() - 1);
				break;
			case GC_LEFT:
			case 'h':
				psg = max(psg - 1, 0);
				loadfile();
				base = 0;
				break;
			case GC_RIGHT:
			case 'l':
				psg = min(psg + 1, npsg - 1);
				loadfile();
				base = 0;
				break;
			case GC_PAGE_UP:
				if (base == 0) {
					psg = max(psg - 1, 0);
					loadfile();
					base = file.size() - 1;
				} else {
					base = max(base - maxi_row + pad_row * 2, 0);
				}
				break;
			case GC_PAGE_DOWN:
				if (base == file.size() - 1) {
					psg = min(psg + 1, npsg - 1);
					loadfile();
					base = 0;
				} else {
					base = min(base + maxi_row - pad_row * 2, (int)file.size() - 1);
				}
				break;
			case GC_HOME:
				base = 0;
				break;
			case GC_END:
				base = file.size() - 1;
				break;
			case 'q':
				saveconfig();
				return 0;
		}
	}
}