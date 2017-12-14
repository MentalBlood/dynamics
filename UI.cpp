GLdouble get_double(GLdouble& min, GLdouble& max)
{
	char s[32];
	GLdouble a;
	while (1)
	{
		scanf("%32s", s);
		char *c = s;
		for (; *c; c++)
			if (*c != '0' && *c != '1' && *c != '2' && *c != '3' && *c != '4' && *c != '5' && *c != '6' && *c != '7' && *c != '8' && *c != '9' && 
				*c != '.' && *c != '-')
			{
				printf("Enter valid floating point number: ");
				break;
			}
		if (!*c)
		{
			a = GLdouble(strtod(s, NULL));
			if (a <= max && a >= min) return a;
			printf("This number is out of range [%lf, %lf], enter a valid one: ", min, max);
		}
	}
	return GLdouble(strtod(s, NULL));
}

int get_int(int min, int max)
{
	char s[16];
	int a = 0;
	while (1)
	{
		scanf("%16s", s);
		char *c = s;
		for (; *c; c++)
		{
			a *= 10;
			if (*c == '0') continue;
			switch (*c)
			{
				case '1':
					++a;
					break;
				case '2':
					a += 2;
					break;
				case '3':
					a += 3;
					break;
				case '4':
					a += 4;
					break;
				case'5':
					a += 5;
					break;
				case '6':
					a += 6;
					break;
				case '7':
					a += 7;
					break;
				case '8':
					a += 8;
					break;
				case '9':
					a += 9;
					break;
				case 0:
					return a;
				default:
					printf("Enter a valid integer number: ");
			}
			if (a < min || a > max)
			{
				printf("This number is out of range [%d, %d], enter a valid one: ", min, max);
				break;
			}
		}
	}
}

GLdouble input(const char *question, GLdouble default_answer, GLdouble input_for_default_answer, GLdouble min, GLdouble max)
{
	GLdouble answer;
	printf("%s", question); answer = get_double(min, max);
	if (answer == input_for_default_answer) return default_answer;
	return answer;
}

void setup_new_system()
{
	printf("Input parametres (print -1 for default):\n");
	GLdouble bodies_number, min_size, max_size, min_m, max_m, min_v, max_v, min_vertexes_number, max_vertexes_number;
	bodies_number = input("Bodies number (default is 70): ", 50, -1, 0, 1000);
	min_size = input("Minimum size (default is 3): ", 3, -1, 1, scale/2);
	max_size = input("Maximum size (default is 8): ", 8, -1, min_size, scale/2);
	min_m = input("Minimum mass (default is 20): ", 20, -1, 0.1, 1000);
	max_m = input("Maximum mass (default is 20): ", 20, -1, min_m, 1000);
	min_v = input("Minimum velocity (default is 0.05): ", 0.05, -1, 0, scale/100);
	max_v = input("Maximum velocity (default is 0.11): ", 0.11, -1, min_v, scale/100);
	min_vertexes_number = input("Minimum vertexes number (default is 4): ", 3, -1, 3, 30);
	max_vertexes_number = input("Maximum vertexes number (default is 15): ", 15, -1, min_vertexes_number, 30);
	
	printf("Set rectagular area where system may act:\n");
	Vector2 down_left_area_corner, up_right_area_corner;
	down_left_area_corner.x = input("Up left area corner x (default is -100): ", -100, -1, -3*scale, scale*3);
	down_left_area_corner.y = input("Up left area corner y (default is -100): ", -100, -1, -scale, scale-20);
	up_right_area_corner.x = input("Down right area corner x (default is 100): ", 100, -1, down_left_area_corner.x+10, scale*3);
	up_right_area_corner.y = input("Down right area corner y (default is 100): ", 100, -1, down_left_area_corner.y+10, scale);

	printf("Set background (area) color by three numbers from 0.0 to 1.0:\n");
	Color3f background_color;
	background_color.R = input("Red (default is 0.1): ", 0.1, -1, 0, 1);
	background_color.G = input("Green (default is 0.1): ", 0.1, -1, 0, 1);
	background_color.B = input("Blue (default is 0.1): ", 0.1, -1, 0, 1);

	char system_name[128];
	printf("Give a name for this system (maximum 127 symbols): "); scanf("%128s", system_name);

	systems.push_back( System(	bodies_number, min_size, max_size, min_m, max_m, min_v, max_v, min_vertexes_number, 
								max_vertexes_number, down_left_area_corner, up_right_area_corner, background_color, system_name));
}

void UI()
{
	char c;
	int choice;
	char demo_name[] = "Demo";
UI_BEGIN:
	printf("1 - Default configuration\n2 - Custom configuration\n");
	while (1)
	{
		scanf("%c", &c);
		switch (c)
		{
			case '1':
				systems.push_back( System(70, 3, 8, 20, 20, 0.05, 0.11, 4, 15, Vector2(-scale, -scale), Vector2(scale, scale), Color3f(0.1, 0.1, 0.1), demo_name) );
				return;
			case '2':
				setup_new_system();
				printf("1 - Start\n2 - Add one more\n3 - Remove");
				while (1)
				{
					scanf("%c", &c);
					switch (c)
					{
						case '1':
							return;
						case '2':
							setup_new_system();
							break;
						case '3':
							for (int i = 0; i < systems.size(); i++)
								printf("%d - system \"%s\" with %d bodies\n", i+1, systems[i].name, systems[i].bodies.size());
							printf("0 - Remove all\n");
							choice = get_int(0, systems.size());
							if (!choice)
								while (!systems.empty()) systems.pop_back();
							else systems.erase(systems.begin() + --choice);
							goto UI_BEGIN;
							break;
						default:
							printf("Wrong input\n");
					}
				}
			default:
				printf("Wrong input\n");
		}
	}
}
