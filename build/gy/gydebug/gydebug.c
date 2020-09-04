#include "gydebug.h"

static char* logbuffers;  /* ����logbuffer��������Ⱦ��˫���� */
static char* logwriter; /* ��ǰ��writer */
static char* logwirterbegin; /* ��ǰwriter����ʼ */
static char* logwirterend; /* ��ǰwriter��ĩβ */
static int logbufferindex; /* ��ǰwriterʹ�õ�buffer */
static int logbuffersize; /* ����buffer�Ĵ�С */

static void gydebug_writesizetobuffer(size_t size)
{
	memcpy(logwriter, &size, size);
	logwriter += size;
}

static void gydebug_writestrtobuffer(const char* str, size_t size)
{
	memcpy(logwriter, &str, size);
	logwriter += size;
}

static int gydebug_log(lua_State *L)
{
	int top = lua_gettop(L);
	if (top != 1 && top  != 2)
	{
		luaL_error(L, "Parameter Error!The Number Of Parameter Must Be One Or Two!");
		return 0;
	}

	size_t tagsize = 0;				//tag����
	const char* tag = NULL;		//tag����

	size_t messagesize = 0;		//log����
	const char* message = NULL;			//log����

	if (top == 2)
	{
		tagsize = lua_rawlen(L, 1);
		tag = lua_tolstring(L, 1, &tagsize);
	}
	messagesize = lua_rawlen(L, top);
	message = lua_tolstring(L, top, &messagesize);

	if (logwriter + sizeof(size_t) * 2 + tagsize + messagesize > logwirterend)
	{
		luaL_error(L, "Log Memory Overflow!");
		return 0;
	}

	gydebug_writesizetobuffer(tagsize);
	gydebug_writestrtobuffer(tag, tagsize);
	gydebug_writesizetobuffer(messagesize);
	gydebug_writestrtobuffer(message, messagesize);

	return 1;
}

static void lua_gydebug_swapbuffer()
{
	if (logbufferindex == 0)
	{
		logbufferindex = 1;
		logwriter = logbuffers + logbuffersize;
	}
	else
	{
		logbufferindex = 0;
		logwriter = logbuffers;
	}
	logwirterend = logwriter + logbuffersize;
	logwirterbegin = logwriter;
}

LUALIB_API int lua_gydebug_getstrlensize()
{
	return sizeof(size_t);
}

LUALIB_API char* lua_gydebug_getbuffer()
{
	return logwirterbegin;
}

LUALIB_API size_t lua_gydebug_getbufferlength()
{
	return logwriter - logwirterbegin;
}

/*
** Input: 
		size ����Buffer�Ĵ�С
*/
LUALIB_API int luaopen_gydebug(lua_State *L) {
	// TEMP
	logbuffers = (char*)malloc(sizeof(char) * 10000 * 2);
	logbuffersize = 10000;
	logbufferindex = 1;
	lua_gydebug_swapbuffer();

	luaL_Reg reg[] = {
		{"log", gydebug_log},
		{ NULL, NULL }
	};

	luaL_newlib(L, reg);
	luaL_setfuncs(L, reg, 0);

	return 1;
}