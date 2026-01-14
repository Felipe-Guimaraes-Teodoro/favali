#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#include "lua.h"

#define DECLARATIONS \
    FIELD(double, osc) \
    FIELD(bool, boolean) \
    FIELD(VOIDPTR, data) \
    FIELD(NUM_ARRAY, samples) \
    FIELD(STRING, message) \
    FIELD(double, goal_x) \
    FIELD(double, goal_y) \
    FIELD(double, goal_z) \

#define FIELD(type, name) MAP_##type(name)

#define MAP_double(name) double name;
#define MAP_bool(name) bool name;
#define MAP_VOIDPTR(name) void* name;
#define MAP_NUM_ARRAY(name) double* name; unsigned int name##_len;
#define MAP_STRING(name) char* name; unsigned int name##_len;

typedef struct {
    DECLARATIONS
    pthread_mutex_t mtx;
    int busy;
    int co_alive;

    #undef FIELD
} LuaCtx;

// globals
extern LuaCtx *lua_ctx;
extern lua_State *lua;

void exec_script();
void scripting_quit();

#ifdef __cplusplus
}
#endif
