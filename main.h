#ifndef TOWERDEFENSESDL_MAIN_H
#define TOWERDEFENSESDL_MAIN_H
#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
//#include "glExtension.h"
#include <stdlib.h>
#include <math.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include "Timer.h"
#include "shaders.h"


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>


typedef struct {
    float x, y;
} vec2f;

typedef struct {
    float x, y, z;
} vec3f;

typedef struct {
    glm::vec3 r, n, c;
} Vertex;


// Globals
bool debug = true;
SDL_Window *window;
const float gripper_increment = .2;
const int milli = 1000;

typedef struct {
    float A;
    float k;
    float w;
} sinewave;

sinewave sws[] =
        {
                {0.25, 2 * M_PI / 1, 0.25 * M_PI},
                {0.25, 1 * M_PI / 1, 0.5 * M_PI}
        };

int nsw = 2;

enum RenderMode {
    IMMEDIATE_MODE = 0,
    STORE_ARRAY = 1,
    STORE_ARRAY_INDICE = 2,
    VERTEXT_ARRAY = 3,
    VERTEX_BUFFER_OBJECT
} renMode = VERTEX_BUFFER_OBJECT;

enum FillingMode{
    LINE = 0,
    FILL = 1
} fillMode = LINE;

enum Direction : unsigned {
    LEFT = 0,
    TOP = 1,
    RIGHT = 2,
    BOTTOM = 3
} direction = LEFT;

std::string MODE_STRING[] = {
        "IMMEDIATE_MODE",
        "STORE_ARRAY",
        "STORE_ARRAY_INDICE",
        "VERTEXT_ARRAY",
        "VERTEX_BUFFER_OBJECT"
};

enum {
    IM = 0, SA, SAI, VA, VBO, nM
} mode = VBO;

bool PAUSE = false;
bool STATIC_RENDERING = false;
bool USE_SHADER = true;

#define BUFFER_OFFSET(i) ((void*)(i))

Vertex *vertices;
unsigned *indices;
unsigned n_vertices, n_indices;
unsigned vbo, ibo;
unsigned rows = 50, cols = 50;
bool lightMode = true;

void idleCB();

void mouseCB(int button, int stat, int x, int y);

void mouseMotionCB(int x, int y);

// CALLBACK function when exit() called ///////////////////////////////////////
void exitCB();
void initGL();
int initGLUT(int argc, char **argv);
bool initSharedMem();
void initLights();

void deleteVBO(const GLuint vboId);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void showInfo();
void updateVertices(float *vertices, float *srcVertices, float *srcNormals, int count, float time);
void showFPS();
void toPerspective();


// constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float CAMERA_DISTANCE = 7.0f;
const int TEXT_WIDTH = 8;
const int TEXT_HEIGHT = 13;


// global variables
void *font = GLUT_BITMAP_8_BY_13;
GLuint vboId1 = 0;                  // ID of VBO for vertex arrays (to store vertex coords and normals)
GLuint vboId2 = 0;                  // ID of VBO for index array
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
bool vboSupported, vboUsed;
int drawMode = 0;
Timer timer, t1, t2;
float drawTime, updateTime;
float *srcVertices;                 // pointer to copy of vertex array
int vertexCount;                 // number of vertices

float min = 999;
float max = -999;
float average;
GLuint program;

/// GLM SET UP

glm::vec3 cyan(0.0, 1.0, 1.0);
glm::vec3 magenta(1.0, 0.0, 1.0);
glm::vec3 yellow(1.0, 1.0, 0.0);
glm::vec3 white(1.0, 1.0, 1.0);
glm::vec3 grey(0.8, 0.8, 0.8);
glm::vec3 black(0.0, 0.0, 0.0);
const float shininess = 50.0;

glm::mat4 modelViewMatrix;
glm::mat3 normalMatrix;
#endif //TOWERDEFENSESDL_MAIN_H
