#include <GL/freeglut.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <stdio.h>
using namespace std;

#include "engine.cpp"
#include "UI.cpp"
#include "keys.cpp"

int main(int argc, char **argv)
{
	UI();
	active_system_index = 0;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("Dynamics");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutTimerFunc(dt, TimerFunction, 1);
	SetupRC();
	glutKeyboardFunc(key);
	setup_keys();
	glutMainLoop();
	return 0;
}
