#include "../include/font.h"
#include "../include/sharedptr.h"
#include <map>
#include <set>
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

static FT_Library lib;
static FT_Face face;

inline void SetColor(unsigned c) {
	glColor3f((c & 0xFF) / 255.0, ((c >> 8) & 0xFF) / 255.0, ((c >> 16) & 0xFF) / 255.0);
}

class Index {
	FT_UInt index;
	Index(const Index& ) {}
	Index& operator=(const Index& ) {
		return *this;
	}

public:
	explicit Index(wchar_t ch) {
		index = FT_Get_Char_Index(face, ch);
	}
	FT_UInt get() const {
		return index;
	}
};

typedef SharedPtr<Index> IndexPtr;

static std::map<wchar_t, IndexPtr> indexs;
static FontInfo info;

class Char {
	int ref;
	TextTex tex;
	Char(const Char& ) {}
	Char& operator=(const Char& ) {
		return *this;
	}

public:
	Char(wchar_t ch) {
		ref = 0;
		if (indexs.find(ch) == indexs.end()) {
			indexs[ch] = IndexPtr(new Index(ch));
		}
		FT_Load_Glyph(face, indexs[ch]->get(), FT_LOAD_DEFAULT);
		FT_Glyph glyph;
		FT_Get_Glyph(face->glyph, &glyph);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
		FT_Bitmap& bitmap = bitmap_glyph->bitmap;
		GLubyte* data = new GLubyte[bitmap.width * bitmap.rows * 4];
		memset(data, 0xFF, bitmap.width * bitmap.rows * 4);
		for (unsigned i = 0; i < bitmap.rows; ++i) {
			for (unsigned j = 0; j < bitmap.width; ++j) {
				data[(((bitmap.rows - i - 1) * bitmap.width + j) << 2) | 3] = bitmap.buffer[i * bitmap.width + j];
			}
		}
		FT_Glyph_Metrics& metrics = face->glyph->metrics;
		tex.width = metrics.width / 64.0;
		tex.height = metrics.height / 64.0;
		tex.bearx = metrics.horiBearingX / 64.0;
		tex.beary = metrics.horiBearingY / 64.0;
		tex.advance = metrics.horiAdvance / 64.0;
		glGenTextures(1, &tex.tex);
		glBindTexture(GL_TEXTURE_2D, tex.tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.width, bitmap.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		delete data;
		tex.lst = glGenLists(1);
		glNewList(tex.lst, GL_COMPILE);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0);
		glVertex2d(tex.bearx, tex.beary - tex.height - info.descender);
		glTexCoord2d(1, 0);
		glVertex2d(tex.bearx + tex.width, tex.beary - tex.height - info.descender);
		glTexCoord2d(1, 1);
		glVertex2d(tex.bearx + tex.width, tex.beary - info.descender);
		glTexCoord2d(0, 1);
		glVertex2d(tex.bearx, tex.beary - info.descender);
		glEnd();
		glEndList();
	}
	~Char() {
		glDeleteTextures(1, &tex.tex);
		glDeleteLists(tex.lst, 1);
	}
	int getRef() const {
		return ref;
	}
	TextTex& get() {
		++ref;
		return tex;
	}
};

inline bool operator<(const Char& a, const Char& b) {
	return a.getRef() < b.getRef();
}

typedef SharedPtr<Char> CharPtr;

static std::map<wchar_t, CharPtr> tex;

void TextTex::Draw(unsigned color) const {
	glBindTexture(GL_TEXTURE_2D, tex);
	SetColor(color);
	glCallList(lst);
	glBindTexture(GL_TEXTURE_2D, 0);
}

FontInfo fontInit(const char* font, int dpi) {
	FT_Init_FreeType(&lib);
	FT_New_Face(lib, font, 0, &face);
	FT_Set_Char_Size(face, 0, 16 << 6, dpi, dpi);
	FT_Size_Metrics& metrics = face->size->metrics;
	info.ascender = metrics.ascender / 64.0;
	info.descender = metrics.descender / 64.0;
	info.height = metrics.height / 64.0;
	info.advance = metrics.max_advance / 64.0 / 2;
	return info;
}

TextTex fontGet(wchar_t ch) {
	std::map<wchar_t, CharPtr>::iterator it = tex.find(ch);
	if (it != tex.end()) {
		return it->second->get();
	} else {
		if (tex.size() > 256) {
			std::set<std::pair<CharPtr, wchar_t> > st;
			for (it = tex.begin(); it != tex.end(); ++it) {
				st.insert(std::pair<CharPtr, wchar_t>(it->second, it->first));
				if (st.size() > 64) {
					st.erase(--st.end());
				}
			}
			for (std::set<std::pair<CharPtr, wchar_t> >::iterator i = st.begin(); i != st.end(); ++i) {
				tex.erase(i->second);
			}
		}
		CharPtr p(new Char(ch));
		tex.insert(std::pair<wchar_t, CharPtr>(ch, p));
		return p->get();
	}
}