#include <GL/freeglut.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

GLdouble windowWidth, windowHeight, scale = 100, dt = 33, Pi2 = 2*3.14159265;

typedef struct Dot
{
	GLdouble x, y;
} Dot;

typedef struct Vertex	//just dot with pointer to next dot
{
	GLdouble x, y;
	Vertex *next;
} Vertex;

void drawLine(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	glLineWidth(1);
	glColor3ub(60, 170, 60);
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
}

GLdouble frand(GLdouble a, GLdouble b)
{
	if (a == b) return a;
	return (b-a)/100*GLdouble(rand()%100) + a;
}

class Body
{
	private:
		GLdouble vx, vy, W, I, angle, m, mass_x, mass_y;
	public:
		Vertex *vertexes;
		GLdouble x0, y0;
		int vertexes_number;
		
		void calculate_center_of_mass()
		{
			GLdouble A = 0, Gx = 0, Gy = 0, D;
			Vertex *v = vertexes;
			do
			{
				printf("D = %lf, Gx = %lf, Gy = %lf, A = %lf\n", D, Gx, Gy, A);
				D = v->x * v->next->y - v->next->x * v->y; 
				Gx += (v->x + v->next->x)*D;
				Gy += (v->y + v->next->y)*D;
				A += D;
				v = v->next;
			} while (v != vertexes);
			A *= 3; Gx /= A; Gy /= A;
			mass_x = Gx; mass_y = Gy;
			printf("mass center is (%lf, %lf), coordinates are (%lf, %lf)\n", mass_x, mass_y, x0, y0);
		}

		Body(GLdouble x0, GLdouble y0, GLdouble vx, GLdouble vy, GLdouble m, int vertexes_number, GLdouble size): x0(x0), y0(y0), vx(vx), vy(vy), m(m), angle(0), vertexes_number(vertexes_number)
		{
			vertexes = (Vertex*)malloc(sizeof(Vertex));
			vertexes->x = frand(0, size); vertexes->y = 0;
			Vertex *d = vertexes;
			GLdouble a;
			if (vertexes_number > 1)	//cause we have already create one vertex
			{
				GLdouble da = Pi2/GLdouble(vertexes_number);	//we will create vertexes as points on the radii of a circle of radius <size>, so da is angle between adjacent radii
				printf("da = %lf\n", da);
				a = da;
				for (int i = 0; i < vertexes_number-1; i++, a += da)	//create the remaining n-1 vertices
				{
					d->next = (Vertex*)malloc(sizeof(Vertex));
					d = d->next;
					GLdouble l = frand(0, size);	//distance from center of circle to new vertex
					d->x = cos(a)*l; d->y = sin(a)*l;	//calculating coordinates
					printf("a = %lf, l = %lf; (%lf, %lf)\n", a, l, d->x, d->y);
				}
			} else {vertexes->x = 0; vertexes->y = 0;}	//polygon of one vertex?? okay
			d->next = vertexes;	//loop the list
			printf("Vertexes number is %d\n", vertexes_number);
			calculate_center_of_mass();
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

		void draw()
		{
			Vertex *v = vertexes;
			do
			{
				//printf("%d %d -> %d %d", v->x + x0, v->y + y0, v->next->x + x0, v->next->y + y0);
				drawLine(v->x + x0, v->y + y0, v->next->x + x0, v->next->y + y0);
				glRectf(x0+mass_x, y0+mass_y, x0+mass_x+0.8, y0+mass_y+0.8);
				v = v->next;
			} while (v != vertexes);
		}
};

void intersects(Body &b1, Body &b2, Dot intersections[])
{
	Vertex *v1 = b1.vertexes, *v2 = b2.vertexes; int n = 0;
	do
	{
		int mass_center_orient = ((v2->x - b2.x0) * (v2->next->y - b2.y0) - (v2->y - b2.y0) * (v2->next->x - b2.x0) >= 0);
		do
		{
			if ((v2->x - v1->x) * (v2->next->y - v1->y) - (v2->y - v1->y) * (v2->next->x - v1->x) >= 0)
			{
				if (mass_center_orient)
				{
					intersections[n].x = v1->x;
					intersections[n].y = v1->y;
					++n;
				}
			}
			else
				if (!mass_center_orient)
				{
					intersections[n].x = v1->x;
					intersections[n].y = v1->y;
					++n;
				}
			v2 = v2->next;
		} while (v2 != b2.vertexes);
		v1 = v1->next;
	} while (v1 != b1.vertexes);
}

class System
{
	private:
		Body *bodies;
		int bodies_number;
	public:
		System(int bodies_number, GLdouble bodies_size, GLdouble m_min, GLdouble m_max, GLdouble v_min, GLdouble v_max, int vertexes_min, int vertexes_max): bodies_number(bodies_number)
		{
			srand(time(0));
			bodies = (Body*)malloc(sizeof(Body)*bodies_number);
			GLdouble v, v_angle, x0 = -scale + 2*bodies_size, y0 = -scale + 2*bodies_size;
			for (Body *b = bodies; b - bodies < bodies_number; b++)	//creating bodies
			{
				v = frand(v_min, v_max); v_angle = frand(0, Pi2);	//random velocity and its angle
				*b = Body(x0, y0, v*cos(v_angle), v*sin(v_angle), frand(m_min, m_max), rand()%(vertexes_max-vertexes_min) + vertexes_min, bodies_size);
				x0 += 2*bodies_size; if (x0 > scale) {y0 += 2*bodies_size; x0 = -scale + 2*bodies_size;}	//moving across the grid
			}
		}

		void draw() { for (Body *b = bodies; b - bodies < bodies_number; b++) b->draw(); }
};

System s(30, 15, 10, 10, 1, 1, 5, 7);

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
	s.draw();	
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
	printf("Heigh = %lf, width = %lf\n", windowHeight, windowWidth);
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
