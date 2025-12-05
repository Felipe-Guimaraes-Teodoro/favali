#include <stdlib.h>
#include <unistd.h>

#include "lualib.h"
#include "lauxlib.h"
#include "lua.h"

#include "scripting.h"

LuaCtx *lua_ctx = NULL;
lua_State *lua = NULL;
static lua_State *lua_co = NULL;

void lua_init_ctx(lua_State* L) {
    lua_newtable(L);

    #define FIELD(type, name) INIT_##type(name)

    #define INIT_double(name) lua_pushnumber(L, 0.0); lua_setfield(L, -2, #name); 
    #define INIT_bool(name) lua_pushboolean(L, 0); lua_setfield(L, -2, #name);
    #define INIT_VOIDPTR(name) lua_pushnil(L); lua_setfield(L, -2, #name);
    #define INIT_NUM_ARRAY(name) lua_newtable(L); lua_setfield(L, -2, #name);
    #define INIT_STRING(name) lua_pushstring(L, ""); lua_setfield(L, -2, #name);

    DECLARATIONS
    
    #undef FIELD

    #undef INIT_double
    #undef INIT_bool
    #undef INIT_VOIDPTR
    #undef INIT_NUM_ARRAY
    #undef INIT_STRING

    lua_setglobal(L, "ctx");
}

void lua_fetch_ctx(lua_State* L) {
    lua_getglobal(L, "ctx");

    /*
    if (lua_isnoneornil(L, -1)) {
        lua_init_ctx(L);
        perror("ctx was none/nil");
        lua_pop(L, 1);
        return;
    }
    */

    #define FIELD(type, name) FETCH_##type(name)

    #define FETCH_double(name) \
        lua_getfield(L, -1, #name); \
        lua_ctx->name = lua_tonumber(L, -1); \
        lua_pop(L, 1); 

    #define FETCH_bool(name) \
        lua_getfield(L, -1, #name); \
        lua_ctx->name = lua_toboolean(L, -1); \
        lua_pop(L, 1);

    #define FETCH_VOIDPTR(name) \
        lua_getfield(L, -1, #name); \
        lua_ctx->name = lua_touserdata(L, -1); \
        lua_pop(L, 1);
    #define FETCH_NUM_ARRAY(name) \
        lua_getfield(L, -1, #name); \
        if (lua_istable(L, -1)) { \
            int n = (int)lua_rawlen(L, -1); \
            lua_ctx->name##_len = n; \
            lua_ctx->name = malloc(sizeof(double) * n); \
            for (int i = 1; i <= n; i++) { \
                lua_rawgeti(L, -1, i); \
                lua_ctx->name[i-1] = lua_tonumber(L, -1); \
                lua_pop(L, 1); \
            } \
        } else { \
            lua_ctx->name = NULL; \
            lua_ctx->name##_len = 0; \
        } \
        lua_pop(L, 1);
    #define FETCH_STRING(name) \
        lua_getfield(L, -1, #name); \
        if (lua_isstring(L, -1)) { \
            size_t len; \
            const char* s = lua_tolstring(L, -1, &len); \
            if (lua_ctx->name) free(lua_ctx->name); \
            lua_ctx->name = malloc(len + 1); \
            memcpy(lua_ctx->name, s, len); \
            lua_ctx->name[len] = '\0'; \
            lua_ctx->name##_len = (unsigned int)len; \
        } else { \
            if (lua_ctx->name) free(lua_ctx->name); \
            lua_ctx->name = NULL; \
            lua_ctx->name##_len = 0; \
        } \
        lua_pop(L, 1);

    DECLARATIONS

    // is it socially acceptable to create a macro to auto undef these?
    #undef FETCH_double
    #undef FETCH_bool
    #undef FETCH_VOIDPTR
    #undef FETCH_NUM_ARRAY
    #undef FETCH_STRING

    lua_topointer(L, -1);

    #undef FIELD
    lua_pop(L, 1);
}

#undef DECLARATIONS

#undef MAP_double
#undef MAP_bool
#undef MAP_VOIDPTR
#undef MAP_NUM_ARRAY
#undef MAP_STRING

pthread_t lua_execution_thread;
pthread_t lua_synchronization_thread;

#define LOCK pthread_mutex_lock(&lua_ctx->mtx)
#define TRYLOCK pthread_mutex_trylock(&lua_ctx->mtx)
#define UNLOCK pthread_mutex_unlock(&lua_ctx->mtx)

int tick(int delay_ms) {
    usleep(delay_ms * 1000);
    return 1;
}

void *pt_lua_execute(void *arg) {
    int nresults = 0;

    while(tick(16)) {
        LOCK;
        lua_ctx->busy = true;

        int status = lua_resume(lua_co, NULL, 0, &nresults);

        if (status == LUA_YIELD) {
            UNLOCK;
            continue;
        }

        if (status == LUA_OK) {
            UNLOCK;
            break;
        }

        fprintf(stderr, "Lua error: %s\n", lua_tostring(lua_co, -1));
        lua_pop(lua_co, 1);
        break;
    }
    
    lua_ctx->busy = false;
    UNLOCK;

    return NULL;
}

void *pt_lua_sync(void *arg) {
    lua_State *lua = (lua_State*)arg;

    while (tick(16)) {
        if (TRYLOCK == 0) {
            lua_fetch_ctx(lua);
            UNLOCK;
        }
    }

    return NULL;
}

void hook(lua_State *L, lua_Debug *ar) {
    lua_yield(L, 0);
}

void exec_script() {
    if (!lua) {
        lua = luaL_newstate();
        luaL_openlibs(lua);

        lua_ctx = calloc(1, sizeof(LuaCtx));
        pthread_mutex_init(&lua_ctx->mtx, NULL);

        lua_init_ctx(lua);

        pthread_create(&lua_synchronization_thread, NULL, pt_lua_sync, lua);
        pthread_detach(lua_synchronization_thread);
    }
    if (lua_ctx->busy) { return; }

    lua_co = lua_newthread(lua);

    if (luaL_loadfile(lua_co, "script.lua") != LUA_OK) {
        fprintf(stderr, "Load error: %s\n", lua_tostring(lua_co, -1));
        return;
    }

    lua_sethook(lua_co, hook, LUA_MASKCOUNT, 20000);

    pthread_create(&lua_execution_thread, NULL, pt_lua_execute, lua);
    pthread_detach(lua_execution_thread);
}

void scripting_quit() {
    LOCK;
    lua_close(lua);
    UNLOCK;
}
