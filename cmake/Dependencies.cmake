# Third-party libraries, fetched at pinned versions (DEC-023). Changing a version is a
# decision (coding-standards.md section 6): raylib DEC-033, Box2D DEC-020, GoogleTest DEC-023.
include(FetchContent)

# Box2D v3.1.1 (DEC-020). Its samples and unit tests are only built when Box2D is the
# top-level project, so nothing else needs switching off.
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.1 # 8c661469c9507d3ad6fbd2fea3f1aa71669c2fe3
    GIT_SHALLOW TRUE
    SYSTEM
)
FetchContent_MakeAvailable(box2d)

# raylib 6.2 (DEC-033). Not fetched at all in the headless preset.
if(ENGINE_WITH_RAYLIB)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.2 # 02566453880e06bec97f5b8160fa6fe1e98397ca
        GIT_SHALLOW TRUE
        SYSTEM
    )
    FetchContent_MakeAvailable(raylib)
endif()

if(ENGINE_BUILD_TESTS)
    # Use the same C runtime as our targets on MSVC.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0 # 52eb8108c5bdec04579160ae17225d66034bd723
        GIT_SHALLOW TRUE
        SYSTEM
    )
    FetchContent_MakeAvailable(googletest)
endif()
