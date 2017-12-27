#include "../include/glcon.h"
#include "../include/utfstr.h"
#include "../include/font.h"
#include "../include/threadutil.h"
#include "../include/config.h"
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <pthread.h>
#include <queue>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
struct CHAR {
	unsigned front, back;
	wchar_t ch;
};

static int inited = 0;
static pthread_t thread;
static Mutex lock;
static Cond input;
static int row, column;
static CHAR** buffer;
static int prow, pcolumn;
static FontInfo fontinfo;
static unsigned front = 0, back = 0xFFFFFF;
static GLuint boxlst;
static std::queue<int> input_chars;

inline void SetColor(unsigned c) {
	glColor3f((c & 0xFF) / 255.0, ((c >> 8) & 0xFF) / 255.0, ((c >> 16) & 0xFF) / 255.0);
}

static void CharCallback(GLFWwindow*, unsigned k) {
	MutexLock locker(input.GetLock());
	input_chars.push(k);
	if (input_chars.size() == 1) {
		input.SignalAll();
	}
}

static void KeyCallback(GLFWwindow*, int key, int, int act, int) {
	MutexLock locker(input.GetLock());
	if (act != GLFW_RELEASE) {
		int k = key;
		switch(key) {
		case GLFW_KEY_ENTER:
			k = '\n';
			break;
		case GLFW_KEY_TAB:
			k = '\t';
			break;
		case GLFW_KEY_BACKSPACE:
			k = '\b';
			break;
		}
		input_chars.push(k);
		if (input_chars.size() == 1) {
			input.SignalAll();
		}
	}
}

static void* CoreLoop(void*) {
	glfwInit();
	fontinfo = fontInit(_FONT_PATH_, 96);
	buffer = new CHAR*[row];
	for (int i = 0; i < row; ++i) {
		buffer[i] = new CHAR[column];
		memset(buffer[i], 0, column * sizeof(CHAR));
	}
	lock.Unlock();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* con = glfwCreateWindow(column * fontinfo.advance, row * fontinfo.height, "", 0, 0);
	glfwSetCharCallback(con, CharCallback);
	glfwSetKeyCallback(con, KeyCallback);
	glfwMakeContextCurrent(con);
	glfwSwapInterval(2);
	glLoadIdentity();
	glOrtho(0, column * fontinfo.advance, 0, row * fontinfo.height, -1, 1);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1, 1, 1, 1);
	boxlst = glGenLists(1);
	glNewList(boxlst, GL_COMPILE);
	glBegin(GL_QUADS);
	glVertex2d(0, 0);
	glVertex2d(0 + fontinfo.advance, 0);
	glVertex2d(0 + fontinfo.advance, fontinfo.height);
	glVertex2d(0, fontinfo.height);
	glEnd();
	glEndList();
	while (!glfwWindowShouldClose(con)) {
		glClear(GL_COLOR_BUFFER_BIT);
		for (int i = 0; i < row; ++i) {
			for (int j = 0; j < column; ++j) {
				glPushMatrix();
				glTranslated(j * fontinfo.advance, (row - i - 1) * fontinfo.height, 0);
				SetColor(buffer[i][j].back);
				glCallList(boxlst);
				glPopMatrix();
			}
		}
		for (int i = 0; i < row; ++i) {
			for (int j = 0; j < column; ++j) {
				glPushMatrix();
				glTranslated(j * fontinfo.advance, (row - i - 1) * fontinfo.height, 0);
				if (buffer[i][j].ch) {
					fontGet(buffer[i][j].ch).Draw(buffer[i][j].front);
					if (buffer[i][j].ch >= 128) {
						++j;
					}
				}
				glPopMatrix();
			}
		}
		glfwSwapBuffers(con);
		glfwPollEvents();
	}
	glfwDestroyWindow(con);
	glfwTerminate();
	return 0;
}

extern "C" void gcInit(int r, int c) {
	assert(!inited);
	inited = 1;
	row = r;
	column = c;
	lock.Lock();
	pthread_create(&thread, 0, CoreLoop, 0);
}

extern "C" void gcPutc(wchar_t ch) {
	MutexLock locker(lock);
	if (ch < 128) {
		switch(ch) {
		case '\n':
			++prow;
			pcolumn = 0;
			break;
		case '\r':
			pcolumn = 0;
			break;
		case '\t':
			pcolumn += (4 - (pcolumn & 3)) & 3;
			break;
		default:
			buffer[prow][pcolumn].ch = ch;
			buffer[prow][pcolumn].front = front;
			buffer[prow][pcolumn].back = back;
			if (++pcolumn == column) {
				++prow;
				pcolumn = 0;
			}
		}
	} else {
		if (pcolumn + 1 == column) {
			++prow;
			pcolumn = 0;
		}
		buffer[prow][pcolumn].ch = ch;
		buffer[prow][pcolumn].front = front;
		buffer[prow][pcolumn].back = back;
		++pcolumn;
		buffer[prow][pcolumn].front = front;
		buffer[prow][pcolumn].back = back;
		if (++pcolumn == column) {
			++prow;
			pcolumn = 0;
		}
	}
}

static void gcPuts16(const wchar_t* str) {
	while (*str) {
		gcPutc(*str++);
	}
}

extern "C" void gcPuts(const char* str) {
	gcPuts16(Utf8(str).to16().c_str());
}

extern "C" void gcPrintf(const char* format, ...) {
	va_list ap;
	va_start(ap, format);
	static char buffer[1 << 16];
	vsnprintf(buffer, 1 << 16, format, ap);
	va_end(ap);
	gcPuts(buffer);
}

extern "C" void gcAttr(unsigned f, unsigned b) {
	front = f;
	back = b;
}

extern "C" void gcMove(int r, int c) {
	prow = r;
	pcolumn = c;
}

extern "C" int gcGetc() {
	MutexLock locker(input.GetLock());
	while (input_chars.size() == 0) {
		input.Wait();
	}
	int ret = input_chars.front();
	input_chars.pop();
	return ret;
}