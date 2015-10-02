//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <string>
#include <X11/keysym.h>
#include <unistd.h>
#include <GL/glx.h>
extern "C" {
    #include "fonts.h"
}

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 360

#define MAX_PARTICLES 40000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

#define NUM_BOXES 5
struct Game {
    Shape box[NUM_BOXES];
 	Shape circle; // new from class
    Particle particle[MAX_PARTICLES];
    int n;
    int lastMousex, lastMousey;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    game.n=0;

    int xOrigin = 100;
    int yOrigin = 250;
    int width = 80;
    int height = 10;

    //declare a box shape
   	game.box[0].width = width;
    game.box[0].height = height;
    game.box[0].center.x = xOrigin;
    game.box[0].center.y = yOrigin;

    game.box[1].width = width;
    game.box[1].height = height;
    game.box[1].center.x = xOrigin + 50;
    game.box[1].center.y = yOrigin - 30;

    game.box[2].width = width;
    game.box[2].height = height;
    game.box[2].center.x = xOrigin + 100;
    game.box[2].center.y = yOrigin - 60;

    game.box[3].width = width;
    game.box[3].height = height;
    game.box[3].center.x = xOrigin + 150;
    game.box[3].center.y = yOrigin - 90;

    game.box[4].width = width;
    game.box[4].height = height;
    game.box[4].center.x = xOrigin + 200;
    game.box[4].center.y = yOrigin - 120;

	game.circle.center.x = 400;
    game.circle.center.y = -20;
    game.circle.radius = 100;

	

    //start animation
    while(!done) {
        while(XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_mouse(&e, &game);
            done = check_keys(&e, &game);
        }
        movement(&game);
        render(&game);
        glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                            ButtonPress | ButtonReleaseMask |
                            PointerMotionMask |
                            StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
                    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
	//initialize fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

#define rnd()(float)rand() / (float)RAND_MAX
void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
        return;
    std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y =   rnd()*.8*+.02;
    p->velocity.x =  .5 + rnd()*0.5;
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    //static int n = 0;

    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed
            //int y = WINDOW_HEIGHT - e->xbutton.y;
            for (int i = 0; i <10; i++)
            	//makeParticle(game, e->xbutton.x, y);
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed
            return;
        }
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        int y = WINDOW_HEIGHT - e->xbutton.y;
        for (int i = 0; i <10; i++){
            //makeParticle(game, e->xbutton.x, y);
        }

        //for (++n < 10)
        //return;

        game->lastMousex = e->xbutton.x;
        game->lastMousey = y;
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
        //You may check other keys here.
		int key2 = XLookupKeysym(&e->xkey, 1);
		if(key2 == XK_B){
			int x = game->lastMousex;
			int y = game->lastMousey;
			makeParticle(game, x, y);
		}

    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if (game->n <= 0)
        return;

    for(int i = 0; i < 10; i++){
      makeParticle(game, game->lastMousex, game->lastMousey);

    }

    for( int i=0; i<game->n; i++){
        p = &game->particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
        p->velocity.y -= GRAVITY;


    //check for collision with shapes...
    for(int j = 0; j < NUM_BOXES; j++){
      Shape *s = &game->box[j];
      if(p->s.center.y < s->center.y + s->height &&
            p->s.center.y > s->center.y - s->height &&
            p->s.center.x >= s->center.x - s->width &&
            p->s.center.x <= s->center.x + s->width){
        p->velocity.y *= -0.4;

      }


	 //check circle collision
      float d0,d1,dist;
	d0 = p->s.center.x - game->circle.center.x;
	d1 = p->s.center.y - game->circle.center.y;
	dist = sqrt(d0*d0 + d1*d1);
	if (dist < game->circle.radius) {
	    p->s.center.x = game->circle.center.x + (game->circle.radius * d0/dist);
	    p->s.center.y = game->circle.center.y + (game->circle.radius * d1/dist);
	    p->velocity.x += d0/dist;
	    p->velocity.y += d1/dist;
	}





//    box->width = 100;
//	box->height = 10;
//	box->center.x = 120 + 5*65;
//    box->center.y = 500 - 5*60;



    //check for off-screen
    if (p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT) {
        //std::cout << "off screen" << std::endl;
        memcpy(&game->particle[i], &game->particle[game->n-1], sizeof(Particle));
        game->n--;
    }
    }
}
}


void render(Game *game)
{
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...

	 static int firsttime = 1;
    static int verts[60][2];
    static int n = 60;
    
    glColor3ub(50, 150,50);
    if(firsttime){
      float angle = 0;
      float inc = (3.14459 * 2.0) / (float)n;
      for(int i = 0; i < n; i++){
		verts[i][0] = cos(angle) * game->circle.radius + game->circle.center.x;
		verts[i][1] = sin(angle) * game->circle.radius + game->circle.center.y;
		angle += inc;
	
	
      }
      
	
	firsttime = 0;
      }
      
     
	glPushMatrix();
        glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < n; i++){
	  glVertex2i(verts[i][0],  verts[i][1]);
	
        
      
    }
    
	glEnd();
        glPopMatrix();

	Rect rect;
    rect.bot=WINDOW_HEIGHT-30;
    rect.left=0;
    rect.center=0;
    ggprint16(&rect, 36, 0x00ffffff,"Waterfall Model");

	

	

	

    

    //draw boxes here
    for(int j = 0; j < NUM_BOXES; j++){
        Shape *s;
        glColor3ub(50,50,50);
        s = &game->box[j];
        glPushMatrix();
        glTranslatef(s->center.x, s->center.y, s->center.z);
        w = s->width;
        h = s->height;
        glBegin(GL_QUADS);
        glVertex2i(-w,-h);
        glVertex2i(-w, h);
        glVertex2i( w, h);
        glVertex2i( w,-h);
        glEnd();
        glPopMatrix();
    }

	


    //draw all particles here
    for(int i=0; i < game->n; i++){

		int red = rand() % 20;
		int green = rand() % 100;
		int blue = rand() % 255;

        glPushMatrix();

        glColor3ub(red,green, blue);

        Vec *c = &game->particle[i].s.center;
        w = 2;
        h = 2;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w, c->y-h);
        glVertex2i(c->x-w, c->y+h);
        glVertex2i(c->x+w, c->y+h);
        glVertex2i(c->x+w, c->y-h);
        glEnd();
        glPopMatrix();
    }

	int bot = 240;
	int left = 55;
	int center = 0;

	Rect rect2;
    rect2.bot=bot;
    rect2.left=left;
    rect2.center=center;
    ggprint12(&rect2, 36, 0x00ffffff,"Requirements");

	Rect rect3;
    rect3.bot=bot-30;
    rect3.left=left+75;
    rect3.center=center;
    ggprint12(&rect3, 36, 0x00ffffff,"Design");

	Rect rect4;
    rect4.bot=bot - 60;
    rect4.left= left + 130;
    rect4.center=0;
    ggprint12(&rect4, 36, 0x00ffffff,"Coding");

	Rect rect5;
    rect5.bot= bot -90;
    rect5.left= left + 180;
    rect5.center=0;
    ggprint12(&rect5, 36, 0x00ffffff,"Testing");

	Rect rect6;
    rect6.bot= bot - 120;
    rect6.left= left + 210;
    rect6.center=0;
    ggprint12(&rect6, 36, 0x00ffffff,"Maintenance");


}



