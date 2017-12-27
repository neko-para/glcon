#include "../include/utfstr.h"

inline unsigned Low6bit(unsigned c) {
	return c & 63;
}

Utf16 Utf8::to16() const {
	Utf16 utf16;
	const unsigned char* up = (const unsigned char*)c_str();
	for (size_t p = 0; p < size(); ++p) {
		if (!(up[p] & (1 << 7))) {
			utf16.push_back(up[p]);
		} else if (((up[p] >> 5) & 7) == 6) {
			utf16.push_back(((1 << 11) - 1) & ((up[p] << 6) | Low6bit(up[p + 1])));
			p += 1;
		} else {
			utf16.push_back(((1 << 16) - 1) & ((up[p] << 12) | (Low6bit(up[p + 1]) << 6) | Low6bit(up[p + 2])));
			p += 2;
		}
	}
	return utf16;
}

Utf8 Utf16::to8() const {
	Utf8 str;
	const wchar_t* ptr = c_str();
	const wchar_t* end = ptr + length();
	while (ptr != end) {
		if (*ptr < (1 << 7)) {
			str.push_back(*ptr);
		} else if (*ptr < (1 << 11)) {
			str.push_back((3 << 6) | (*ptr >> 6));
			str.push_back((1 << 7) | Low6bit(*ptr));
		} else {
			str.push_back((7 << 5) | (*ptr >> 12));
			str.push_back((1 << 7) | Low6bit(*ptr >> 6));
			str.push_back((1 << 7) | Low6bit(*ptr));
		}
		++ptr;
	}
	return str;
}