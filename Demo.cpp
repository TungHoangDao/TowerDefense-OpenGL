/*
 * robot arm using SDL2
 *
 * This program shows how to composite modeling transformations
 * to draw translated and rotated hierarchical models.
 * Interaction:  pressing the s and e keys (shoulder and elbow)
 * alters the rotation of the robot arm shoulder and elbow.
 * Also left and right plates of the gripper.
 *
 * Originally based on the Redbook robot arm example.
 */
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

/// Global controller.
typedef struct {
    float t;
    float dt;
    float tLast;
    float startTime;
    float waterT;
    float go;

    bool OSD;
    int frames;
    float frameRate;
    float frameRateInterval;
    float lastFrameRateT;

    bool gameOver;

    float mapArea;
} global_t;

global_t g = {0, 0, 0, 0, 0, 0, true, 0, 0, 0.2, 0, false};

typedef struct {
    float x, y;
} vec2f;

// Globals
bool debug = true;
float shoulder, elbow, wrist, left_plate, right_plate;
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
    VERTEX_ARRAY,
    VERTEX_BUFFER_OBJECT,
    NUMRENMODES
} renMode = VERTEX_ARRAY;

enum DerefMethods {
    DRAWARRAYS = 0,
    MULTIDRAWARRAYS,
    ARRAYELEMENT,
    DRAWELEMENTS,
    DRAWELEMENTSALL,
    MULTIDRAWELEMENTS,
    NUMDEREFMETHODS
} derefMethod = MULTIDRAWELEMENTS;

#define BUFFER_OFFSET(i) ((void*)(i))

typedef struct {
    float x, y, z;
} vec3f;
typedef struct {
    vec3f r, n;
} Vertex;

Vertex *vertices;
unsigned *indices;
unsigned n_vertices, n_indices;
unsigned vbo, ibo;
unsigned rows = 50, cols = 50;

enum {
    IM = 0, SA, SAI, VA, VBO, nM
} mode = VBO;


// GLUT CALLBACK functions
void displayCB();

void reshapeCB(int w, int h);

void timerCB(int millisec);

void idleCB();

void keyboardCB(unsigned char key, int x, int y);

void mouseCB(int button, int stat, int x, int y);

void mouseMotionCB(int x, int y);

// CALLBACK function when exit() called ///////////////////////////////////////
void exitCB();


void initGL();

int initGLUT(int argc, char **argv);

bool initSharedMem();

void clearSharedMem();

void initLights();

void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);

GLuint
createVBO(const void *data, int dataSize, GLenum target = GL_ARRAY_BUFFER_ARB, GLenum usage = GL_STATIC_DRAW_ARB);

void deleteVBO(const GLuint vboId);

void drawString(const char *str, int x, int y, float color[4], void *font);

void drawString3D(const char *str, float pos[3], float color[4], void *font);

void showInfo();

void updateVertices(float *vertices, float *srcVertices, float *srcNormals, int count, float time);

void showFPS();

void toOrtho();

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



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font) {
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while (*str) {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}


///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void showInfo() {
    // backup current model-view matrix
    glPushMatrix();                 // save current modelview matrix
    glLoadIdentity();               // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);    // switch to projection matrix
    glPushMatrix();                 // save current projection matrix
    glLoadIdentity();               // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    std::stringstream ss;
    ss << "VBO: " << (vboUsed ? "on" : "off") << std::ends;  // add 0(ends) at the end
    drawString(ss.str().c_str(), 1, screenHeight - TEXT_HEIGHT, color, font);
    ss.str(""); // clear buffer

    ss << std::fixed << std::setprecision(3);
    ss << "Updating Time: " << updateTime << " ms" << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (2 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Drawing Time: " << drawTime << " ms" << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (3 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Press SPACE key to toggle VBO on/off." << std::ends;
    drawString(ss.str().c_str(), 1, 1, color, font);

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}


///////////////////////////////////////////////////////////////////////////////
// display frame rates
///////////////////////////////////////////////////////////////////////////////
void showFPS() {
    static Timer timer;
    static int count = 0;
    static std::string fps = "0.0 FPS";
    double elapsedTime = 0.0;;

    // update fps every second
    ++count;
    elapsedTime = timer.getElapsedTime();
    if (elapsedTime > 1.0) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << (count / elapsedTime) << " FPS" << std::ends; // update fps string
        ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
        fps = ss.str();
        count = 0;                      // reset counter
        timer.start();                  // restart timer
    }

    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection

    float color[4] = {1, 1, 0, 1};
    int textWidth = (int) fps.size() * TEXT_WIDTH;
    drawString(fps.c_str(), screenWidth - textWidth, screenHeight - TEXT_HEIGHT, color, font);

    // restore projection matrix
    glPopMatrix();                      // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);         // switch to modelview matrix
    glPopMatrix();                      // restore to previous modelview matrix
}


void enableVAs() {
    glEnableClientState(GL_VERTEX_ARRAY);
}

void disableVAs() {
    glDisableClientState(GL_VERTEX_ARRAY);
}

void bindVBOs() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void unbindVBOs() {
    glBindBuffer(0, vbo);
    glBindBuffer(0, ibo);
}

void buildVBOs() {
    glGenBuffers(1, &vbo); //buffer for vertex
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, n_vertices * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &ibo); //buffer for indice
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);
}

void enableVBOs() {
    glEnableClientState(GL_VERTEX_ARRAY);

}

void disableVBOs() {
    glDisableClientState(GL_VERTEX_ARRAY);

}


void calcSineWave3D(sinewave wave, float x, float z, float t, float *y, bool der, float *dydx) {
    float angle = wave.k * x * x + wave.k * z * z + wave.w * t;
//    float angle = wave.k * z * z  + wave.w * t;

    *y = wave.A * sinf(angle);
    if (der) {
        *dydx = wave.k * wave.A * cosf(angle);
    }
}

void drawGrid2D(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);

    /* Grid */
    float dy = 2.0f / (float) rows;
    float dx = 2.0f / (float) cols;
    for (int i = 0; i < cols; i++) {
        float x = -1.0 + i * dx;
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= rows; j++) {
            float y = -1.0 + j * dy;
            glVertex3f(x, y, 0.0);
            glVertex3f(x + dx, y, 0.0);
        }
        glEnd();
    }

    glPopAttrib();
}

float rand01() {
    return rand() / (float) RAND_MAX;

}

void computeAndStoreGrid2D(int rows, int cols) {
    n_vertices = (rows + 1) * (cols + 1);
    n_indices = (rows + 1) * (cols - 1) * 2 + (rows + 1) * 2;
    // or more simply: n_indices = n_vertices * 2;
    free(vertices);
    vertices = (Vertex *) malloc(n_vertices * sizeof(Vertex));
    free(indices);
    indices = (unsigned *) malloc(n_indices * sizeof(unsigned));


    /* Grid */

    /* Vertices */
    float dy = 20 / (float) rows;
    float dx = 20 / (float) cols;
    Vertex *vtx = vertices;
    for (int i = 0; i <= cols; i++) {
        float x = -1.0 + i * dx;
        for (int j = 0; j <= rows; j++) {
            float z = -1.0 + j * dy;
            float y;
            float dxdy;
            calcSineWave3D(sws[0], x, z, g.t, &y, false, &dxdy);
            vtx->r = (vec3f) {x, y, z};
            vtx++;
        }
    }

    /* Indices */
    unsigned *idx = indices;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j <= rows; j++) {
            *idx++ = i * (rows + 1) + j;
            *idx++ = (i + 1) * (rows + 1) + j;
        }
    }

#define DEBUG_STORE_VERTICES
#ifdef DEBUG_STORE_VERTICES
    for (int i = 0; i <= cols; i++) {
        for (int j = 0; j <= rows; j++) {
            int idx = i * (rows + 1) + j;
            printf("(%5.2f,%5.2f, %5.2f)", vertices[idx].r.x, vertices[idx].r.y, vertices[idx].r.z);
        }
        printf("\n");
    }
    for (int i = 0; i < n_indices; i++) {
        printf("%d ", indices[i]);
    }
    printf("\n");
#endif

}

#define DEBUG_DRAW_GRID_ARRAY

void drawGrid2DStoredVertices(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);

    /* Grid */
    for (int i = 0; i < cols; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= rows; j++) {
            int idx = i * (rows + 1) + j;
#ifdef DEBUG_DRAW_GRID_ARRAY
            printf("%d %d %d %f %f\n", i, j, idx, vertices[idx].r.x, vertices[idx].r.y);
#endif
            glVertex3fv(&vertices[idx].r.x);
            idx += rows + 1;
#ifdef DEBUG_DRAW_GRID_ARRAY
            printf("%d %d %d %f %f\n", i, j, idx, vertices[idx].r.x, vertices[idx].r.y);
#endif
            glVertex3fv(&vertices[idx].r.x);
        }
        glEnd();
    }

    glPopAttrib();
}

void drawGrid2DStoredVerticesAndIndices(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);

    /* Grid */
    unsigned *idx = indices;
    for (int i = 0; i < cols; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= rows; j++) {
            glVertex3fv(&vertices[*idx].r.x);
            printf("%d %d %d %f %f\n", i, j, *idx, vertices[*idx].r.x, vertices[*idx].r.y);

            idx++;

            glVertex3fv(&vertices[*idx].r.x);
            printf("%d %d %d %f %f\n", i, j, *idx, vertices[*idx].r.x, vertices[*idx].r.y);

            idx++;
        }
        glEnd();
    }

    glPopAttrib();
}

void drawGrid2DVAs(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].r);

    for (int i = 0; i < cols; i++)
        glDrawElements(GL_TRIANGLE_STRIP, (rows + 1) * 2, GL_UNSIGNED_INT, &indices[i * (rows + 1) * 2]);

    glPopAttrib();
}

void drawGrid2DVBOs(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);

    bindVBOs();
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
    /* Grid */
    for (int i = 0; i < cols; i++) {
        glDrawElements(GL_TRIANGLE_STRIP, (rows + 1) * 2, GL_UNSIGNED_INT,
                       BUFFER_OFFSET(i * (rows + 1) * 2 * sizeof(unsigned int)));
    }
    unbindVBOs();
    glPopAttrib();
}

void UpdateVertices() {

//    // bind then map the VBO
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    float* ptr = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
//
//// if the pointer is valid (mapping was successful), update VBO
//    if(ptr) {
//        updateMyVBO(ptr, ...);    // modify buffer data
//    glUnmapBuffer(GL_ARRAY_BUFFER); // unmap it after use }
//// you can draw the updated VBO ...
//
//    for(int i = 0; i < n_vertices; i++)
//    {
//        Vertex *vertex = &vertices[i];
//        float dxdy;
//        calcSineWave3D(sws[0],vertex->r.x,vertex->r.z,g.t,&vertex->r.y,false,&dxdy);
//    }
}


void checkForGLerrors(int lineno) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
        printf("%d: %s\n", lineno, gluErrorString(error));
}


/* Immediate mode, vertex at a time */
void renderCubeIM() {
//    int i, j;
//
//    glBegin(GL_QUADS);
//    for (i = 0; i < numQuads; i++)
//        for (j = 0; j < 4; j++) {
//            glColor3fv(&colors[indices1DArray[i*4+j]*3]);
//            glVertex3fv(&vertices[indices1DArray[i*4+j]*3]);
//        }
//    glEnd();
}

/* Use VAs or VBOs */
void renderCubeVAVBO() {
//    int i, j;
//
//    /* Bind/unbind buffers and set vertex and color array pointers */
//    switch (derefMethod) {
//        case DRAWARRAYS:
//        case MULTIDRAWARRAYS:
//            if (renMode == VERTEX_ARRAY) {
//                glBindBuffer(GL_ARRAY_BUFFER, 0);
//                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//                glVertexPointer(3, GL_FLOAT, 0, verticesQuads);
//                glColorPointer(3, GL_FLOAT, 0, colorsQuads);
//            } else if (renMode == VERTEX_BUFFER_OBJECT) {
//                glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTICES_QUADS]);
//                glVertexPointer(3, GL_FLOAT, 0, 0);
//                glBindBuffer(GL_ARRAY_BUFFER, buffers[COLORS_QUADS]);
//                glColorPointer(3, GL_FLOAT, 0, 0);
//            }
//            break;
//        case ARRAYELEMENT:
//        case DRAWELEMENTS:
//        case DRAWELEMENTSALL:
//        case MULTIDRAWELEMENTS:
//            if (renMode == VERTEX_ARRAY) {
//                glBindBuffer(GL_ARRAY_BUFFER, 0);
//                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//                glVertexPointer(3, GL_FLOAT, 0, vertices);
//                glColorPointer(3, GL_FLOAT, 0, colors);
//            }  if (renMode == VERTEX_BUFFER_OBJECT) {
//        glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTICES]);
//        glVertexPointer(3, GL_FLOAT, 0, 0);
//        glBindBuffer(GL_ARRAY_BUFFER, buffers[COLORS]);
//        glColorPointer(3, GL_FLOAT, 0, 0);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDICES]);
//    }
//            break;
//    }
//
//    /* Render using chosen dereference technique. */
//    switch (derefMethod) {
//        case DRAWARRAYS:
//            glDrawArrays(GL_QUADS, 0, 24);
//            break;
//        case MULTIDRAWARRAYS:
//            /* Just an example to demonstrate usage - no advantage to use
//             * glMultiDrawArrays for cube as all primitives are quads and same
//             * size so glDrawArrays does the job.
//             */
//            glMultiDrawArrays(GL_QUADS, indicesFirsts, indicesCounts, numQuads);
//            break;
//        case ARRAYELEMENT:
//            glBegin(GL_QUADS);
//            for (i = 0; i < numQuads; i++)
//                for (j = 0; j < 4; j++)
//                    glArrayElement(((GLuint*)indices1DArrayOfArray[i])[j]);
//            glEnd();
//            break;
//        case DRAWELEMENTS:
//            for (i = 0; i < numQuads; i++)
//                if (renMode == VERTEX_ARRAY)
//                    /* Can use either indices1DArray or indices2DArrays */
//                    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, &indices1DArray[i*4]);
//                else  if (renMode == VERTEX_BUFFER_OBJECT)
//                    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, indicesOffsets[i]);
//            break;
//        case DRAWELEMENTSALL:
//            if (renMode == VERTEX_ARRAY)
//                glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, indices1DArray);
//            else  if (renMode == VERTEX_BUFFER_OBJECT)
//                glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
//            break;
//        case MULTIDRAWELEMENTS:
//            /* Another example just to demonstrate usage.  Like
//             * glMultiDrawArrays, no advantage using the 'multi' version for
//             * cube consisting of quads as all same size.
//             */
//            if (renMode == VERTEX_ARRAY)
//                glMultiDrawElements(GL_QUADS, indicesCounts, GL_UNSIGNED_INT,
//                                    indices1DArrayOfArray, numQuads);
//            else  if (renMode == VERTEX_BUFFER_OBJECT)
//                glMultiDrawElements(GL_QUADS, indicesCounts, GL_UNSIGNED_INT,
//                                    indicesOffsets, numQuads);
//            break;
//    }
}


/*
 * We reimplement GLUT's glutPostRedisplay() function.  Why? Because
 * it allows us to easily request a redraw, without having to worry
 * about doing redundant redraws of the scene. We use the
 * wantRedisplay variable as a latch that is cleared when a redraw is
 * done.
 */
int wantRedisplay = 1;

void postRedisplay() {
    wantRedisplay = 1;
}

void quit(int code) {
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(code);
}

///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights() {
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 20, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}


// OpenGL initialisation
void init(void) {
    glShadeModel(GL_FLAT);
    glColor3f(1.0, 1.0, 1.0);
    computeAndStoreGrid2D(rows, cols);
    buildVBOs();
}


// Draws a wireframe box
void myWireBox(float l, float h) {
    glPushMatrix();
    glScalef(0.5, 0.5, 0.5);
    glScalef(l, h, 1.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-1.0, -1.0);
    glVertex2f(1.0, -1.0);
    glVertex2f(1.0, 1.0);
    glVertex2f(-1.0, 1.0);
    glEnd();
    glPopMatrix();
}


// Draws a wireframe diamond - a rotated box
void myWireDiamond(float l, float h) {
    glPushMatrix();
    glScalef(l, h, 1.0);
    glScalef(1.0 / sqrt(2.0), 1.0 / sqrt(2.0), 1.0);
    glRotatef(45.0, 0.0, 0.0, 1.0);
    myWireBox(1.0, 1.0);
    glPopMatrix();
}

void DrawAxes(float len) {
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(len, 0.0, 0.0);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, len, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, len);
    glEnd();
}

void display(void) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);


    glColor3f(1.0, 1.0, 1.0);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // tramsform camera
    glTranslatef(0, 0, -cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0);   // pitch
    glRotatef(cameraAngleY, 0, 1, 0);   // heading

//    glTranslatef(0, -1.57f, 0);

    t1.start();

    // Draw grid
    printf("mode %d\n", mode);
    switch (mode) {
        case IM:
            drawGrid2D(rows, cols);
            break;
        case SA:
            drawGrid2DStoredVertices(rows, cols);
            break;
        case SAI:
            drawGrid2DStoredVerticesAndIndices(rows, cols);
            break;
        case VA:
            enableVAs();
            drawGrid2DVAs(rows, cols);
            disableVAs();
            break;
        case VBO:
            enableVBOs();
            drawGrid2DVBOs(rows, cols);
            disableVBOs();
            break;
        case nM:
            break;
    }

    DrawAxes(1);
//    DrawRobotArm();
    if (renMode == IMMEDIATE_MODE)
        renderCubeIM();
    else if (renMode == VERTEX_ARRAY || renMode == VERTEX_BUFFER_OBJECT)
        renderCubeVAVBO();
    // Does the same thing as glutSwapBuffers
    glPopMatrix();

    t1.stop(); //===============================================================
    drawTime = (float) t1.getElapsedTimeInMilliSec() - updateTime;

    showFPS();
    showInfo();
    SDL_GL_SwapWindow(window);

    // Check for OpenGL errors at least once per frame
    int err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("display: %s\n", gluErrorString(err));
    }
}

///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem() {
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points
    drawTime = updateTime = 0;
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective() {
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei) screenWidth, (GLsizei) screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float) (screenWidth) / screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void reshape(int w, int h) {
    screenWidth = w;
    screenHeight = h;
    toPerspective();
}

// Key down events
void keyDown(SDL_KeyboardEvent *e) {
    switch (e->keysym.sym) {
        case SDLK_ESCAPE:
            quit(0);
            break;
        case SDLK_LCTRL:
            printf("LCTRL\n");
            break;

        case SDLK_w:
            break;

        case SDLK_s:
            break;

        case SDLK_a:
            break;

        case SDLK_d:
            break;

        case SDLK_m:
            if (mode == VA)
                disableVAs();
            if (mode == VBO)
                disableVBOs();

            if (mode >= nM)
                mode = IM;
            if (mode == VA)
                enableVAs();
            if (mode == VBO)
                enableVBOs();
            break;

        default:
            break;
    }

    postRedisplay();
}

// Key up events
void keyUp(SDL_KeyboardEvent *e) {

}


void mouseCB(int button, int state, int x, int y) {
    mouseX = x;
    mouseY = y;

    if (button == 1) {
        if (state == SDL_MOUSEBUTTONDOWN) {
            mouseLeftDown = true;
        } else if (state == SDL_MOUSEBUTTONUP)
            mouseLeftDown = false;
    } else if (button == 3) {
        if (state == SDL_MOUSEBUTTONDOWN) {
            mouseRightDown = true;
        } else if (state == SDL_MOUSEBUTTONUP)
            mouseRightDown = false;
    }
}


void mouseMotionCB(int x, int y) {
    if (mouseLeftDown) {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if (mouseRightDown) {
        cameraDistance -= (y - mouseY) * 0.2f;
        mouseY = y;
    }
}

void eventDispatcher() {
    SDL_Event e;

    // Handle events
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                if (debug)
                    printf("Quit\n");
                quit(0);
                break;
            case SDL_MOUSEMOTION:
                mouseMotionCB(e.motion.x, e.motion.y);
                if (debug)
                    printf("Mouse moved by %d,%d to (%d,%d)\n",
                           e.motion.xrel, e.motion.yrel, e.motion.x, e.motion.y);
                postRedisplay();
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouseCB(e.button.button, SDL_MOUSEBUTTONDOWN, e.button.x, e.button.y);
                if (debug)
                    printf("Mouse button %d pressed at (%d,%d)\n",
                           e.button.button, e.button.x, e.button.y);
                postRedisplay();
                break;
            case SDL_MOUSEBUTTONUP:
                mouseCB(e.button.button, SDL_MOUSEBUTTONUP, e.button.x, e.button.y);
                if (debug)
                    printf("Mouse button %d pressed at (%d,%d)\n",
                           e.button.button, e.button.x, e.button.y);
                postRedisplay();
                break;


            case SDL_KEYDOWN:
                keyDown(&e.key);
                break;
            case SDL_WINDOWEVENT:
                if (debug)
                    printf("Window event %d\n", e.window.event);
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SHOWN:
                        if (debug)
                            SDL_Log("Window %d shown", e.window.windowID);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        if (debug)
                            printf("SDL_WINDOWEVENT_SIZE_CHANGED\n");
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        if (debug)
                            printf("SDL_WINDOWEVENT_RESIZED.\n");
                        if (e.window.windowID == SDL_GetWindowID(window)) {
                            SDL_SetWindowSize(window, e.window.data1, e.window.data2);
                            reshape(e.window.data1, e.window.data2);
                            postRedisplay();
                        }
                        break;
                    case SDL_WINDOWEVENT_CLOSE:
                        if (debug)
                            printf("Window close event\n");
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

static float tLast = 0.0;
static float frameCountElapse = 0.0;

// Used to update application state e.g. compute physics, game AI
void update() {
    float t, dt;

    t = SDL_GetTicks() / (float) milli;

    if (tLast < 0.0) {
        tLast = t;
        return;
    }

    dt = t - tLast;

    tLast = t;

    frameCountElapse += dt;

    /* Get elapsed time and convert to s */
    g.t = SDL_GetTicks();
    g.t /= 1000.0;

    /* Calculate delta t */
    g.dt = g.t - g.tLast;

    /* Update tLast for next time, using static local variable */
    g.tLast = g.t;

    UpdateVertices();
}

[[noreturn]] /*
 * Since we no longer have glutMainLoop() to do all the work for us,
 * we now have to do it ourselves. Good and bad. Good in that we have
 * more control, bad in that we have to do more work. I think that
 * good outweighs bad by a long shot :)
 *
 * This is essentially what GLUT's main loop does.
 */
void mainLoop() {
    while (true) {
        eventDispatcher();
        if (1) {
            display();
            wantRedisplay = 0;
        }
        update();
    }
}

/*
 * Function for setting up the SDL window. You don't have to do it in
 * a function like this. You can stick it directly in main() if you
 * wish.
 *
 * Do not put any OpenGL calls before this function is called!
 */
int initGraphics() {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window =
            SDL_CreateWindow("Sinwave VBO",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "%s:%d: create window failed: %s\n",
                __FILE__, __LINE__, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_GLContext mainGLContext = SDL_GL_CreateContext(window);
    if (mainGLContext == 0) {
        fprintf(stderr, "%s:%d: create context failed: %s\n",
                __FILE__, __LINE__, SDL_GetError());
        exit(EXIT_FAILURE);
    }


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    glewInit();

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    reshape(w, h);

    return 0;
}

/*
 * This is automatically called when the program exits, thanks to
 * "atexit()" in main(). This is a good place to do your program
 * shutdown and free memory, etc.
 */
void sys_shutdown() {
    SDL_Quit();
}


int main(int argc, char **argv) {
    glutInit(&argc, argv);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "%s:%d: unable to init SDL: %s\n",
                __FILE__, __LINE__, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Set up the window and OpenGL rendering context
    if (initGraphics()) {
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // OpenGL initialisation, must be done before any OpenGL calls
    init();
    initSharedMem();
    atexit(sys_shutdown);

    timer.start();

    mainLoop();

    return EXIT_SUCCESS;
}