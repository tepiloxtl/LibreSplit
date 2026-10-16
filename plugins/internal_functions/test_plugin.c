// This plugin does not work in it's current state
// My idea is to give plugins access to some internal functions in the PlugAPI interface
// So plugins can't mose around the entire internals, is this worth something?

#include <lauxlib.h> 
#include "lasr/functions/getMaps.h"
#include "lasr/maps/maps.h"
#include "plugins/plugin.h"
#include "plugins/plugin_utils.h"
#include <stdio.h>

/* =======================================
 * Metadata section
 * ---------------------------------------
 * Here we write all the metadata of the plugin, as well as
 * the supported ABI version
 */
const abi_version_t abi_version = 1; // v0.1
const char plugin_name[] = "Test Plugin 3";
const char plugin_description[] = "Does something, wraps the getMaps function to demonstrate calling LS functions";
const char plugin_version[] = "0.1";
const char plugin_author[] = "The LibreSplit Core Team";

/**
 * Just a clone of getMaps, to demonstrate calling functions from a
 * plugin
 */
int getmaps_external(lua_State* L)
{
    printf("You are now using getMaps from a plugin");

    // Optional first argument: filter by map name.
    const char* filter = luaL_optstring(L, 1, NULL);
    // Optional second argument: max results.
    lua_Integer limit = luaL_optinteger(L, 2, -1);

    if (maps_cache == NULL) {
        maps_cache_size = maps_getAll();
    }
    // Create a table, in "array mode", maps_cache_size big
    lua_createtable(L, maps_cache_size, 0);
    int count = 0;
    // Stack: array
    for (uint32_t i = 0; i < maps_cache_size; i++) {
        ProcessMap map = maps_cache[i];
        if (filter != NULL && strcmp(map.name, filter) != 0) {
            continue;
        }
        if (limit >= 0 && count >= limit) {
            break;
        }
        // Create a new table with 4 non-array fields
        lua_createtable(L, 0, 4);
        // Stack: array, table
        // Push "name" onto the stack
        lua_pushstring(L, map.name);
        // Stack: array, table, value
        lua_setfield(L, -2, "name");
        // Stack: array, table
        // Push "start"
        lua_pushnumber(L, (lua_Integer)map.start);
        // Stack: array, table, value
        lua_setfield(L, -2, "start");
        // Stack: array, table
        // Push "end"
        lua_pushnumber(L, (lua_Integer)map.end);
        lua_setfield(L, -2, "end");
        // Push "size"
        lua_pushnumber(L, (lua_Integer)map.size);
        lua_setfield(L, -2, "size");
        lua_rawseti(L, -2, ++count);

        // Stack: Array
    }
    return 1;
}

int plug_init(PlugAPI* api)
{
    // Register the getMapsExternal Lua function, using getmaps_external as its C function
    api->register_lua_function("getMapsExternal", getmaps_external);
    return 0;
}
