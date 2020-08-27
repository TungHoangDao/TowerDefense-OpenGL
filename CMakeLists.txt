cmake_minimum_required(VERSION 3.16)
project(TowerDefenseSDL)

set(CMAKE_CXX_STANDARD 11)

#add_executable(TowerDefenseSDL main.cpp Global.cpp Global.h Timer.cpp Timer.h glExtension.cpp glExtension.h glext.h glxext.h teapot.h WaveFunc.cpp WaveFunc.h)
add_executable(TowerDefenseSDL Global.cpp Global.h Timer.cpp Timer.h Demo.cpp glext.h glxext.h teapot.h WaveFunc.cpp WaveFunc.h)
target_link_libraries(TowerDefenseSDL -lglew32s -lglu32 -lOpenGL32 -lfreeGLUT -lmingw32 -lSDL2main -lSDL2 )
#target_link_libraries(TowerDefenseSDL -lglu32 -lOpenGL32 -lfreeGLUT -lmingw32 -lSDL2main -lSDL2 )