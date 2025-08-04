cmake_minimum_required(VERSION 3.10)
project(WindsongLyre)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
add_executable(WindsongLyre main.cpp
        Scene1.cpp
        Note.cpp
        Channel.hpp
        Sound.hpp
        Note.hpp
        Piano.hpp
        resource/icon.rc
        resource/resources.rc
        button.hpp
        scene.hpp
        graphics.hpp
        scene1.hpp
        graphics.cpp
        Scene2.hpp
        Scene2.cpp
        Scene3.hpp
        Scene3.cpp
        string.cpp
        string.hpp)
target_link_libraries(WindsongLyre libeasyx.a)
target_link_libraries(WindsongLyre winmm.lib)

set(Library ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include_directories(${Library}/libsndfile)
target_link_libraries(WindsongLyre ${Library}/libsndfile/libsndfile.a)

include_directories(${Library}/rubberband)
target_link_libraries(WindsongLyre ${Library}/rubberband/librubberband.a)


set(app_icon_resource_windows "resource/icon.rc")
