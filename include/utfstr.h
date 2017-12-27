#ifndef _UTF_STR_H_
#define _UTF_STR_H_

#include <string>

class Utf8;
class Utf16;

class Utf16 : public std::wstring {
public:
	Utf16(const wchar_t* str = L"") : std::wstring(str) {}
	Utf16(wchar_t ch, size_t n = 1) : std::wstring(n, ch) {}
	explicit Utf16(const Utf8& str);
	Utf8 to8() const;	
};

class Utf8 : public std::string {
public:
	Utf8(const char* str = "") : std::string(str) {}
	Utf8(char ch, size_t n = 1) : std::string(n, ch) {}
	explicit Utf8(const Utf16& str);
	Utf16 to16() const;
};

inline Utf16::Utf16(const Utf8& str) {
	*this = str.to16();
}

inline Utf8::Utf8(const Utf16& str) {
	*this = str.to8();
}

#endif