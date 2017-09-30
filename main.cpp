#include <GL/freeglut.h>
#include <math.h>
#include <time.h>

GLdouble windowWidth, windowHeight, scale = 100, dt = 33, Pi = 3.14159265;
GLdouble Pi100 = Pi/100;	//this is for optimization

typedef struct Vertex	//just dot with pointer to next dot
{
	GLdouble x, y;
	Vertex *next;
} Vertex;

GLdouble orient(Vertex *a, Vertex *b, Vertex *c)	//from what side of line (a, b) is locating dot c
{
	  return (a->x - c->x)*(b->y - c->y) - (a->y - c->y) * (b->x - c->x);
}

class Body
{
	private:
		GLdouble vx, vy, mass_x, mass_y, W, I, angle, m, x0, y0;
		Vertex *vertexes;
	public:
		Body(GLdouble x0, GLdouble y0, GLdouble vx, GLdouble vy, GLdouble m, int vertexes_number, GLdouble size): x0(x0), y0(y0), vx(vx), vy(vy), m(m), angle(0)
		{
			vertexes = (Vertex*)malloc(sizeof(Vertex));
			Vertex *d = vertexes;
			if (vertexes_number > 1)	//cause we have already create one vertex
			{
				GLdouble da = Pi/vertexes_number;	//will create vertexes as points on the radii of a circle of radius <size>, so da is angle between adjacent radii
				size /= 100;	//preparation for the generation of random numbers not larger than <size>
				for (int i = 0, a = 0; i < vertexes_number-1; i++, a += da)	//create the remaining n-1 vertices
				{
					d->next = (Vertex*)malloc(sizeof(Vertex));
					d = d->next;
					GLdouble l = GLdouble(rand()%100)*size;	//distance from center of circle to new vertex
					d->x = cos(a)*l; d->y = sin(a)*l;	//calculating coordinates
				}
			} else {vertexes->x = 0; vertexes->y = 0;}	//polygon of one vertex?? okay
			d->next = vertexes;	//loop the list
		}

		void apply_impulse(GLdouble x, GLdouble y, Vertex *normal, GLdouble impulse)
		{	
			vx += impulse * normal->x * 1/m;
			vy += impulse * normal->y * 1/m;
			W += impulse * (normal->y * (x-mass_x) - normal->x *  (y - mass_y)) * 1/I;
		}

		void update_position(GLdouble dt)
		{
			Vertex *d = vertexes; 
			do
			{
				d->x += vx * dt; d->y += vy * dt; angle += W * dt;
				d = d->next;
			} while (d != vertexes);
		}

		void update_speed(GLdouble dt, GLdouble Fx, GLdouble Fy, GLdouble H) {vx += Fx / m * dt; vy += Fy / m * dt; W += H / I * dt;}
};

class System
{
	private:
		Body *bodies;
	public:
		System(int bodies_number, GLdouble bodies_size, GLdouble m_min, GLdouble m_max, GLdouble v_min, GLdouble v_max, int vertexes_min, int vertexes_max)
		{
			bodies = (Body*)malloc(sizeof(Body)*bodies_number);
			GLdouble x0 = bodies_size, y0 = bodies_size, v, v_angle;	//we will put bodies at the centres of grid sectors
			v_max /= 100; v_min /= (100*v_min/v_max);	//preparations for the generation of random numbers between <v_min> andd <v_max>
			m_max /= 100; m_min /= (100*v_min/v_max);	//preparations for the generation of random numbers between <m_min> and <m_max>
			int max_dv = int(v_max-v_min), max_dm = int(m_max-m_min), max_dvertexes = vertexes_max-vertexes_min;	//a bit more prepartions (in the name of optimization, of course)
			for (Body *b = bodies; b - bodies < bodies_number; b++)	//creating bodies
			{
				v = (GLfloat(rand()%max_dv) + v_min)*100; v_angle = Pi100*GLdouble(rand()%100);	//random velocity and its angle
				*b = Body(x0, y0, v*cos(v_angle), v*sin(v_angle), (GLfloat(rand()%max_dm) + m_min)*100, rand()%max_dvertexes + vertexes_min, bodies_size);
				x0 += bodies_size; if (x0 > windowWidth) {y0 += bodies_size; x0 = bodies_size;}	//moving across the greed
			}
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
 
void ChangeSize(GLsizei w, GLsizei h)	//will be call every time window resizing
{	
	GLdouble aspectRatio;
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	aspectRatio = (GLdouble)w / (GLdouble)h;
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
