#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <vector>
#include <GL/glut.h>
#include <iostream>
#include <iomanip> 
#include <ctime>
#include <sstream> 
#include <string>
#include <random>

using namespace std;

const GLuint FRAME_RATE = 100;
float BALL_SCALE = 10.0;

int dropHeight = 500;
int dropPosFromWall = 0;

const float gravity = -9.81;
float airDensity = 1;
float sphereDragCoefficient = 0.47;

float footballRadius = 0.11; // m

static bool paused = false;
bool fullScreen = 0;

class ball1 {
public:
    float r, g, b;
    int segments;
    double ballX;
    double ballY;
    double radius;
    float mass = 0.45; //kg

    double angle;
    double v0X = 1.0;
    double v0Y = 0.0;
    double velocityX = v0X; //hastighet i x retning
    double velocityY = v0Y; //hastighet i y retning
    double accelerationX;
    double accelerationY = gravity;

    float area = 2 * M_PI * (footballRadius * footballRadius);
    double airRecistanceCoefficient = (airDensity * sphereDragCoefficient * area) / 2;
    double Vt = sqrt((mass * abs(gravity)) / airRecistanceCoefficient);

    int timerX, timerY;

    void RandColor();
};
static list<ball1> balls1;


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
static size_t WindowWidth = 1400;
static size_t WindowHeight = 700;
static GLuint CursorX, CursorY;
static GLuint CentreX, CentreY;


void drawText(const char* text, int length, int x, int y) { //skrive tekst
    glMatrixMode(GL_PROJECTION);
    double* matrix = new double[16];
    glGetDoublev(GL_PROJECTION_MATRIX, matrix);
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -5, 5);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2i(x, y);
    for (int i = 0; i < length; i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)text[i]);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(matrix);
    glMatrixMode(GL_MODELVIEW);
}


void draw_line() {
    glBegin(GL_LINES);
    glVertex2f(5, 0);
    glVertex2f(5, 100); //y-axis
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(0, 5);
    glVertex2f(200, 5); //x-axis
    glEnd();
    /*glBegin(GL_LINES);
    glVertex2f(112.5, 0);
    glVertex2f(112.5, 100); //y-axis
    glEnd();*/
    glBegin(GL_LINES);
    glVertex2f(2.5, 10);
    glVertex2f(5, 10);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 15);
    glVertex2f(5, 15);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 20);
    glVertex2f(5, 20);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 25);
    glVertex2f(5, 25);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 30);
    glVertex2f(5, 30);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 35);
    glVertex2f(5, 35);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 40);
    glVertex2f(5, 40);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 45);
    glVertex2f(5, 45);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 50);
    glVertex2f(5, 50);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 55);
    glVertex2f(5, 55);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(2.5, 60);
    glVertex2f(5, 60);
    glEnd();
}


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

void draw_scene() {
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
    vector<list<ball1>::iterator> to_erase;

    string text1;
    text1 = "OpenGL - Bouncy Ball"; //text output
    glColor3f(0, 1, 0);
    drawText(text1.data(), text1.size(), 350, 575); //text position 

    string textESC;
    textESC = "Press 'ESC' to exit."; //text output
    glColor3f(1, 1, 1);
    drawText(textESC.data(), textESC.size(), 50, 20); //text position

    string textF;
    textF = "Press 'F' to go fullscreen."; //text output
    glColor3f(1, 1, 1);
    drawText(textF.data(), textF.size(), 175, 20); //text position

// graph horizontal lines
	double y_val = 90;
	string text5;
	text5 = "5"; //text output
	glColor3f(1, 1, 1);
	drawText(text5.data(), text5.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();


	y_val = 132.5;
	string text10;
	text10 = "10"; //text output
	glColor3f(1, 1, 1);
	drawText(text10.data(), text10.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 175;
	string text15;
	text15 = "15"; //text output
	glColor3f(1, 1, 1);
	drawText(text15.data(), text15.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 217.5;
	string text20;
	text20 = "20"; //text output
	glColor3f(1, 1, 1);
	drawText(text20.data(), text20.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 260;
	string text25;
	text25 = "25"; //text output
	glColor3f(1, 1, 1);
	drawText(text25.data(), text25.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 302.5;
	string text30;
	text30 = "30"; //text output
	glColor3f(1, 1, 1);
	drawText(text30.data(), text30.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 345;
	string text35;
	text35 = "35"; //text output
	glColor3f(1, 1, 1);
	drawText(text35.data(), text35.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 387.5;
	string text40;
	text40 = "40"; //text output
	glColor3f(1, 1, 1);
	drawText(text40.data(), text40.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 430;
	string text45;
	text45 = "45"; //text output
	glColor3f(1, 1, 1);
	drawText(text45.data(), text45.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 472.5;
	string text50;
	text50 = "50"; //text output
	glColor3f(1, 1, 1);
	drawText(text50.data(), text50.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();

	y_val = 515;
	string text55;
	text55 = "55"; //text output
	glColor3f(1, 1, 1);
	drawText(text55.data(), text55.size(), 7.5, y_val); //text position
	glBegin(GL_LINES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50, y_val * 1.5);
	glVertex2f(WindowWidth, y_val * 1.5);
	glEnd();


    //Check collisions with walls
    for (auto i = balls1.begin(); i != balls1.end(); i++) {

        if (i->ballY > BALL_SCALE + 60) {
            i->accelerationY = ((gravity * i->mass) + (i->airRecistanceCoefficient * i->velocityY * i->velocityY)) / i->mass;
            i->velocityY = i->accelerationY * i->timerY * 0.01 + i->v0Y;                                                            //i->velocityY = i->v0Y + (i->Vt * tanh((gravity * i->timerY * 0.01) / i->Vt));
            i->ballY += i->velocityY;
            i->timerY++;
        }

        else {
            i->v0Y = fabs(i->velocityY);
            i->timerY = 0;
            i->angle = compute_angle_after_collision(i->angle, 0.0);
            i->ballY += i->v0Y;
        }

        if (i->ballX > BALL_SCALE + 60) {
            i->accelerationX = -(i->airRecistanceCoefficient * i->velocityX * i->velocityX) / i->mass;
            i->velocityX = i->accelerationX * i->timerX * 0.01 + i->v0X;
            i->ballX += i->v0X;
            i->timerX++;
        }

        else {
            i->v0X = fabs(i->velocityX);
            i->timerX = 0;
            i->angle = compute_angle_after_collision(i->angle, 90.0);
            i->ballX += i->v0X;
        }

        if (i->ballX < 0 || i->ballX > WindowWidth || i->ballY < 0 || i->ballY > WindowHeight) //sletter ball når den exiter vinduet
            to_erase.push_back(i);


        cout << fixed << setprecision(2) << "Time:" << i->timerY * 0.01 << "s" //print til console
            << " | Position: (x = " << i->ballX << ", y = " << i->ballY << ")"
            << " | Velocity x, y: (" << i->velocityX << " m/s, " << i->velocityY << " m/s)" << endl;
        stringstream buffer;
        buffer << fixed << setprecision(2) << "Time:" << i->timerY * 0.01 << "s" //cout til string
            << " | Position: (x = " << i->ballX << ", y = " << i->ballY << ")"
            << " | Velocity x, y: (" << i->velocityX << " m/s, " << i->velocityY << " m/s)";
        string output = buffer.str();
        glColor3f(1, 0, 0);
        drawText(output.data(), output.size(), 350, 20);


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
        glScalef(1.0, 1.0, 1.0);
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
    DrawBall(100, 0, 0, 10, 0);
    glPopMatrix();
    glEndList();
}



void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    draw_scene();
    glutSwapBuffers();

    glBegin(GL_POLYGON);
    glColor3f(1.0, 0.0, 0.0);

    glVertex2d(0, 0);
    glColor3f(0.0, 1.0, 0.0);

    glVertex2d(0, 100);
    glColor3f(1.0, 0.0, 0.0);

    glVertex2d(100, 100);
    glColor3f(0.0, 0.0, 1.0);

    glVertex2d(100, 0);

    glEnd();
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
        m.timerX = 0;
        m.timerY = 0;
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
    if (key == 27)
    {
        exit(EXIT_SUCCESS);
    }


    if (key == 'f' || key == 'F')
    {
        glutFullScreen();
    }

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
