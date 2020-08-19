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
#include <stdlib.h>
#include <math.h>
#include <cstdio>


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

typedef struct { float x, y; } vec2f;

// Globals
bool debug = true;
float shoulder, elbow, wrist, left_plate, right_plate;
SDL_Window *window;
const float gripper_increment = .2;
const int milli = 1000;



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

typedef struct { float x, y, z; } vec3f;
typedef struct { vec3f r, n; } Vertex;

Vertex *vertices;
unsigned *indices;
unsigned n_vertices, n_indices;
unsigned vbo, ibo;
unsigned rows = 4, cols = 4;

enum { IM = 0, SA, SAI, VA, VBO, nM } mode = IM;

void enableVAs()
{
    glEnableClientState(GL_VERTEX_ARRAY);
}

void disableVAs()
{
    glDisableClientState(GL_VERTEX_ARRAY);
}

void bindVBOs()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void unbindVBOs()
{
    glBindBuffer(0, vbo);
    glBindBuffer(0, ibo);
}

void buildVBOs()
{
    glGenBuffers(1, &vbo); //buffer for vertex
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, n_vertices * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo); //buffer for indice
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices * sizeof(unsigned int), indices, GL_STATIC_DRAW);
}

void enableVBOs()
{
    glEnableClientState(GL_VERTEX_ARRAY);

}

void disableVBOs()
{
    glDisableClientState(GL_VERTEX_ARRAY);

}


void drawGrid2D(int rows, int cols)
{
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

void computeAndStoreGrid2D(int rows, int cols)
{
    n_vertices = (rows + 1) * (cols + 1);
    n_indices = (rows + 1) * (cols - 1) * 2 + (rows + 1) * 2;
    // or more simply: n_indices = n_vertices * 2;
    free(vertices);
    vertices = (Vertex *)malloc(n_vertices * sizeof(Vertex));
    free(indices);
    indices = (unsigned *)malloc(n_indices * sizeof(unsigned));


    /* Grid */

    /* Vertices */
    float dy = 2.0f / (float) rows;
    float dx = 2.0f / (float) cols;
    Vertex *vtx = vertices;
    for (int i = 0; i <= cols; i++) {
        float x = -1.0 + i * dx;
        for (int j = 0; j <= rows; j++) {
            float y = -1.0 + j * dy;
            vtx->r = (vec3f) { x, y, 0.0 };
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
            printf("(%5.2f,%5.2f)", vertices[idx].r.x, vertices[idx].r.y);
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
void drawGrid2DStoredVertices(int rows, int cols)
{
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

void drawGrid2DStoredVerticesAndIndices(int rows, int cols)
{
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);

    /* Grid */
    unsigned  *idx = indices;
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

void drawGrid2DVAs(int rows, int cols)
{
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glVertexPointer(3, GL_FLOAT,sizeof(Vertex), &vertices[0].r);

    for (int i = 0; i < cols; i++)
        glDrawElements(GL_TRIANGLE_STRIP, (rows + 1) * 2, GL_UNSIGNED_INT, &indices[i * (rows + 1) * 2]);

    glPopAttrib();
}

void drawGrid2DVBOs(int rows, int cols)
{
    glPushAttrib(GL_CURRENT_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);

    bindVBOs();
    glVertexPointer(3, GL_FLOAT,sizeof(Vertex), BUFFER_OFFSET(0));
    /* Grid */
    for (int i = 0; i < cols; i++) {
        glDrawElements(GL_TRIANGLE_STRIP, (rows + 1) * 2, GL_UNSIGNED_INT, BUFFER_OFFSET(i * (rows + 1) * 2 * sizeof(unsigned int)));
    }
    unbindVBOs();
    glPopAttrib();
}



void checkForGLerrors(int lineno)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
        printf("%d: %s\n", lineno, gluErrorString(error));
}


/* Immediate mode, vertex at a time */
void renderCubeIM()
{
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
void renderCubeVAVBO()
{
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
void postRedisplay()
{
    wantRedisplay = 1;
}

void quit(int code)
{
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(code);
}

// OpenGL initialisation
void init(void)
{
    glShadeModel (GL_FLAT);
    glColor3f(1.0, 1.0, 1.0);
    computeAndStoreGrid2D(rows, cols);
    buildVBOs();
}


// Draws a wireframe box
void myWireBox(float l, float h)
{
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
void myWireDiamond(float l, float h)
{
    glPushMatrix();
    glScalef(l, h, 1.0);
    glScalef(1.0/sqrt(2.0), 1.0/sqrt(2.0), 1.0);
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

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);

    /* Oblique view, scale cube */
    glRotatef(15.0, 1.0, 0.0, 0.0);
    glRotatef(-30.0, 0.0, 1.0, 0.0);
    glScalef(0.5, 0.5, 0.5);

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
    SDL_GL_SwapWindow(window);

    // Check for OpenGL errors at least once per frame
    int err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("display: %s\n",gluErrorString(err));
    }
}

void reshape(int w, int h)
{
//    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-5.0, 20.0, -5.0, 20.0, 0.1, 100.0);
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    glTranslatef (0.0, 0.0, -5.0);

    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Key down events
void keyDown(SDL_KeyboardEvent *e)
{
    switch (e->keysym.sym) {
        case SDLK_ESCAPE:
            quit(0);
            break;
        case SDLK_LCTRL:
            printf("LCTRL\n");
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
}

// Key up events
void keyUp(SDL_KeyboardEvent *e)
{

}

void eventDispatcher()
{
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
                if (debug)
                    printf("Mouse moved by %d,%d to (%d,%d)\n",
                           e.motion.xrel, e.motion.yrel, e.motion.x, e.motion.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (debug)
                    printf("Mouse button %d pressed at (%d,%d)\n",
                           e.button.button, e.button.x, e.button.y);
                break;
            case SDL_KEYDOWN:
                keyDown(&e.key);
                break;
            case SDL_WINDOWEVENT:
                if (debug)
                    printf("Window event %d\n", e.window.event);
                switch (e.window.event)
                {
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
void update()
{
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
}

[[noreturn]] /*
 * Since we no longer have glutMainLoop() to do all the work for us,
 * we now have to do it ourselves. Good and bad. Good in that we have
 * more control, bad in that we have to do more work. I think that
 * good outweighs bad by a long shot :)
 *
 * This is essentially what GLUT's main loop does.
 */
void mainLoop()
{
    while (1) {
        eventDispatcher();
        if (wantRedisplay) {
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
int initGraphics()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window =
            SDL_CreateWindow("Robot Arm Using SDL2",
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
void sys_shutdown()
{
    SDL_Quit();
}


int main(int argc, char** argv)
{

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

    atexit(sys_shutdown);

    mainLoop();

    return EXIT_SUCCESS;
}