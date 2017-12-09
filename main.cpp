#include <GL/freeglut.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <stdio.h>
using namespace std;

GLdouble windowWidth, windowHeight, scale = 100, dt = 4, Pi2 = 2*3.14159265;

class Vector2
{
	public:
		GLdouble x, y;
		
		Vector2(GLdouble x, GLdouble y): x(x), y(y) {}
		
		GLdouble length() { return sqrt(x*x + y*y); }

		GLdouble length2() { return x*x + y*y; }

		void rotate(GLdouble a)
		{
			GLdouble new_x = x * cos(a) - y * sin(a);
			y = y * cos(a) + x * sin(a);
			x = new_x;
		}
		
		Vector2 operator+ (Vector2 v2) { return Vector2(x + v2.x, y + v2.y); }
		Vector2 operator- (Vector2 v2) { return Vector2(x - v2.x, y - v2.y); }
		Vector2 operator+= (Vector2 v2) { return Vector2(this->x + v2.x, this->y + v2.y); }
		Vector2 operator-= (Vector2 v2) { return Vector2(this->x - v2.x, this->y - v2.y); }

		GLdouble operator* (Vector2 v2) { return x*v2.x + y*v2.y; }

		Vector2 operator* (GLdouble dt) { return Vector2(x*dt, y*dt); }
		Vector2 operator/ (GLdouble d)  { return Vector2(x/d, y/d); }
		Vector2 operator*= (GLdouble dt) { return Vector2(x*dt, y*dt); }
		Vector2 operator/= (GLdouble d) { return Vector2(x/d, y/d); }
	
		Vector2 operator^ (GLdouble w) { return Vector2(w*y, -w*x); }

		GLdouble operator^ (Vector2 v2) { return x*v2.y - y*v2.x; }

		Vector2 operator- () { return Vector2(-x, -y); }
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

		Body(Vector2 pos, Vector2 velocity, GLdouble m, int vertexes_number, GLdouble size): pos(pos), velocity(velocity), m(m), angle(0), vertexes_number(vertexes_number), size(size), W(frand(0.0005, 0.0008)), I(m*m)
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
			velocity = velocity*0.98 + norm * (impulse / m);
			W = W*0.98 + ((norm * impulse) ^ (point - pos)) / I;
			//printf("v = (%lf, %lf)\n", vx, vy);
		}

		void rotate(GLdouble a) { for (int i = 0; i < vertexes_number; i++) vertexes[i].rotate(a); }

		void move()
		{
			//printf("%lf\n", velocity);
			pos = pos + velocity*dt; rotate(W * dt);
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

bool orient(Vector2 a, Vector2 b, Vector2 c)
{
	//printf("a = (%lf, %lf), b = (%lf, %lf), c = (%lf, %lf)\n", a.x, a.y, b.x, b.y, c.x, c.y);
	if (a.x > b.x) return ((b.x - c.x)*(a.y - c.y) - (b.y - c.y) * (a.x - c.x)) >= 0;
	return ((a.x - c.x)*(b.y - c.y) - (a.y - c.y) * (b.x - c.x)) >= 0;
}

inline GLdouble sq(GLdouble x)
{
	return x*x;
}

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
				pos.x += 2.5*bodies_size; if (pos.x > scale) {pos.y += 2*bodies_size; pos.x = -scale + 2*bodies_size;}	//moving across the grid
			}
		}

		void draw() { for (int i = 0; i < bodies_number; i++) bodies[i].draw(); }

		void move() { for (int i = 0; i < bodies_number; i++) bodies[i].move(); }

		void solve_collision(Body &b1, Body &b2)
		{
			int i = 0;
			for (; i < b1.vertexes.size(); i++)
			{
				int j = 0;
				Vector2 c = b1.vertexes[i] + b1.pos;
				for (; j < b2.vertexes.size()-1; j++)
					if (orient(b2.vertexes[j] + b2.pos, b2.vertexes[j+1] + b2.pos, c) != orient(b2.vertexes[j] + b2.pos, b2.vertexes[j+1] + b2.pos, b2.pos)) break;
				
				if (j == b2.vertexes.size()-1)
					if (orient(b2.vertexes[j] + b2.pos, b2.vertexes[0] + b2.pos, c) == orient(b2.vertexes[j] + b2.pos, b2.vertexes[0] + b2.pos, b2.pos)) break;
			}
			if (i == b1.vertexes.size()) return;
			Vector2 contact_point = b1.vertexes[i] + b1.pos;

			GLdouble h, h1;
			Vector2 contact_norm(0, 0);
			for (int j = 0; j < b2.vertexes.size()-1; j++)
			{
				Vector2 a = b2.vertexes[j] + b2.pos, b = b2.vertexes[j+1] + b2.pos;
				h1 = sqrt((a-contact_point).length2() - sq((b-a)*(contact_point-a)/((b-a).length())));
				if (h1 < h || !j)
				{
					h = h1;
					//printf("h = %lf\n", h);
					contact_norm = b-a;
				}
			}
			contact_norm = Vector2(contact_norm.y, -contact_norm.x);
			contact_norm = contact_norm / contact_norm.length();
			//printf("contact_norm2 = (%lf, %lf)\n", contact_norm.x, contact_norm.y);
			//getc(stdin);
			
			Vector2 n1 = contact_norm;
			GLdouble w1 = contact_norm ^ (contact_point - b1.pos);
			Vector2 n2 = -contact_norm;
			GLdouble w2 = -(contact_norm ^ (contact_point - b2.pos));
			GLdouble velocityProjection = n1 * b1.velocity + w1 * b1.W + n2 * b2.velocity + w2 * b2.W;

			if (velocityProjection > 0) return;

			GLdouble a = n1 * b1.velocity + n2 * b2.velocity + w1 * b1.W + w2 * b2.W;
			GLdouble b = n1 * n1 / b1.m + w1 * w1 / b1.I + n2 * n2 / b2.m + w2 * w2 / b2.I;

			GLdouble lambda = -a / b / 3;
			if (lambda < 0.5) lambda = 0.5;

			b2.apply_impulse(contact_point, contact_norm, -lambda);
			b1.apply_impulse(contact_point, contact_norm, lambda);
		}

		void solve_collisions()
		{
			for (int i = 0; i < bodies.size()-1; i++)
			{
				for (int j = i+1; j < bodies.size(); j++)
				{
					if ( (bodies[i].pos - bodies[j].pos).length() > bodies[i].size + bodies[j].size ) continue;
					solve_collision(bodies[i], bodies[j]);
				}
			}
		}
};

System s(60, 10, 20, 20, 0.01, 0.02, 3, 5);

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
	s.move();
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
