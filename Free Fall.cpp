#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <vector>
#include <GL/glut.h>
#include <iostream>
#include <iomanip> //Lar meg sette cout desimaler til "alltid 2"
#include <ctime>


const GLuint FRAME_RATE = 100;
float BALL_SCALE = 10.0;

int dropHeight = 500;
int dropPosFromWall = 0;

const float gravity = -9.81;


class ball1 {
public:
    float r, g, b;
    int segments;
    double ballX;
    double ballY;
    double radius;

    double angle;
    double v0X = 0.0;
    double v0Y = 0.0;
    double velocityX = v0X; //hastighet i x retning
    double velocityY = v0Y; //hastighet i y retning
    double accelerationX;
    double accelerationY = gravity;

    int timer;

    void RandColor();
};
static std::list<ball1> balls1;


void ball1::RandColor() {
    float color[3];
    srand(static_cast <unsigned> (time(0)));
    for (int i = 0; i < 3; i++) {
        color[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    }
    r = color[0], g = color[1], b = color[2];
}


void DrawBall(int segments, double ballX, double ballY, double radius, double angle) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(ballX, ballY);
    for (int i = 0; i <= segments; i++) {
        angle = i * 2 * M_PI / segments;
        glVertex2f(ballX + radius * cos(angle), ballY + radius * sin(angle));
    }
    glEnd();
}



enum DISPLAY_LISTS {
    DL_HEIGHTLINE,
    DL_CROSSHAIR,
    DL_BALL,
    DL_MAX
};



static GLuint DisplayListsBase;
static size_t WindowWidth = 1200;
static size_t WindowHeight = 600;
static GLuint CursorX, CursorY;
static GLuint CentreX, CentreY;



void draw_line() {
    glBegin(GL_LINES);
    glVertex2f(5, 0);
    glVertex2f(5, 100);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(0, 5);
    glVertex2f(200, 5);
    glEnd();
}



/*void draw_crosshair()
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.5, -0.5);
    glVertex2f(-0.5, 0.5);
    glVertex2f(0.5, 0.5);
    glVertex2f(0.5, -0.5);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(-1.5, 0.0);
    glVertex2f(1.5, 0.0);
    glVertex2f(0.0, 1.5);
    glVertex2f(0.0, -1.5);
    glEnd();
}
*/


double compute_angle(double a_x, double a_y, double b_x, double b_y)
{
    double angle = 0.0;
    if (b_x > a_x && b_y > a_y)
        angle = atan2(b_y - a_y, b_x - a_x) * 180.0 / M_PI;
    else if (b_x < a_x && b_y > a_y)
        angle = 180.0 - (atan2(b_y - a_y, a_x - b_x) * 180.0 / M_PI);
    else if (b_x < a_x && b_y < a_y)
        angle = 180.0 + (atan2(a_y - b_y, a_x - b_x) * 180.0 / M_PI);
    else if (b_x > a_x && b_y < a_y)
        angle = 360.0 - (atan2(a_y - b_y, b_x - a_x) * 180.0 / M_PI);
    return angle;
}


double normalize_angle(double angle) {
    if (angle < 0.0) return normalize_angle(angle + 360);
    if (angle >= 360.0) return normalize_angle(angle - 360);
    return angle;
}



double compute_angle_after_collision(double angle_ball, double angle_wall) {
    double rotation = -angle_wall;
    double rotated_angle_ball = normalize_angle(angle_ball + rotation);
    double rotated_new_angle = normalize_angle(360 - rotated_angle_ball);
    double new_angle = normalize_angle(rotated_new_angle - rotation);
    return new_angle;
}



void draw_scene()
{
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0); //farge på linje
    glTranslatef(10.0, 10.0, 0.0);
    glScalef(10.0, 10.0, 1.0);
    glCallList(DisplayListsBase + DL_HEIGHTLINE);
    glPopMatrix();

    double angle = compute_angle(CentreX, CentreY, CursorX, CursorY);

    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glTranslatef(CursorX, CursorY, 0.0);
    glScalef(10.0, 10.0, 1.0);
    glCallList(DisplayListsBase + DL_CROSSHAIR);
    glPopMatrix();
    std::vector<std::list<ball1>::iterator> to_erase;

    //Check collisions with walls
    for (auto i = balls1.begin(); i != balls1.end(); i++) {


        if (i->ballY > BALL_SCALE + 60) {
            i->velocityY = i->accelerationY * i->timer * 0.01 + i->v0Y;
            i->ballY += i->velocityY;
            std::cout << std::fixed << std::setprecision(2) << "Timer:" << i->timer * 0.01 << "s" //print til console
                << " | Position: (x = " << i->ballX << ", y = " << i->ballY << ")"
                << " | Velocity x, y: (" << i->velocityX << " m/s, " << i->velocityY << " m/s)" << std::endl;
        }


        else {
            i->v0Y = fabs(i->velocityY);
            i->timer = 0;
            i->angle = compute_angle_after_collision(i->angle, 0.0);
            i->ballY += i->v0Y;
        }

        i->timer++;

        if (i->ballX < BALL_SCALE + 60) {
            i->angle = compute_angle_after_collision(i->angle, 90.0);
            i->velocityX = fabs(i->v0X);
            i->ballX += i->velocityX;
        }

        if (i->ballX < 0 || i->ballX > WindowWidth || i->ballY < 0 || i->ballY > WindowHeight) //sletter ball når den exiter vinduet
            to_erase.push_back(i);

        //check collisions with other balls
        for (auto j = balls1.begin(); j != balls1.end(); j++) {
            if (i != j && (fabs((i->ballX - j->ballX) * (i->ballX - j->ballX) +
                (i->ballY - j->ballY) * (i->ballY - j->ballY))) <=
                4.0 * BALL_SCALE * BALL_SCALE) {
                double angle_centerline = compute_angle(i->ballX, i->ballY, j->ballX, j->ballY);
                double angle_wall = normalize_angle(angle_centerline + 90.0);
                double new_angle =
                    compute_angle_after_collision(i->angle, angle_wall);
                i->angle = new_angle;
            }
        }

    }
    for (auto i = balls1.begin(); i != balls1.end(); i++)
    {
        i->ballX += i->v0X * cos(i->angle * M_PI / 180.0);
        i->ballY += i->v0Y * sin(i->angle * M_PI / 180.0);
        glPushMatrix();
        glColor3f(i->r, i->g, i->b);
        glTranslatef(i->ballX, i->ballY, 0.0);
        glRotatef(i->angle, 0.0, 0.0, 1.0);
        glScalef(100, 100, 1.0);
        glCallList(DisplayListsBase + DL_BALL);
        glPopMatrix();
    }
    for (auto i : to_erase)
        balls1.erase(i); //sletter ball
}



void init_scene()
{
    glClearColor(0, 0, 0, 0);
    glutSetCursor(GLUT_CURSOR_NONE);
    CentreX = dropPosFromWall + 200; //X-posisjon som ball droppes fra
    CentreY = dropHeight + 80; //Y-posisjon som ball droppes fra
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WindowWidth, 0, WindowHeight);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, WindowWidth, WindowHeight);
}



void make_display_lists()
{
    DisplayListsBase = glGenLists(DL_MAX);
    glNewList(DisplayListsBase + DL_HEIGHTLINE, GL_COMPILE);
    glPushMatrix();
    draw_line();
    glPopMatrix();
    glEndList();
    glNewList(DisplayListsBase + DL_CROSSHAIR, GL_COMPILE);
    glPushMatrix();
    //draw_crosshair();
    glPopMatrix();
    glEndList();
    glNewList(DisplayListsBase + DL_BALL, GL_COMPILE);
    glPushMatrix();
    DrawBall(100, 0.0, 0.0, 0.1, 0);
    glPopMatrix();
    glEndList();
}



void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    draw_scene();
    glutSwapBuffers();
}



void reshape(GLsizei nw, GLsizei nh)
{
    WindowWidth = nw;
    WindowHeight = nh;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WindowWidth, 0, WindowHeight);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, WindowWidth, WindowHeight);
}



void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        ball1 m;
        CursorX = x;
        CursorY = WindowHeight - y;
        m.RandColor();
        m.ballX = CentreX;
        m.ballY = CentreY;
        m.angle = compute_angle(CentreX, CentreY, CentreX, CentreY); //compute_angle(CentreX, CentreY, CursorX, CursorY)
        m.timer = 0;
        balls1.push_back(m);
    }
}



void passive_motion(int x, int y)
{
    CursorX = x;
    CursorY = WindowHeight - y;
}



void keyboard(unsigned char key, int x, int y)
{
    if (key == 'q' || key == 'Q')
    {
        exit(EXIT_SUCCESS);
    }
    /*if (key == '+')
    {
        dropHeight + 50;
    }
    if (key == '-')
    {
        dropHeight - 50;
    }*/
}



void timerDis(int value)
{
    glutPostRedisplay();
    glutTimerFunc(1000 / FRAME_RATE, timerDis, value);
}



int main(int argc, char* argv[]) {
    srand(static_cast <unsigned> (time(0)));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WindowWidth, WindowHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("My Window");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(passive_motion);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(1000 / FRAME_RATE, timerDis, 0);
    init_scene();
    make_display_lists();
    glutMainLoop();
    return (EXIT_SUCCESS);
}
