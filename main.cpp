#include <GL/freeglut.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <stdio.h>
using namespace std;

GLdouble windowWidth, windowHeight, scale = 100, dt = 5, Pi2 = 2*3.14159265;
GLdouble borderline = scale;

class Vector2
{
	public:
		GLdouble x, y;
		
		Vector2(GLdouble x, GLdouble y): x(x), y(y) {}
		Vector2(): x(0), y(0) {}
		
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
		Vector2& operator+= (const Vector2& v2) { this->x += v2.x; this->y += v2.y; return *this; }
		Vector2& operator-= (const Vector2& v2) { this->x -= v2.x; this->y -= v2.y; return *this; }

		GLdouble operator* (Vector2 v2) { return x*v2.x + y*v2.y; }

		Vector2 operator* (GLdouble dt) { return Vector2(x*dt, y*dt); }
		Vector2 operator/ (GLdouble d)  { return Vector2(x/d, y/d); }
		Vector2& operator*= (const GLdouble& dt) { this->x *= dt, this->y *= dt; return *this; }
		Vector2& operator/= (const GLdouble& d) { this->x /= d, this->y /= d; return *this; }
	
		Vector2 operator^ (GLdouble w) { return Vector2(w*y, -w*x); }

		GLdouble operator^ (Vector2 v2) { return x*v2.y - y*v2.x; }

		Vector2 operator- () { return Vector2(-x, -y); }
};

void drawLine(Vector2 a, Vector2 b)
{
	glLineWidth(1);
	glColor3ub(rand()%255, rand()%255, rand()%255);
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
			Vector2 mass_center;
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
			W *= 0.9;
		}

		void rotate(GLdouble a) { for (int i = 0; i < vertexes_number; i++) vertexes[i].rotate(a); }

		void move()
		{
			pos = pos + velocity*dt; rotate(W * dt);
		}

		void draw()
		{
			for (int i = 0; i < vertexes_number-1; i++)
			{
				drawLine(vertexes[i] + pos, vertexes[i+1] + pos);
				//glRectf(pos.x-0.4, pos.y-0.4, pos.x+0.4, pos.y+0.4);
			}
			drawLine(vertexes[0] + pos, vertexes[vertexes_number-1] + pos);
		}
};

bool orient(Vector2 &a, Vector2 &b, Vector2 &c)
{
	if (a.x > b.x) return ((b.x - c.x)*(a.y - c.y) - (b.y - c.y) * (a.x - c.x)) >= 0;
	return ((a.x - c.x)*(b.y - c.y) - (a.y - c.y) * (b.x - c.x)) >= 0;
}

inline GLdouble sq(GLdouble x) { return x*x; }

void gravitate_pair(Body &b1, Body &b2)
{
	Vector2 direction = b1.pos - b2.pos;
	if (direction.length() < 20) return;
	Vector2 dv = direction * (b1.m * b2.m / (sq(direction.length()))) * dt / 5000000;
	b1.velocity -= dv;
	b2.velocity += dv;
}

class System
{
	private:
		vector<Body> bodies;
		int bodies_number;
	public:
		System(int bodies_number, GLdouble bodies_size_min, GLdouble bodies_size_max, GLdouble m_min, GLdouble m_max, GLdouble v_min, GLdouble v_max, int vertexes_min, int vertexes_max): bodies_number(bodies_number)
		{
			srand(time(0));
			GLdouble v, v_angle;
			Vector2 pos(-scale + 2*bodies_size_max, -scale + 2*bodies_size_max);
			for (int i = 0; i < bodies_number; i++)	//creating bodies
			{
				v = frand(v_min, v_max); v_angle = frand(0, Pi2);	//random velocity and its angle
				bodies.push_back( Body(pos, Vector2(v*cos(v_angle), v*sin(v_angle)), frand(m_min, m_max), rand()%(vertexes_max-vertexes_min + 1) + vertexes_min, frand(bodies_size_min, bodies_size_max)) );
				pos.x += 3*bodies_size_max; if (pos.x > scale) {pos.y += 3*bodies_size_max; pos.x = -scale + 3*bodies_size_max;}	//moving across the grid
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
				Vector2 a, b, c = b1.vertexes[i] + b1.pos;
				for (; j < b2.vertexes.size()-1; j++)
				{
					a = b2.vertexes[j] + b2.pos;
					b = b2.vertexes[j+1] + b2.pos;
					if (orient(a, b, c) != orient(a, b, b2.pos)) break;
				}
				if (j != b2.vertexes.size()-1) continue;
				a = b2.vertexes[j] + b2.pos;
				b = b2.vertexes[0] + b2.pos;
				if (orient(a, b, c) != orient(a, b, b2.pos)) continue;;
			
				Vector2 contact_point = b1.vertexes[i] + b1.pos;

				GLdouble h, h1;
				Vector2 contact_norm;
				for (j = 0; j < b2.vertexes.size()-1; j++)
				{
					a = b2.vertexes[j] + b2.pos, b = b2.vertexes[j+1] + b2.pos;
					h1 = sqrt((a-contact_point).length2() - sq((b-a)*(contact_point-a)/((b-a).length())));
					if ((h1 < h) || !j) { h = h1; contact_norm = b-a; }
				}
				a = b2.vertexes[b2.vertexes.size()-1] + b2.pos, b = b2.vertexes[0] + b2.pos;
				h1 = sqrt((a-contact_point).length2() - sq((b-a)*(contact_point-a)/((b-a).length())));
				if (h1 < h) { h = h1; contact_norm = b-a; }

				contact_norm = Vector2(contact_norm.y, -contact_norm.x);
				contact_norm = contact_norm / contact_norm.length();
				
				Vector2 n1 = contact_norm;
				GLdouble w1 = contact_norm ^ (contact_point - b1.pos);
				Vector2 n2 = -contact_norm;
				GLdouble w2 = -(contact_norm ^ (contact_point - b2.pos));
				GLdouble velocityProjection = n1 * b1.velocity + w1 * b1.W + n2 * b2.velocity + w2 * b2.W;

				b1.pos += contact_norm*h / 1.9;
				b2.pos -= contact_norm*h / 1.9;
				
				if (velocityProjection > 0) continue;

				GLdouble initialVelocityProjection = n1 * b1.velocity + w1 * b1.W + n2 * b2.velocity + w2 * b2.W;
				GLdouble B = n1 * n1 / b1.m + w1 * w1 / b1.I + n2 * n2 / b2.m + w2 * w2 / b2.I;
				GLdouble bounce = -1;
				GLdouble A = (n1 * b1.velocity + n2 * b2.velocity + w1 * b1.W + w2 * b2.W) + bounce * initialVelocityProjection;

				GLdouble lambda = -A / B;
				if (lambda < 0) continue;
				if (lambda < 0.3) lambda = 0.3;
				
				b2.apply_impulse(contact_point, contact_norm, -lambda);
				b1.apply_impulse(contact_point, contact_norm, lambda);
			}
		}

		void solve_collisions()
		{
			for (int i = 0; i < bodies.size(); i++)
			{
				for (int j = 0; j < bodies.size(); j++)
				{
					if (i == j) continue;
					if ( (bodies[i].pos - bodies[j].pos).length() > bodies[i].size + bodies[j].size ) continue;
					solve_collision(bodies[i], bodies[j]);
				}

				bool wall_bounce = false;
				for (int j = 0; j < bodies[i].vertexes.size(); j++)
				{
					GLdouble d = bodies[i].vertexes[j].x + bodies[i].pos.x - borderline;
					if (d > 0)
					{
						bodies[i].pos.x -= d;
						bodies[i].velocity.x *= -1;
						wall_bounce = true;
						continue;
					}
					d = bodies[i].vertexes[j].x + bodies[i].pos.x + borderline;
					if (d < 0)
					{
						bodies[i].pos.x -= d;
						bodies[i].velocity.x *= -1;
						wall_bounce = true;
						continue;
					}
					d = bodies[i].vertexes[j].y + bodies[i].pos.y - borderline;
					if (d > 0)
					{
						bodies[i].pos.y -= d;
						bodies[i].velocity.y *= -1;
						wall_bounce = true;
						continue;
					}
					d = bodies[i].vertexes[j].y + bodies[i].pos.y + borderline;
					if (d < 0)
					{
						bodies[i].pos.y -= d;
						bodies[i].velocity.y *= -1;
						wall_bounce = true;
						continue;
					}
				}
				if (wall_bounce) bodies[i].W *= -1;
			}
		}

		void gravitate()
		{
			for (int i = 0; i < bodies.size()-1; i++)
				for (int j = i+1; j < bodies.size(); j++)
				{
					gravitate_pair(bodies[i], bodies[j]);
				}
		}
};

System s(50, 3, 8, 20, 20, 0.05, 0.11, 3, 10);

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.1, 0.1, 0.1);
	glRectf(-scale, -scale, scale, scale);
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
	if (h == 0) h = 1;
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
	s.gravitate();
	s.move();
	glutPostRedisplay();
	glutTimerFunc(dt, TimerFunction, 1);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("Dynamics");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutTimerFunc(dt, TimerFunction, 1);
	SetupRC();
	glutMainLoop();
	return 0;
}
