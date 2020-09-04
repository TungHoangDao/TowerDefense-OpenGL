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

typedef struct {
    float x, y;
} vec2f;

typedef struct {
    float x, y, z;
} vec3f;

typedef struct {
    vec3f r, n;
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

void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);

GLuint
createVBO(const void *data, int dataSize, GLenum target = GL_ARRAY_BUFFER_ARB, GLenum usage = GL_STATIC_DRAW_ARB);
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
    ss << "MODE (SPACE): " << MODE_STRING[(int)renMode] << std::ends;  // add 0(ends) at the end
    drawString(ss.str().c_str(), 1, screenHeight - TEXT_HEIGHT, color, font);
    ss.str(""); // clear buffer

    ss << std::fixed << std::setprecision(3);
    ss << "Updating Time: " << updateTime << " ms" << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (3 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Drawing Time: " << drawTime << " ms" << " MAX: " << max << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (5 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Light (l): " << (lightMode ? "on" : "off") << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (7 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Fill (f): " << (fillMode ? "on" : "off") << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (9 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Drawing: " << "row: " << rows << " col: " << cols << " Vertices: " << n_vertices << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (11 * TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Use ARROW to change rows and cols" << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight - (13 * TEXT_HEIGHT), color, font);

    ss << "Press SPACE key to toggle between Mode" << std::ends;
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
    glEnableClientState(GL_NORMAL_ARRAY);
}

void disableVAs() {
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
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
    glEnableClientState(GL_NORMAL_ARRAY);

}

void disableVBOs() {
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void calcSineWave3D(sinewave wave, float x, float z, double t, float *y, bool der, float *dydx) {
    float angle = wave.k * x * x + wave.k * z * z + wave.w * t;
//    float angle = wave.k * z * z  + wave.w * t;

    *y = wave.A * sinf(angle);
    if (der) {
        *dydx = wave.k * wave.A * cosf(angle);
    }
}
void drawGrid2D(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, fillMode == LINE ? GL_LINE : GL_FILL);
    glColor3f(1.0, 1.0, 1.0);

    float nx,ny,nz;
    float dydx;

    /* Grid */
    float dy = 20.0f / (float) rows;
    float dx = 20.0f / (float) cols;
    for (int i = 0; i < cols; i++) {
        float x = -1.0 + i * dx;
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= rows; j++) {
            float z = -1.0 + j * dy;
            float y;

            calcSineWave3D(sws[0], x, z, timer.getElapsedTime(), &y, true, &dydx);
            ny = dydx;
            nx = 1.0;
            nz = 0;

            glNormal3f(-ny,nx,nz);
            glVertex3f(x, y, z);

            calcSineWave3D(sws[0], x+dx, z, timer.getElapsedTime(), &y, true, &dydx);
            ny = dydx;
            nx = 1.0;
            nz = 0;

            glNormal3f(-ny,nx,nz);
            glVertex3f(x + dx, y, z);
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


    float nx,ny,nz;
    float dydx;

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
            calcSineWave3D(sws[0], x, z, timer.getElapsedTime(), &y, true, &dydx);
            ny = dydx;
            nx = 1.0;
            nz = 0;

            vtx->r = (vec3f) {x, y, z};
            vtx->n = (vec3f){-ny,nx,nz};

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

void drawGrid2DStoredVertices(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, fillMode == LINE ? GL_LINE : GL_FILL);
    glColor3f(1.0, 1.0, 1.0);

    /* Grid */
    for (int i = 0; i < cols; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= rows; j++) {
            int idx = i * (rows + 1) + j;
#ifdef DEBUG_DRAW_GRID_ARRAY
            printf("%d %d %d %f %f\n", i, j, idx, vertices[idx].r.x, vertices[idx].r.y);
#endif
            glNormal3fv(&vertices[idx].n.x);
            glVertex3fv(&vertices[idx].r.x);
            idx += rows + 1;
#ifdef DEBUG_DRAW_GRID_ARRAY
            printf("%d %d %d %f %f\n", i, j, idx, vertices[idx].r.x, vertices[idx].r.y);
#endif
            glNormal3fv(&vertices[idx].n.x);
            glVertex3fv(&vertices[idx].r.x);
        }
        glEnd();
    }

    glPopAttrib();
}
void drawGrid2DStoredVerticesAndIndices(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, fillMode == LINE ? GL_LINE : GL_FILL);
    glColor3f(1.0, 1.0, 1.0);

    /* Grid */
    unsigned *idx = indices;
    for (int i = 0; i < cols; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= rows; j++) {
            glNormal3fv(&vertices[*idx].n.x);
            glVertex3fv(&vertices[*idx].r.x);
            idx++;

            glNormal3fv(&vertices[*idx].n.x);
            glVertex3fv(&vertices[*idx].r.x);
            idx++;
        }
        glEnd();
    }

    glPopAttrib();
}
void drawGrid2DVAs(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, fillMode == LINE ? GL_LINE : GL_FILL);
    glColor3f(1.0, 1.0, 1.0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &vertices[0].r);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), &vertices[0].n);

    for (int i = 0; i < cols; i++)
        glDrawElements(GL_TRIANGLE_STRIP, (rows + 1) * 2, GL_UNSIGNED_INT, &indices[i * (rows + 1) * 2]);

    glPopAttrib();
}
void drawGrid2DVBOs(int rows, int cols) {
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, fillMode == LINE ? GL_LINE : GL_FILL);
    glColor3f(1.0, 1.0, 1.0);

    bindVBOs();
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
    glNormalPointer(GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec3f)));

    /* Grid */
    for (int i = 0; i < cols; i++) {
        glDrawElements(GL_TRIANGLE_STRIP, (rows + 1) * 2, GL_UNSIGNED_INT,
                       BUFFER_OFFSET(i * (rows + 1) * 2 * sizeof(unsigned int)));
    }
    unbindVBOs();
    glPopAttrib();
}


///////////////////////////////////////////////////////////////////////////////
// wobble the vertex in and out along normal
///////////////////////////////////////////////////////////////////////////////
void updateVerticesIM(Vertex *vertices,unsigned count,float time)
{
    if (STATIC_RENDERING)
    {
        return;
    }

    if(!vertices)
        return;

    float x, y, z;
    float nx,ny,nz;
    float dydx;

    for(int i=0; i < count; ++i)
    {
        x = vertices[i].r.x;
        y = vertices[i].r.y;
        z = vertices[i].r.z;

        calcSineWave3D(sws[0],x,z,time,&y,true,&dydx);
        ny = dydx;

        // update vertex coords
        vertices[i].r.y = y;
        vertices[i].n.x = -ny;
    }
}



///////////////////////////////////////////////////////////////////////////////
// wobble the vertex in and out along normal
///////////////////////////////////////////////////////////////////////////////
void updateVertices(Vertex* dstVertices, Vertex* srcVertices, int count, float time)
{
    if (STATIC_RENDERING)
    {
        return;
    }

    if(!dstVertices || !srcVertices)
        return;

    float x, y, z;

    for(int i=0; i < count; ++i)
    {
        x = srcVertices[i].r.x;
        y = srcVertices[i].r.y;
        z = srcVertices[i].r.z;

        float dxdy;
        calcSineWave3D(sws[0],x,z,time,&y,true,&dxdy);

        // update vertex coords
        dstVertices[i].r.y = y;
        dstVertices[i].n.x = -dxdy;
    }
}

void checkForGLerrors(int lineno) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
        printf("%d: %s\n", lineno, gluErrorString(error));
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
    float lightPos[4] = {0, 0, 20, 5}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration

//    glLightfv(GL_LIGHT1, GL_AMBIENT, lightKa);
//    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightKd);
//    glLightfv(GL_LIGHT1, GL_SPECULAR, lightKs);
//
//    // position the light
//    float lightPos1[4] = {0, 0, 20, 5}; // positional light
//    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
//
//    glEnable(GL_LIGHT1);
//
//    glLightfv(GL_LIGHT2, GL_AMBIENT, lightKa);
//    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightKd);
//    glLightfv(GL_LIGHT2, GL_SPECULAR, lightKs);
//
//    // position the light
//    float lightPos2[4] = {0, 0, 20, 5}; // positional light
//    glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);
//
//    glEnable(GL_LIGHT2);
//
//    glLightfv(GL_LIGHT3, GL_AMBIENT, lightKa);
//    glLightfv(GL_LIGHT3, GL_DIFFUSE, lightKd);
//    glLightfv(GL_LIGHT3, GL_SPECULAR, lightKs);
//
//    // position the light
//     float lightPos3[4] = {0, 0, 20, 5}; // positional light
//    glLightfv(GL_LIGHT3, GL_POSITION, lightPos3);
//
//    glEnable(GL_LIGHT3);
//
//    glLightfv(GL_LIGHT4, GL_AMBIENT, lightKa);
//    glLightfv(GL_LIGHT4, GL_DIFFUSE, lightKd);
//    glLightfv(GL_LIGHT4, GL_SPECULAR, lightKs);
//
//    // position the light
//    float lightPos4[4] = {0, 0, 20, 5}; // positional light
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPos4);
//
//    glEnable(GL_LIGHT4);
//
//    glLightfv(GL_LIGHT5, GL_AMBIENT, lightKa);
//    glLightfv(GL_LIGHT5, GL_DIFFUSE, lightKd);
//    glLightfv(GL_LIGHT5, GL_SPECULAR, lightKs);
//
//    // position the light
//    float lightPos5[4] = {0, 0, 20, 5}; // positional light
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPos5);
//
//    glEnable(GL_LIGHT5);
//
//    glLightfv(GL_LIGHT6, GL_AMBIENT, lightKa);
//    glLightfv(GL_LIGHT6, GL_DIFFUSE, lightKd);
//    glLightfv(GL_LIGHT6, GL_SPECULAR, lightKs);
//
//    // position the light
//   float  lightPos6[4] = {0, 0, 20, 5}; // positional light
//    glLightfv(GL_LIGHT6, GL_POSITION, lightPos6);
//
//    glEnable(GL_LIGHT6);
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

    glTranslatef(0, -1.57f, 0);

    t1.start();

    DrawAxes(1);

    // Draw grid
    if (renMode == IMMEDIATE_MODE)
    {
        drawGrid2D(rows, cols);
    } else if (renMode == STORE_ARRAY)
    {
        // measure the elapsed time of updateVertices()
        t2.start(); //---------------------------------------------------------
        updateVerticesIM(vertices,n_vertices,(float)timer.getElapsedTime());
        t2.stop(); //----------------------------------------------------------
        updateTime = (float)t2.getElapsedTimeInMilliSec();

        drawGrid2DStoredVertices(rows,cols);
    }
    else if (renMode == STORE_ARRAY_INDICE)
    {
        // measure the elapsed time of updateVertices()
        t2.start(); //---------------------------------------------------------
        updateVerticesIM(vertices,n_vertices,(float)timer.getElapsedTime());
        t2.stop(); //----------------------------------------------------------
        updateTime = (float)t2.getElapsedTimeInMilliSec();

        drawGrid2DStoredVerticesAndIndices(rows,cols);
    }
    else if (renMode == VERTEXT_ARRAY)
    {
        enableVAs();

        // measure the elapsed time of updateVertices()
        t2.start(); //---------------------------------------------------------
        updateVerticesIM(vertices,n_vertices,(float)timer.getElapsedTime());
        t2.stop(); //----------------------------------------------------------
        updateTime = (float)t2.getElapsedTimeInMilliSec();

        drawGrid2DVAs(rows,cols);
        disableVAs();
    }
    else if (renMode == VERTEX_BUFFER_OBJECT)
    {
        enableVBOs();

        // measure the elapsed time of updateVertices()
        t2.start(); //---------------------------------------------------------
        if (!STATIC_RENDERING)
        {
            // map the buffer object into client's memory
            // Note that glMapBuffer() causes sync issue.
            // If GPU is working with this buffer, glMapBufferARB() will wait(stall)
            // for GPU to finish its job.
            auto *ptr = (Vertex *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
            if(ptr)
            {
                // wobble vertex in and out along normal
                updateVertices(ptr, vertices, n_vertices, (float)timer.getElapsedTime());
                glUnmapBuffer(GL_ARRAY_BUFFER);     // release pointer to mapping buffer
            }
        }

        t2.stop(); //----------------------------------------------------------
        updateTime = (float)t2.getElapsedTimeInMilliSec();

        drawGrid2DVBOs(rows, cols);
        disableVBOs();
    }

    glPopMatrix();

    t1.stop(); //===============================================================
    drawTime = (float) t1.getElapsedTimeInMilliSec();

    if (drawTime > max)
    {
        max = drawTime;
    }

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
// destroy a VBO
// If VBO id is not valid or zero, then OpenGL ignores it silently.
///////////////////////////////////////////////////////////////////////////////
void deleteVBO()
{
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &vbo);
    ibo = vbo = 0;
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
    max = -999;


    switch (e->keysym.sym) {
        case SDLK_ESCAPE:
            quit(0);
            break;
        case SDLK_LCTRL:
            printf("LCTRL\n");
            break;

        case SDLK_p:
            PAUSE = !PAUSE;
            break;

        case SDLK_w:
            break;

        case SDLK_s:
            STATIC_RENDERING = !STATIC_RENDERING;
            break;

        case SDLK_a:
            break;

        case SDLK_d:
            break;

        case SDLK_UP:
            rows+=10;
            computeAndStoreGrid2D(rows,cols);
            deleteVBO();
            buildVBOs();
            break;
        case SDLK_DOWN:
            rows-=10;
            computeAndStoreGrid2D(rows,cols);
            deleteVBO();
            buildVBOs();
            break;
        case SDLK_LEFT:
            cols-=10;
            computeAndStoreGrid2D(rows,cols);
            deleteVBO();
            buildVBOs();
            break;
        case SDLK_RIGHT:
            cols+=10;
            computeAndStoreGrid2D(rows,cols);
            deleteVBO();
            buildVBOs();
            break;


        case SDLK_l:
            lightMode = !lightMode;
            if (lightMode)
            {
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
            }
            else
            {
                glDisable(GL_LIGHTING);
                glDisable(GL_LIGHT0);
            }
            break;

        case SDLK_SPACE:
        {
            renMode = (RenderMode)((int)renMode+1 < 5 ? (int)renMode+1 : 0);
            break;
        }

        case SDLK_1:
            renMode = IMMEDIATE_MODE;
            break;

        case SDLK_2:
            renMode = STORE_ARRAY;
            break;

        case SDLK_3:
            renMode = STORE_ARRAY_INDICE;
            break;

        case SDLK_4:
            renMode = VERTEXT_ARRAY;
            break;

        case SDLK_5:
            renMode = VERTEX_BUFFER_OBJECT;
            break;

        case SDLK_f:
            fillMode = (FillingMode)((int)fillMode+1 < 2 ? (int)fillMode+1 : 0);
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

// Used to update application state e.g. compute physics, game AI
void update() {
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
        if (!PAUSE)
        {
            if (1) {
                display();
                wantRedisplay = 0;
            }

            update();
        }
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



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

    // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
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
//    initGL();
    atexit(sys_shutdown);

    timer.start();

    mainLoop();

    return EXIT_SUCCESS;
}