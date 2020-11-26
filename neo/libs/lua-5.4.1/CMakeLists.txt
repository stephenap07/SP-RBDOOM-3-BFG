option(LUA_SUPPORT_DL "Support dynamic loading of compiled modules" OFF)
option(LUA_BUILD_BINARY "Build lua binary" OFF)
option(LUA_BUILD_COMPILER "Build luac compiler" OFF)

set(LUA_LIB_SRCS 
    "src/lapi.c"
    "src/lcode.c"
    "src/lctype.c"
    "src/ldebug.c"
    "src/ldo.c"
    "src/ldump.c"
    "src/lfunc.c"
    "src/lgc.c"
    "src/llex.c"
    "src/lmem.c"
    "src/lobject.c"
    "src/lopcodes.c"
    "src/lparser.c"
    "src/lstate.c"
    "src/lstring.c"
    "src/ltable.c"
    "src/ltm.c"
    "src/lundump.c"
    "src/lvm.c"
    "src/lzio.c"
    "src/lauxlib.c"
    "src/lbaselib.c"
    "src/lcorolib.c"
    "src/ldblib.c"
    "src/liolib.c"
    "src/lmathlib.c"
    "src/loadlib.c"
    "src/loslib.c"
    "src/lstrlib.c"
    "src/ltablib.c"
    "src/lutf8lib.c"
    "src/linit.c"
)

set(LUA_LIB_HEADERS
    "src/luaxlib.h"
    "src/lua.h"
    "src/luaconf.h"
    "src/lualib.h")

add_library(lua_static STATIC ${LUA_LIB_SRCS})

target_include_directories(lua_static PUBLIC "src")
set(LUA_DEFINITIONS)
if(UNIX)
    list(APPEND LUA_DEFINITIONS LUA_USE_POSIX)
    if(LUA_SUPPORT_DL)
        target_compile_definitions(lua_static PRIVATE "LUA_USE_DLOPEN")
        target_link_libraries(lua_static INTERFACE dl)
    endif()
    target_compile_options(lua_static
    PRIVATE "-Wall" "-Wextra"
)
endif()
target_compile_definitions(lua_static 
    PUBLIC ${LUA_DEFINITIONS}
)

if(LUA_BUILD_BINARY)
    add_executable(lua "src/lua.c")
    target_link_libraries(lua PUBLIC lua_static)
    set_target_properties(lua PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    )
    if (HAVE_READLINE_READLINE_H)
        target_compile_definitions(lua PUBLIC "LUA_USE_READLINE")
        target_link_libraries(lua PUBLIC readline)
    endif()
endif()
if(LUA_BUILD_COMPILER)
    add_executable(luac "src/luac.c")
    target_link_libraries(luac PUBLIC lua_static)
    set_target_properties(luac PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    )
endif()
