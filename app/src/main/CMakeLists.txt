cmake_minimum_required(VERSION 3.4.1)

add_library(game SHARED
    code/bundle.cpp
    code/bundle.h
    code/mat3.h
    code/rect.h
    code/sprite.h
    code/types.h
    code/vec2.h
    code/game_state.h
    code/app_clock.h
    code/app_clock.cpp
    code/asset_loader.h
    code/asset_loader.cpp
    code/renderer.h
    code/renderer.cpp
    code/animation.h
    code/animation.cpp
    code/game.h
    code/game.cpp
    code/score_label.h
    code/score_label.cpp
    code/world.h
    code/world.cpp
    code/user_interface.h
    code/user_interface.cpp
    code/app_delegate.cpp
)

include(AndroidNdkModules)

android_ndk_import_module_native_app_glue()

target_link_libraries(game PRIVATE log android EGL GLESv2 native_app_glue)

set_target_properties(game PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    CXX_FLAGS "-fexceptions"
)
