#include <GL/freeglut.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <stdio.h>
using namespace std;

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
	public:
		GLdouble vx, vy, W, I, angle, m;
		Vertex *vertexes;
		GLdouble x0, y0, size;
		int vertexes_number;

		Body(GLdouble x0, GLdouble y0, GLdouble vx, GLdouble vy, GLdouble m, int vertexes_number, GLdouble size): x0(x0), y0(y0), vx(vx), vy(vy), m(m), angle(0), vertexes_number(vertexes_number), size(size), W(0), I(m*m)
		{
			vertexes = (Vertex*)malloc(sizeof(Vertex));
			vertexes->x = frand(0, size); vertexes->y = 0;
			Vertex *d = vertexes;
			GLdouble a;
			if (vertexes_number > 1)	//cause we have already create one vertex
			{
				GLdouble da = Pi2/GLdouble(vertexes_number);	//we will create vertexes as points on the radii of a circle of radius <size>, so da is angle between adjacent radii
				a = da;
				for (int i = 0; i < vertexes_number-1; i++, a += da)	//create the remaining n-1 vertices
				{
					d->next = (Vertex*)malloc(sizeof(Vertex));
					d = d->next;
					GLdouble l = frand(0, size);	//distance from center of circle to new vertex
					d->x = cos(a)*l; d->y = sin(a)*l;	//calculating coordinates
				}
			} else {vertexes->x = 0; vertexes->y = 0;}	//polygon of one vertex?? okay
			d->next = vertexes;	//loop the list
			
			GLdouble A = 0, Gx = 0, Gy = 0, D;
			d = vertexes;
			do
			{
				D = d->x * d->next->y - d->next->x * d->y; 
				Gx += (d->x + d->next->x)*D;
				Gy += (d->y + d->next->y)*D;
				A += D;
				d = d->next;
			} while (d != vertexes);
			A *= 3; Gx /= A; Gy /= A;
			d = vertexes; do {d->x -= Gx; d->y -= Gy; d = d->next;} while (d != vertexes);
		}

		void apply_impulse(GLdouble x, GLdouble y, GLdouble normal_x, GLdouble normal_y, GLdouble impulse)
		{	
			vx += impulse * normal_x * 1/m;
			vy += impulse * normal_y * 1/m;
			W += impulse * (normal_y * (x-x0) - normal_x *  (y - y0)) * 1/I;
			//printf("v = (%lf, %lf)\n", vx, vy);
		}

		void rotate(GLdouble a)
		{
			Vertex *v = vertexes;
			do
			{
				v->x = v->x * cos(a) - v->y * sin(a);
				v->y = v->y * cos(a) + v->x * sin(a);
				v = v->next;
			} while (v != vertexes);
		}

		void move(GLdouble dt)
		{
			rotate(0.01);
			x0 += vx * dt; y0 += vy * dt; angle += W * dt;
		}

		void update_speed(GLdouble dt, GLdouble Fx, GLdouble Fy, GLdouble H) {vx += Fx / m * dt; vy += Fy / m * dt; W += H / I * dt;}

		void draw()
		{
			Vertex *v = vertexes;
			do
			{
				drawLine(v->x + x0, v->y + y0, v->next->x + x0, v->next->y + y0);
				glRectf(x0, y0, x0+0.8, y0+0.8);
				v = v->next;
			} while (v != vertexes);
		}
};

void intersects(Body &b1, Body &b2, vector<Vertex*> &intersections)
{
	GLdouble dx = fabs(b1.x0 - b2.x0), dy = fabs(b1.y0 - b2.y0), min_d = b1.size+b2.size;
	if (dx > min_d || dy > min_d) return;
	Vertex *v1 = b1.vertexes, *v2 = b2.vertexes;
	int mass_center_orient, interflag;
	do
	{
		mass_center_orient = (v2->x + b2.x0 - b2.x0) * (v2->next->y + b2.y0 - b2.y0) - (v2->y + b2.y0 - b2.y0) * (v2->next->x + b2.x0 - b2.x0); interflag = 1;
		do
		{
			if ((v2->x + b2.x0 - v1->x - b1.x0) * (v2->next->y + b2.y0 - v1->y - b1.y0) - (v2->y + b2.y0 - v1->y - b1.y0) * (v2->next->x + b2.x0 - v1->x - b1.x0) < 0)
			{
				if (mass_center_orient >= 0)
				{
					interflag = 0;
					break;
				}
			}
			else
				if (mass_center_orient < 0)
				{
					interflag = 0;
					break;
				}
			v2 = v2->next;
		} while (v2 != b2.vertexes);
		
		if (interflag)
		{
			intersections.push_back(v1);
			break;
		}
		
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

		void move(GLdouble dt) { for (Body *b = bodies; b < bodies+bodies_number; b++) b->move(dt); }

		void solve_collision(Body &b1, Body &b2, Vertex &a, Vertex &b, Vertex &c)
		{
			GLdouble R1_x = c.x - b1.x0, R1_y = c.y - b1.y0,
			R2_x = c.x - b2.x0, R2_y = c.y - b2.y0,

			normal_x = (b.y-a.y), normal_y = (a.x-b.x);

			GLdouble normal_length = sqrt(normal_x*normal_x + normal_y*normal_y);

			normal_x /= normal_length; normal_y /= normal_length;
			//printf("%lf %lf\n", normal_x, normal_y);

			GLdouble	vab =	normal_x * (b1.vx - b1.W * R1_y - b2.vx + b2.W * R2_y) + 
								normal_y * (b1.vy + b1.W * R1_x - b2.vy - b2.W * R2_x),
										
								Z1 = (normal_y * R1_x - normal_x * R1_y) * 1/b1.I,
								Z2 = (normal_y * R2_x - normal_x * R2_y) * 1/b2.I;
						
			GLdouble	J =	normal_x *  (normal_x * 1/b1.m - R1_y * Z1 + normal_x * 1/b2.m + R2_y * Z2) 
							+ normal_y * (normal_y * 1/b1.m + R1_x * Z1 * 1/b1.I + normal_y * 1/b2.m - R2_x * Z2 * 1/b2.I),
						E = 0.1;

			GLdouble impulse = (0.01 - (1 + E) * vab) / J;
			//if (impulse <= 0) break;

			b1.apply_impulse(c.x, c.y, normal_x, normal_y, impulse);
			b2.apply_impulse(c.x, c.y, normal_x, normal_y, -impulse);
		}

		void solve_collisions()
		{
			for (Body *b1 = bodies; b1 < bodies+bodies_number; b1++)
			{
				for (Body *b2 = b1+1; b2 < bodies+bodies_number; b2++)
				{
					
					//GLdouble dx = fabs(b1->x0 - b2->x0), dy = fabs(b1->y0 - b2->y0), min_d = b1->size+b2->size;
					//if (dx > min_d || dy > min_d) return;
					Vertex *v1 = b1->vertexes, *v2 = b2->vertexes;
					int mass_center_orient, interflag;
					do
					{
						mass_center_orient = (v2->x + b2->x0 - b2->x0) * (v2->next->y + b2->y0 - b2->y0) - (v2->y + b2->y0 - b2->y0) * (v2->next->x + b2->x0 - b2->x0);
						interflag = 1;
						do
						{
							if ((v2->x + b2->x0 - v1->x - b1->x0) * (v2->next->y + b2->y0 - v1->y - b1->y0) - (v2->y + b2->y0 - v1->y - b1->y0) * (v2->next->x + b2->x0 - v1->x - b1->x0) < 0)
							{
								if (mass_center_orient >= 0)
								{
									interflag = 0;
									break;
								}
							}
							else
								if (mass_center_orient < 0)
								{
									interflag = 0;
									break;
								}
							v2 = v2->next;
						} while (v2 != b2->vertexes);
			
						if (interflag)
						{
							//printf("COLLISION\n");
							solve_collision(*b1, *b2, *v2, *v2->next, *v1);
							//break;
						}
						
						v1 = v1->next;
					} while (v1 != b1->vertexes);

					v1 = b1->vertexes, v2 = b2->vertexes;
					do
					{
						mass_center_orient = (v1->x + b1->x0 - b1->x0) * (v1->next->y + b1->y0 - b1->y0) - (v1->y + b1->y0 - b1->y0) * (v1->next->x + b1->x0 - b1->x0);
						interflag = 1;
						do
						{
							if ((v1->x + b1->x0 - v2->x - b2->x0) * (v1->next->y + b1->y0 - v2->y - b2->y0) - (v1->y + b1->y0 - v2->y - b2->y0) * (v1->next->x + b1->x0 - v2->x - b2->x0) < 0)
							{
								if (mass_center_orient >= 0)
								{
									interflag = 0;
									break;
								}
							}
							else
								if (mass_center_orient < 0)
								{
									interflag = 0;
									break;
								}
							v1 = v1->next;
						} while (v1 != b1->vertexes);
			
						if (interflag)
						{
							//printf("COLLISION\n");
							solve_collision(*b2, *b1, *v1, *v1->next, *v2);
							//break;
						}
						
						v2 = v2->next;
					} while (v2 != b2->vertexes);

				}
			}
		}
};

System s(10, 20, 10, 10, 0.02, 0.04, 3, 4);

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
}

void TimerFunction(int value)
{
	s.solve_collisions();
	s.move(dt);
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
