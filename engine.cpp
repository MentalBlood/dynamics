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

class Color
{
	public:
		unsigned char R, G, B;
		Color(unsigned char R, unsigned char G, unsigned char B): R(R), G(G), B(B) {}
		Color(): R(rand()%200 + 55), G(rand()%200 + 55), B(rand()%200 + 55) {}
};

void drawLine(Vector2 a, Vector2 b, Color& color)
{
	glLineWidth(1);
	glColor3ub(color.R, color.G, color.B);
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
		Color color;

		Body(Vector2 pos, Vector2 velocity, GLdouble m, int vertexes_number, GLdouble size, Color color):
			pos(pos), velocity(velocity), m(m), angle(0), size(size), W(frand(0.0005, 0.0008)), color(color), I(m*m)
		{
			GLdouble alpha = Pi2 / GLdouble(vertexes_number), rotation = 0, r;
			GLdouble min_size = size*cos(alpha);
			if (min_size < size/2) min_size = size/2;
			for (int i = 0; i < vertexes_number; i++)
			{
				r = frand(min_size, size);
				vertexes.push_back( Vector2(r*cos(rotation), r*sin(rotation)) );
				rotation += alpha;
			}
			
			GLdouble A = 0, D;
			Vector2 mass_center;
			for (int i = 0; i < vertexes.size()-1; i++)
			{
				D = vertexes[i] ^ vertexes[i+1];
				mass_center += (vertexes[i] + vertexes[i+1]) * D;
				A += D;
			}
			D = vertexes[vertexes.size()-1] ^ vertexes[0];
			mass_center += (vertexes[vertexes.size()-1] + vertexes[0]) * D;
			A += D;

			mass_center /= 3*A;
			for (int i = 0; i < vertexes.size(); i++) vertexes[i] -= mass_center;
		}

		void apply_impulse(Vector2 point, Vector2 norm, GLdouble impulse)
		{	
			velocity += norm * (impulse / m);
			W += ((norm * impulse) ^ (point - pos)) / I;
			W *= 0.9;
		}

		void rotate(GLdouble a) { for (int i = 0; i < vertexes.size(); i++) vertexes[i].rotate(a); }

		void move() { pos = pos + velocity*dt; rotate(W * dt); }
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
	//if (direction.length() < 10) return;
	Vector2 dv = direction * (b1.m * b2.m / (sq(direction.length()))) * dt / 4500000;
	b1.velocity -= dv;
	b2.velocity += dv;
}

void reverse_gravitate_pair(Body &b1, Body &b2)
{
	Vector2 direction = b1.pos - b2.pos;
	//if (direction.length() < 10) return;
	Vector2 dv = direction * (b1.m * b2.m / (sq(direction.length()))) * dt / 4500000;
	b1.velocity += dv;
	b2.velocity -= dv;
}

class Color3f
{
	public:
		GLdouble R, G, B;
		Color3f(): R(frand(0.1, 1)), G(frand(0.1, 1)), B(frand(0.1, 1)) {}
		Color3f(GLdouble R, GLdouble G, GLdouble B): R(R), G(G), B(B) {}
};

class System
{
	private:
		bool stoped, reversed_gravitation;
		Vector2 center;

	public:
		char *name;
		Vector2 down_left_area_corner, up_right_area_corner;
		Color3f color;
		vector<Body> bodies;

		System(	int bodies_number, GLdouble bodies_size_min, GLdouble bodies_size_max, GLdouble m_min, 
				GLdouble m_max, GLdouble v_min, GLdouble v_max, int vertexes_min, int vertexes_max, 
				Vector2 down_left_area_corner, Vector2 up_right_area_corner, Color3f color, char *system_name ): 
			down_left_area_corner(down_left_area_corner), 
			up_right_area_corner(up_right_area_corner), 
			color(color),
			stoped(false),
			reversed_gravitation(false),
			center((down_left_area_corner + up_right_area_corner)/2)
		{
			name = (char*)malloc(sizeof(char)*(strlen(system_name)+1));
			strcpy(name, system_name);
			srand(time(0));
			GLdouble v, v_angle;
			Vector2 pos(down_left_area_corner.x + 2*bodies_size_max, down_left_area_corner.y + 2*bodies_size_max);
			for (int i = 0; i < bodies_number; i++)	//creating bodies
			{
				v = frand(v_min, v_max); v_angle = frand(0, Pi2);	//random velocity and its angle
				GLdouble m = frand(m_min, m_max);
				bodies.push_back( Body(	pos, Vector2(v*cos(v_angle), v*sin(v_angle)), m, rand()%(vertexes_max-vertexes_min + 1) + vertexes_min, frand(bodies_size_min, bodies_size_max) * 2*m/(m_min + m_max), Color()) );
				pos.x += 2.5*bodies_size_max; if (pos.x + bodies_size_max > up_right_area_corner.x) {pos.y += 2.5*bodies_size_max; pos.x = down_left_area_corner.x + 2*bodies_size_max;}	//moving across the grid
			}
		}

		void draw()
		{
			glColor3f(color.R, color.G, color.B);
			glRectf(down_left_area_corner.x, down_left_area_corner.y, up_right_area_corner.x, up_right_area_corner.y);
			
			for (int i = 0; i < bodies.size(); i++)
			{
				for (int j = 0; j < bodies[i].vertexes.size()-1; j++)
					drawLine(bodies[i].vertexes[j] + bodies[i].pos, bodies[i].vertexes[j+1] + bodies[i].pos, bodies[i].color);
				glRectf(bodies[i].pos.x-0.4, bodies[i].pos.y-0.4, bodies[i].pos.x+0.4, bodies[i].pos.y+0.4);
				drawLine(bodies[i].vertexes[0] + bodies[i].pos, bodies[i].vertexes[bodies[i].vertexes.size()-1] + bodies[i].pos, bodies[i].color);
			}
		}

		void move_bodies() { if (stoped) return; for (int i = 0; i < bodies.size(); i++) bodies[i].move(); }	

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

				bool right_bounce = false, left_bounce = false, down_bounce = false, up_bounce = false, bounce = false;
				for (int j = 0; j < bodies[i].vertexes.size(); j++)
				{
					GLdouble d = bodies[i].vertexes[j].x + bodies[i].pos.x - up_right_area_corner.x;
					if (d > 0 && !right_bounce)
					{
						bodies[i].pos.x -= d*1.1;
						bodies[i].velocity.x *= -1;
						right_bounce = true;
						bounce = true;
						continue;
					}
					d = bodies[i].vertexes[j].x + bodies[i].pos.x - down_left_area_corner.x;
					if (d < 0 && !left_bounce)
					{
						bodies[i].pos.x -= d*1.1;
						bodies[i].velocity.x *= -1;
						left_bounce = true;
						bounce = true;
						continue;
					}
					d = bodies[i].vertexes[j].y + bodies[i].pos.y - down_left_area_corner.y;
					if (d < 0 && !down_bounce)
					{
						bodies[i].pos.y -= d*1.1;
						bodies[i].velocity.y *= -1;
						down_bounce = true;
						bounce = true;
						continue;
					}
					d = bodies[i].vertexes[j].y + bodies[i].pos.y - up_right_area_corner.y;
					if (d > 0 && !up_bounce)
					{
						bodies[i].pos.y -= d*1.1;
						bodies[i].velocity.y *= -1;
						up_bounce = true;
						bounce = true;
						continue;
					}
				}
				if (bounce) bodies[i].W *= -1;
			}
		}

		void gravitate()
		{
			for (int i = 0; i < bodies.size()-1; i++)
				for (int j = i+1; j < bodies.size(); j++)
				{
					if (reversed_gravitation) reverse_gravitate_pair(bodies[i], bodies[j]);
					else gravitate_pair(bodies[i], bodies[j]);
				}
		}

		void stop_unstop() { if (stoped) stoped = false; else stoped = true; }
		void reverse_velocities()
		{
			for (int i = 0; i < bodies.size(); i++)
			{
				bodies[i].velocity *= -1;
				bodies[i].W *= -1;
			}
		}
		void reverse_gravitation()
		{
			if (reversed_gravitation) reversed_gravitation = false;
			else reversed_gravitation = true;
		}
		void move_x(GLdouble dx)
		{
			for (int i = 0; i < bodies.size(); i++) bodies[i].pos.x += dx;
			down_left_area_corner.x += dx;
			up_right_area_corner.x += dx;
		}
		void move_y(GLdouble dy)
		{
			for (int i = 0; i < bodies.size(); i++) bodies[i].pos.y += dy;
			down_left_area_corner.y += dy;
			up_right_area_corner.y += dy;
		}
		void change_window_size_x(GLdouble dx) { up_right_area_corner.x += dx; }
		void change_window_size_y(GLdouble dy) { down_left_area_corner.y += dy; }
};

vector<System> systems;
int active_system_index = -1;

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	for (int i = 0; i < systems.size(); i++)
	{
		systems[i].draw();
		if (active_system_index == i)
		{
			Color inverted_system_background_color(255 - systems[i].color.R, 255 - systems[i].color.G, 255 - systems[i].color.B);
			drawLine(systems[i].down_left_area_corner, Vector2(systems[i].up_right_area_corner.x, systems[i].down_left_area_corner.y), inverted_system_background_color);
			drawLine(systems[i].down_left_area_corner, Vector2(systems[i].down_left_area_corner.x, systems[i].up_right_area_corner.y), inverted_system_background_color);
			drawLine(systems[i].up_right_area_corner, Vector2(systems[i].up_right_area_corner.x, systems[i].down_left_area_corner.y), inverted_system_background_color);
			drawLine(systems[i].up_right_area_corner, Vector2(systems[i].down_left_area_corner.x, systems[i].up_right_area_corner.y), inverted_system_background_color);
		}
	}
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
	if (w <= h)
	{
		windowWidth = scale;
		windowHeight = scale / aspectRatio;
		glOrtho(-scale, scale, -windowHeight, windowHeight, 1.0, -1.0);
	}
	else
	{
		windowWidth = scale * aspectRatio;
		windowHeight = scale;
		glOrtho(-windowWidth, windowWidth, -scale, scale, 1.0, -1.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void TimerFunction(int value)
{
	for (int i = 0; i < systems.size(); i++)
	{
		systems[i].solve_collisions();
		systems[i].gravitate();
		systems[i].move_bodies();
	}
	glutPostRedisplay();
	glutTimerFunc(dt, TimerFunction, 1);
}
