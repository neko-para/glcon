#ifndef _FONT_H_
#define _FONT_H_

#include <GL/gl.h>

struct TextTex {
	GLuint tex;
	GLuint lst;
	float bearx, beary;
	float width, height;
	float advance;
	void Draw(unsigned color) const;
};

struct FontInfo {
	float ascender;
	float descender;
	float height;
	float advance;
};

FontInfo fontInit(const char* font, int dpi);
TextTex fontGet(wchar_t ch);

#endif