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
	Vector2 up_left_area_corner, down_right_area_corner;
	up_left_area_corner.x = input("Up left area corner x (default is -100): ", -100, -1, -scale, scale*3);
	up_left_area_corner.y = input("Up left area corner y (default is -100): ", -100, -1, -scale, scale-1);
	down_right_area_corner.x = input("Down right area corner x (default is 100): ", 100, -1, up_left_area_corner.x+1, scale*3);
	down_right_area_corner.y = input("Down right area corner y (default is 100): ", 100, -1, up_left_area_corner.y+1, scale-1);

	printf("Set background (area) color by three numbers from 0.0 to 1.0:\n");
	Color3f background_color;
	background_color.R = input("Red (default is 0.1): ", 0.1, -1, 0, 1);
	background_color.G = input("Green (default is 0.1): ", 0.1, -1, 0, 1);
	background_color.B = input("Blue (default is 0.1): ", 0.1, -1, 0, 1);

	systems.push_back( System(	bodies_number, min_size, max_size, min_m, max_m, min_v, max_v, min_vertexes_number, 
								max_vertexes_number, up_left_area_corner, down_right_area_corner, background_color) );
}

void UI()
{
	char c;
	printf("1 - Default configuration\n2 - Custom configuration\n");
	while (1)
	{
		c = getc(stdin);
		switch (c)
		{
			case '1':
				systems.push_back( System(70, 3, 8, 20, 20, 0.05, 0.11, 4, 15, Vector2(-scale, -scale), Vector2(scale, scale), Color3f(0.1, 0.1, 0.1)) );
				return;
				break;
			case '2':
				setup_new_system();
				return;
			default:
				printf("Wrong input\n");
		}
	}
}
