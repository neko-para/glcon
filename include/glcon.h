#ifndef _GLCON_H_
#define _GLCON_H_

#include <wchar.h>

#define GC_RGB(r, g, b) ((unsigned)( \
		((unsigned char)(r)) \
		| (((unsigned char)(g)) << 8) \
		| (((unsigned char)(b)) << 16) \
	))

#define GC_ESCAPE        256
#define GC_INSERT        260
#define GC_DELETE        261
#define GC_RIGHT         262
#define GC_LEFT          263
#define GC_DOWN          264
#define GC_UP            265
#define GC_PAGE_UP       266
#define GC_PAGE_DOWN     267
#define GC_HOME          268
#define GC_END           269
#define GC_CAPS_LOCK     280
#define GC_SCROLL_LOCK   281
#define GC_NUM_LOCK      282
#define GC_PRINT_SCREEN  283
#define GC_PAUSE         284
#define GC_F1            290
#define GC_F2            291
#define GC_F3            292
#define GC_F4            293
#define GC_F5            294
#define GC_F6            295
#define GC_F7            296
#define GC_F8            297
#define GC_F9            298
#define GC_F10           299
#define GC_F11           300
#define GC_F12           301

#ifdef __cplusplus
extern "C" {
#endif

void gcInit(int r, int c);
void gcPutc(wchar_t ch);
void gcPuts(const char* str);
void gcPrintf(const char* format, ...);
void gcAttr(unsigned front, unsigned back);
void gcMove(int r, int c);
int gcGetc();

#ifdef __cplusplus
}
#endif

#endif