#include <GL/freeglut.h>
#include <math.h>
#include <time.h>
using namespace std;

GLfloat size = 0.6, windowWidth, windowHeight, scale = 100, dt = 33;

typedef struct Dot
{
	GLfloat x, y;
	Dot *next;
} Dot;

GLfloat orient(Dot *a, Dot *b, Dot *c)
{
	  return (a->x - c->x)*(b->y - c->y) - (a->y - c->y) * (b->x - c->x);
}

class Body
{
	private:
		GLfloat vx, vy, mass_x, mass_y, W, I, angle, m;
		Dot *dots;
	public:
		void apply_impulse(GLfloat x, GLfloat y, Dot *normal, GLfloat impulse)
		{	
			vx += impulse * normal->x * 1/m;
			vy += impulse * normal->y * 1/m;
			W += impulse * (normal->y * (x-mass_x) - normal->x *  (y - mass_y)) * 1/I;
		}
		void update_position(GLfloat dt)
		{
			for (Dot *d = dots; d->next; d++)
			{
				d->x += vx * dt; d->y += vy * dt; angle += W * dt;
			}
		}
		void update_speed(GLfloat dt, GLfloat Fx, GLfloat Fy, GLfloat H) {vx += Fx / m * dt; vy += Fy / m * dt; W += H / I * dt;}
};

class System
{
	private:
		Body *bodies;
	public:
		System(int bodies_number, GLfloat bodies_size, GLfloat min_m, GLfloat max_m, GLfloat v_min, GLfloat v_max, int vertexes_min, int vertexes_max)
		{
			
		}
};

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	glutSwapBuffers();
}


void SetupRC(void)
{
	glClearColor(0.0f,0.0f,0.0f,1.0f);
}
 
void ChangeSize(GLsizei w, GLsizei h) {
	GLfloat aspectRatio;
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	aspectRatio = (GLfloat)w / (GLfloat)h;
	if (w <= h){
		windowWidth = scale;
		windowHeight = scale / aspectRatio;
		glOrtho(-scale, scale, -windowHeight, windowHeight, 1.0, -1.0);
	} else {
		windowWidth = scale * aspectRatio;
		windowHeight = scale;
		glOrtho(-windowWidth, windowWidth, -scale, scale, 1.0, -1.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void TimerFunction(int value)
{
	
	glutPostRedisplay();
	glutTimerFunc(dt, TimerFunction, 1);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("Physics");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutTimerFunc(dt, TimerFunction, 1);
	SetupRC();
	glutMainLoop();
	return 0;
}
