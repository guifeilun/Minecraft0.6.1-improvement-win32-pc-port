cmake_minimum_required(VERSION 3.21)
# XP_TARGET must be processed BEFORE project() so that the static-CRT setting
# applies to CMake's own compiler-detection probe (which otherwise tries to
# link against ucrtd.lib — not available on the v141_xp toolset).
option(XP_TARGET "Target Windows XP (requires v141_xp toolset, 32-bit only, statically links the CRT)" OFF)
if(XP_TARGET)
    # CMP0091 NEW = honor CMAKE_MSVC_RUNTIME_LIBRARY (instead of baking /MD into flags).
    # Must be set as default for sub-projects (CPM-fetched deps) too.
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    cmake_policy(SET CMP0091 NEW)
    # Statically link the CRT so the XP target needs no redistributable.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
endif()
project(MinecraftPE)
if(XP_TARGET)
    if(NOT WIN32)
        message(FATAL_ERROR "XP_TARGET only applies to Windows builds")
    endif()
    if(NOT MSVC)
        message(FATAL_ERROR "XP_TARGET requires MSVC (v141_xp toolset)")
    endif()
    # Pin Windows headers to XP so post-XP APIs fail at compile time rather than runtime.
    # Don't set NTDDI_VERSION — OpenAL 1.19.1 forces _WIN32_WINNT=0x0502 (Server 2003)
    # internally and would conflict; the Win SDK auto‑derives NTDDI_VERSION from _WIN32_WINNT.
    add_definitions(-DWINVER=0x0501 -D_WIN32_WINNT=0x0501 -DPSAPI_VERSION=1 -DWIN32_LEAN_AND_MEAN)
    # Make the resulting EXE actually load on XP (XP = NT 5.1)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE,5.01")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.01")
    message(STATUS "XP_TARGET enabled: targeting Windows XP SP3 (NT 5.1), static CRT")
endif()
include(cmake/CPM.cmake)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
include(cmake/EnumOption.cmake)
# This project is Windows (win32) only.
set(PLATFORM_CPP "PLATFORM_DESKTOP")
find_package(Threads REQUIRED)
find_package(OpenSSL)
if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing -Wno-narrowing -Wno-invalid-source-encoding -Wno-reserved-user-defined-literal")
endif()
if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN)
    if(MSVC)
        add_compile_options(/utf-8)
    endif()
    include_directories(misc/windows)
    set(EXTRA_LIBS ws2_32 winhttp)
endif()

set(PNG_LIB png_shared)
CPMAddPackage(
    NAME "zlib"
    GIT_REPOSITORY "https://github.com/madler/zlib"
    GIT_TAG "v1.3.2"
)
CPMAddPackage(
    NAME "libpng"
    GIT_REPOSITORY "https://github.com/pnggroup/libpng.git"
    GIT_TAG "v1.6.55"
    OPTIONS
        "ZLIB_ROOT ${zlib_SOURCE_DIR}"
        "ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR}"
        "PNG_TOOLS OFF"
        "PNG_TESTS OFF"
)
if(XP_TARGET)
    # GLFW 3.4 dropped XP support; 3.3.10 is the last release that runs on XP
    set(GLFW_TAG "3.3.10")
else()
    set(GLFW_TAG "3.4")
endif()
CPMAddPackage(
    NAME "glfw"
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "${GLFW_TAG}"
    EXCLUDE_FROM_ALL TRUE
    OPTIONS
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_DOCS OFF"
)

# Server — headless dedicated server (Windows).
file(GLOB SERVER_SOURCES
    "src/raknet/*.cpp"
    "src/NinecraftApp.cpp"
    "src/Performance.cpp"
    "src/SharedConstants.cpp"
    "src/client/IConfigListener.cpp"
    "src/client/Minecraft.cpp"
    "src/client/OptionStrings.cpp"
    "src/client/Option.cpp"
    "src/client/Options.cpp"
    "src/client/OptionsFile.cpp"
    "src/client/ServerProfiler.cpp"
    "src/client/gamemode/CreativeMode.cpp"
    "src/client/gamemode/GameMode.cpp"
    "src/client/gamemode/SurvivalMode.cpp"
    "src/client/player/LocalPlayer.cpp"
    "src/client/player/RemotePlayer.cpp"
    "src/client/player/input/KeyboardInput.cpp"
    "src/locale/I18n.cpp"
    "src/main.cpp"
    "src/main_dedicated.cpp"
    "src/nbt/Tag.cpp"
    "src/network/ClientSideNetworkHandler.cpp"
    "src/network/NetEventCallback.cpp"
    "src/network/Packet.cpp"
    "src/network/RakNetInstance.cpp"
    "src/network/ServerSideNetworkHandler.cpp"
    "src/network/command/CommandServer.cpp"
    "src/platform/CThread.cpp"
    "src/platform/HttpClient.cpp"
    # PngLoader.cpp is client‑only (textures); the headless server doesn't need
    # libpng linkage or texture loading.
    "src/platform/time.cpp"
    "src/platform/input/Controller.cpp"
    "src/platform/input/Keyboard.cpp"
    "src/platform/input/Mouse.cpp"
    "src/platform/input/Multitouch.cpp"
    "src/server/ArgumentsSettings.cpp"
    "src/server/ServerLevel.cpp"
    "src/server/ServerPlayer.cpp"
    "src/util/DataIO.cpp"
    "src/util/Mth.cpp"
    "src/util/PerfTimer.cpp"
    "src/util/StringUtils.cpp"
    "src/util/Color.cpp"
    "src/world/Direction.cpp"
    "src/world/entity/*.cpp"
    "src/world/entity/ai/control/MoveControl.cpp"
    "src/world/entity/animal/*.cpp"
    "src/world/entity/item/*.cpp"
    "src/world/entity/monster/*.cpp"
    "src/world/entity/player/Inventory.cpp"
    "src/world/entity/player/Player.cpp"
    "src/world/entity/projectile/Arrow.cpp"
    "src/world/entity/projectile/Throwable.cpp"
    "src/world/food/SimpleFoodData.cpp"
    "src/world/inventory/*.cpp"
    "src/world/item/*.cpp"
    "src/world/item/crafting/*.cpp"
    "src/world/level/*.cpp"
    "src/world/level/biome/Biome.cpp"
    "src/world/level/biome/BiomeSource.cpp"
    "src/world/level/chunk/LevelChunk.cpp"
    "src/world/level/dimension/Dimension.cpp"
    "src/world/level/dimension/NetherDimension.cpp"
    "src/world/level/dimension/FoliageColor.cpp"
    "src/world/level/GrassColor.cpp"
    "src/world/level/levelgen/*.cpp"
    "src/world/level/levelgen/feature/Feature.cpp"
    "src/world/level/levelgen/synth/*.cpp"
    "src/world/level/material/Material.cpp"
    "src/world/level/pathfinder/Path.cpp"
    "src/world/level/storage/*.cpp"
    "src/world/level/tile/*.cpp"
    "src/world/level/tile/entity/*.cpp"
    "src/world/phys/HitResult.cpp"
)
file(GLOB CLIENT_SOURCES
    "src/client/*.cpp"
    "src/client/gamemode/*.cpp"
    "src/client/gui/*.cpp"
    "src/client/gui/components/*.cpp"
    "src/client/gui/screens/*.cpp"
    "src/client/gui/screens/crafting/*.cpp"
    "src/client/gui/screens/touch/*.cpp"
    "src/client/gui/screens/desktop/*.cpp"
    "src/client/model/*.cpp"
    "src/client/model/geom/*.cpp"
    "src/client/particle/*.cpp"
    "src/client/player/*.cpp"
    "src/client/player/input/*.cpp"
    "src/client/player/input/touchscreen/*.cpp"
    "src/client/renderer/*.cpp"
    "src/client/renderer/culling/*.cpp"
    "src/client/renderer/entity/*.cpp"
    "src/client/renderer/ptexture/*.cpp"
    "src/client/renderer/tileentity/*.cpp"
    "src/client/sound/*.cpp"

    "src/locale/*.cpp"

    "src/nbt/*.cpp"
    "src/network/*.cpp"
    "src/network/command/*.cpp"
    "src/platform/*.cpp"
    "src/platform/audio/SoundSystemAL.cpp"
    "src/platform/input/*.cpp"
    "src/raknet/**.cpp"

    "src/server/**.cpp"

    "src/util/**.cpp"
    "src/util/Color.cpp"
    "src/world/*.cpp"
    "src/world/phys/*.cpp"
    "src/world/entity/*.cpp"
    "src/world/entity/ai/control/*.cpp"
    "src/world/entity/animal/*.cpp"
    "src/world/entity/item/*.cpp"
    "src/world/entity/monster/*.cpp"
    "src/world/entity/player/*.cpp"
    "src/world/entity/projectile/*.cpp"
    "src/world/food/*.cpp"
    "src/world/inventory/*.cpp"
    "src/world/item/*.cpp"
    "src/world/item/crafting/*.cpp"
    "src/world/level/*.cpp"
    "src/world/level/biome/*.cpp"
    "src/world/level/chunk/*.cpp"
    "src/world/level/dimension/*.cpp"
    "src/world/level/levelgen/*.cpp"
    "src/world/level/levelgen/feature/*.cpp"
    "src/world/level/levelgen/synth/*.cpp"
    "src/world/level/material/*.cpp"
    "src/world/level/pathfinder/*.cpp"
    "src/world/level/storage/*.cpp"
    "src/world/level/tile/*.cpp"
    "src/world/level/tile/entity/*.cpp"
    "src/SharedConstants.cpp"
    "src/main.cpp"
    "src/NinecraftApp.cpp"
    "src/AppPlatform_glfw.cpp"
)
list(APPEND CLIENT_SOURCES glad/src/glad.c)

add_executable("${PROJECT_NAME}-server" ${SERVER_SOURCES})
target_compile_definitions("${PROJECT_NAME}-server" PUBLIC "STANDALONE_SERVER" "SERVER_PROFILER")
target_include_directories("${PROJECT_NAME}-server" PUBLIC
    "${CMAKE_SOURCE_DIR}/glad/include/"
    "${CMAKE_SOURCE_DIR}/src/"
    "${CMAKE_SOURCE_DIR}/src/raknet/"
)
if (WIN32)
    target_include_directories("${PROJECT_NAME}-server" PUBLIC "misc/windows")
    target_link_libraries("${PROJECT_NAME}-server" ws2_32 winhttp winmm ${CMAKE_THREAD_LIBS_INIT})
else()
    target_link_libraries("${PROJECT_NAME}-server" ${CMAKE_THREAD_LIBS_INIT})
endif()

add_executable(${PROJECT_NAME} ${CLIENT_SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PLATFORM_CPP})
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/glad/include/"
    "${CMAKE_SOURCE_DIR}/src"
    "${glfw_SOURCE_DIR}/include"
    "lib/include"
)

# Client
target_compile_definitions(${PROJECT_NAME} PUBLIC "OPENGL_ES" "NO_EGL" "Desktop")
target_link_libraries(${PROJECT_NAME} zlib ${PNG_LIB} dsound winmm glfw ${EXTRA_LIBS})
if (OpenSSL_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HTTPCLIENT_USE_OPENSSL)
endif()
if (NOT UNIX)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND_EXPAND_LISTS
    )
endif()
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/data" $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)
message(STATUS "Compiling with the flags:")
message(STATUS "  PLATFORM=" ${PLATFORM_CPP})
