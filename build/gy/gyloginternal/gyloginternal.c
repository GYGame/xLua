#include "gyloginternal.h"

static char* logbuffers;  /* 两个logbuffer，类似渲染的双缓冲 */
static char* logwriter; /* 当前的writer */
static char* logwirterbegin; /* 当前writer的起始 */
static char* logwirterend; /* 当前writer的末尾 */
static int logbufferindex; /* 当前writer使用的buffer */
static int logbuffersize; /* 单个buffer的大小 */

static void gyloginternal_writesizetobuffer(size_t size)
{
	memcpy(logwriter, &size, SIZET_LENGTH);
	logwriter += SIZET_LENGTH;
}

static void gyloginternal_writestrtobuffer(const char* str, size_t size)
{
	memcpy(logwriter, str, size);
	logwriter += size;
}

static int gyloginternal_writelog(lua_State *L, size_t type)
{
	//gyloginternal.log("a", debug.traceback("sdf"))

	int top = lua_gettop(L);
	if (top != 1 && top != 2)
	{
		luaL_error(L, "Parameter Error!The Number Of Parameter Must Be One Or Two!");
		return 0;
	}

	// tag长度
	size_t tagsize = 0;
	// tag内容
	const char* tag = NULL;

	// log长度
	size_t messagesize = 0;
	// log内容
	const char* message = NULL;

	if (top == 2)
	{
		tagsize = lua_rawlen(L, 1);
		tag = lua_tolstring(L, 1, &tagsize);
	}
	messagesize = lua_rawlen(L, top);
	message = lua_tolstring(L, top, &messagesize);

	if (logwriter + sizeof(size_t) * 3 + tagsize + messagesize > logwirterend)
	{
		luaL_error(L, "Log Memory Overflow!");
		return 0;
	}

	gyloginternal_writesizetobuffer(type);
	gyloginternal_writesizetobuffer(tagsize);
	gyloginternal_writestrtobuffer(tag, tagsize);
	gyloginternal_writesizetobuffer(messagesize);
	gyloginternal_writestrtobuffer(message, messagesize);

	return 1;
}

static int gyloginternal_log(lua_State *L)
{
	return gyloginternal_writelog(L, TYPE_LOG);
}

static int gyloginternal_warning(lua_State *L)
{
	return gyloginternal_writelog(L, TYPE_WARING);
}

static int gyloginternal_error(lua_State *L)
{
	return gyloginternal_writelog(L, TYPE_ERROR);
}

static int gyloginternal_verbose(lua_State *L)
{
	return gyloginternal_writelog(L, TYPE_VERBOSE);
}

LUALIB_API void lua_gyloginternal_swapbuffer()
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

LUALIB_API int lua_gyloginternal_getsizetlength()
{
	return SIZET_LENGTH;
}

LUALIB_API char* lua_gyloginternal_getbuffer()
{
	return logwirterbegin;
}

LUALIB_API size_t lua_gyloginternal_getbufferlength()
{
	return logwriter - logwirterbegin;
}

/*
** Input:
*/
LUALIB_API int luaopen_gyloginternal(lua_State *L)
{
	logbuffers = (char*)malloc(sizeof(char) * 10000 * 2);
	logbuffersize = 10000;
	logbufferindex = 1;
	lua_gyloginternal_swapbuffer();

	luaL_Reg reg[] = {
		{"log", gyloginternal_log},
		{"warning", gyloginternal_warning},
		{"error", gyloginternal_error},
		{"verbose", gyloginternal_verbose},
		{ NULL, NULL }
	};

	luaL_newlib(L, reg);
	luaL_setfuncs(L, reg, 0);

	return 1;
}