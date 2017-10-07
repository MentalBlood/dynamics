#include <GL/freeglut.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <stdio.h>
using namespace std;

GLdouble windowWidth, windowHeight, scale = 100, dt = 33, Pi2 = 2*3.14159265;

class Vector2
{
	public:
		GLdouble x, y;
		Vector2(GLdouble x, GLdouble y): x(x), y(y) {}
		
		GLdouble length() { return sqrt(x*x + y*y); }

		void rotate(GLdouble a)
		{
			GLdouble new_x = x * cos(a) - y * sin(a);
			y = y * cos(a) + x * sin(a);
			x = new_x;
		}
		
		Vector2 operator+ (Vector2 v2) { return Vector2(x + v2.x, y + v2.y); }
		Vector2 operator- (Vector2 v2) { return Vector2(x - v2.x, y - v2.y); }
		Vector2 operator+= (Vector2 v2) { return Vector2(x + v2.x, y + v2.y); }
		Vector2 operator-= (Vector2 v2) { return Vector2(x - v2.x, y - v2.y); }

		GLdouble operator* (Vector2 v2) { return x*v2.x + y*v2.y; }

		Vector2 operator* (GLdouble dt) { return Vector2(x*dt, y*dt); }
		Vector2 operator/ (GLdouble d)  { return Vector2(x/d, y/d); }
		Vector2 operator*= (GLdouble dt) { return Vector2(x*dt, y*dt); }
		Vector2 operator/= (GLdouble d) { return Vector2(x/d, y/d); }
	
		Vector2 operator^ (GLdouble w) { return Vector2(w*y, -w*x); }

		GLdouble operator^ (Vector2 v2) { return x*v2.y - y*v2.x; }
};

void drawLine(Vector2 a, Vector2 b)
{
	glLineWidth(1);
	glColor3ub(60, 170, 60);
	glBegin(GL_LINES);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, b.y);
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
		Vector2 velocity, pos;
		vector<Vector2> vertexes;
		GLdouble W, I, angle, m, size;
		int vertexes_number;

		Body(Vector2 pos, Vector2 velocity, GLdouble m, int vertexes_number, GLdouble size): pos(pos), velocity(velocity), m(m), angle(0), vertexes_number(vertexes_number), size(size), W(0), I(m*m)
		{
			GLdouble da = Pi2 / GLdouble(vertexes_number), a = 0;
			for (int i = 0; i < vertexes_number; i++)
			{
				vertexes.push_back( Vector2(size*cos(a), size*sin(a)) );
				a += da;
			}
			
			GLdouble A = 0, D;
			Vector2 mass_center(0, 0);
			for (int i = 0; i < vertexes_number-1; i++)
			{
				D = vertexes[i] ^ vertexes[i+1];
				mass_center += (vertexes[i] + vertexes[i+1]) * D;
				A += D;
			}
			D = vertexes[vertexes_number-1] ^ vertexes[0];
			mass_center += (vertexes[vertexes_number-1] + vertexes[0]) * D;
			A += D;

			mass_center /= 3*A;
			for (int i = 0; i < vertexes_number; i++) vertexes[i] -= mass_center;
		}

		void apply_impulse(Vector2 point, Vector2 norm, GLdouble impulse)
		{	
			velocity += norm * (impulse / m);
			W += ((norm * impulse) ^ (point - pos)) / I;
			//printf("v = (%lf, %lf)\n", vx, vy);
		}

		void rotate(GLdouble a) { for (int i = 0; i < vertexes_number; i++) vertexes[i].rotate(a); }

		void move(GLdouble dt)
		{
			rotate(0.01);
			pos += velocity*dt; angle += W * dt;
		}

		void draw()
		{
			for (int i = 0; i < vertexes_number-1; i++)
			{
				drawLine(vertexes[i] + pos, vertexes[i+1] + pos);
				glRectf(pos.x-0.4, pos.y-0.4, pos.x+0.4, pos.y+0.4);
			}
			drawLine(vertexes[0] + pos, vertexes[vertexes_number-1] + pos);
		}
};

class System
{
	private:
		vector<Body> bodies;
		int bodies_number;
	public:
		System(int bodies_number, GLdouble bodies_size, GLdouble m_min, GLdouble m_max, GLdouble v_min, GLdouble v_max, int vertexes_min, int vertexes_max): bodies_number(bodies_number)
		{
			srand(time(0));
			GLdouble v, v_angle;
			Vector2 pos(-scale + 2*bodies_size, -scale + 2*bodies_size);
			for (int i = 0; i < bodies_number; i++)	//creating bodies
			{
				v = frand(v_min, v_max); v_angle = frand(0, Pi2);	//random velocity and its angle
				bodies.push_back( Body(pos, Vector2(v*cos(v_angle), v*sin(v_angle)), frand(m_min, m_max), rand()%(vertexes_max-vertexes_min + 1) + vertexes_min, bodies_size) );
				pos.x += 2*bodies_size; if (pos.x > scale) {pos.y += 2*bodies_size; pos.x = -scale + 2*bodies_size;}	//moving across the grid
			}
		}

		void draw() { for (int i = 0; i < bodies_number; i++) bodies[i].draw(); }

		void move(GLdouble dt) { for (int i = 0; i < bodies_number; i++) bodies[i].move(dt); }

		void solve_collision(Body &b1, Body &b2)
		{
			
		}

		void solve_collisions()
		{
			
		}
};

System s(10, 20, 10, 10, 0.02, 0.04, 3, 5);

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
