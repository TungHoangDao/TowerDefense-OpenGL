cmake_minimum_required(VERSION 3.16)
project(TowerDefenseSDL)

set(CMAKE_CXX_STANDARD 11)

add_executable(TowerDefenseSDL Timer.cpp Timer.h main.cpp glext.h glxext.h shaders.c main.h)
target_link_libraries(TowerDefenseSDL -lglew32s -lglu32 -lOpenGL32 -lfreeGLUT -lmingw32 -lSDL2main -lSDL2 )
