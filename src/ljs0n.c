#include <lua.h>
#include <lauxlib.h>
#include "js0n.h"

#if LUA_VERSION_NUM == 501
int lua_isinteger(lua_State *L, int index)
{
    if (lua_type(L, index) == LUA_TNUMBER)
    {
        lua_Number n = lua_tonumber(L, index);
        lua_Integer i = lua_tointeger(L, index);
        if (i == n)
            return 1;
    }
    return 0;
}
#endif

#ifndef luaL_newlibtable
#define luaL_newlibtable(L, l) \
    (lua_createtable((L), 0, sizeof((l)) / sizeof(*(l)) - 1))
#endif

#ifndef luaL_newlib
#define luaL_newlib(L, l) \
    (luaL_newlibtable((L), (l)), luaL_register((L), NULL, (l)))
#endif

static int lreset(lua_State *L)
{
    size_t jlen;
    int joffset = 0;
    status_t *s = (status_t *)luaL_checkudata(L, 1, "js0n_mt");
    const char *json = luaL_checklstring(L, 2, &jlen);
    size_t top = lua_gettop(L);
    if (top >= 3)
    {
        joffset = luaL_checkinteger(L, 3) - 1;
        if (joffset < 0)
            joffset = 0;
    }
    if (top >= 4)
    {
        jlen = luaL_checkinteger(L, 4);
    }

    js0n_init(s, json, jlen, joffset);
    return 0;
}

static int lrun(lua_State *L)
{
    const char *key;
    size_t vlen = 0, klen;
    status_t *s = (status_t *)luaL_checkudata(L, 1, "js0n_mt");
    if (lua_isinteger(L, 2))
    {
        key = 0;
        klen = luaL_checkinteger(L, 2);
    }
    else
    {
        key = luaL_checklstring(L, 2, &klen);
    }
    const char *val = js0n_run(s, key, klen, &vlen);
    if (val)
    {
        size_t pos = val - s->json;
        lua_pushinteger(L, pos + 1);
        lua_pushinteger(L, pos + vlen);
        return 2;
    }
    else
    {
        lua_pushnil(L);
        return 1;
    }
}

static void js0n_meta(lua_State *L)
{
    if (luaL_newmetatable(L, "js0n_mt"))
    {
        luaL_Reg l[] = {
            {"reset", lreset},
            {NULL, NULL},
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, lrun);
        lua_setfield(L, -2, "__call");
    }
    lua_setmetatable(L, -2);
}

static int lnew(lua_State *L)
{
    size_t jlen;
    int joffset = 0;
    const char *json = luaL_checklstring(L, 1, &jlen);
    size_t top = lua_gettop(L);
    if (top >= 2)
    {
        joffset = luaL_checkinteger(L, 2) - 1;
        if (joffset < 0)
            joffset = 0;
    }
    if (top >= 3)
    {
        jlen = luaL_checkinteger(L, 3);
    }

    status_t *s = (status_t *)lua_newuserdata(L, sizeof(status_t));
    js0n_init(s, json, jlen, joffset);
    js0n_meta(L);
    return 1;
}

LUA_API int luaopen_js0n(lua_State *L)
{
#if LUA_VERSION_NUM == 503
    luaL_checkversion(L);
#endif
    luaL_Reg l[] = {
        {"new", lnew},
        {NULL, NULL},
    };
    luaL_newlib(L, l);
    return 1;
}
