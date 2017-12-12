#define KEYS_ARRAY_SIZE 223

void (*keys[KEYS_ARRAY_SIZE])() = {};
void nothing() {}

void next_system() { active_system_index = (active_system_index + 1) % systems.size(); }
void previous_system()
{
	if (!active_system_index) active_system_index = systems.size() - 1;
	else --active_system_index;
}

void move_left_active() { systems[active_system_index].move_x(-1); }
void move_right_active() { systems[active_system_index].move_x(1); }
void move_down_active() { systems[active_system_index].move_y(-1); }
void move_up_active() { systems[active_system_index].move_y(1); }
void move_backward()
{
	if (!active_system_index) return;
	systems.insert(systems.begin(), systems[active_system_index]);
	systems.erase(systems.begin() + ++active_system_index);
	active_system_index = 0;
}
void move_forward()
{
	if (active_system_index == systems.size()-1) return;
	systems.push_back(systems[active_system_index]);
	systems.erase(systems.begin() + active_system_index);
	active_system_index = systems.size() - 1;
}

void stop_unstop_active() { systems[active_system_index].stop_unstop(); }
void reverse_velocities_active() { systems[active_system_index].reverse_velocities(); }
void reverse_gravitation_active() { systems[active_system_index].reverse_gravitation(); }

void setup_keys(char *file_name)
{
	if (file_name)
	{
		FILE *file = fopen(file_name, "rb");
		char function_name[32];
		int line_number = 0, key_code;
		while (!feof(file))
		{
			++line_number;
			fscanf(file, "%s", function_name);
			fscanf(file, "%d", &key_code);
			printf("%s %d\n", function_name, key_code);
			if (key_code < 0 || key_code > KEYS_ARRAY_SIZE)
			{
				printf("invalid key code %d on line %d\n", key_code, line_number);
				continue;
			}
			if (!strcmp(function_name, "next_system")) keys[key_code] = next_system;
			else if (!strcmp(function_name, "previous_system")) keys[key_code] = previous_system;
			else if (!strcmp(function_name, "stop_unstop_active")) keys[key_code] = stop_unstop_active;
			else if (!strcmp(function_name, "reverse_velocities_active")) keys[key_code] = reverse_velocities_active;
			else if (!strcmp(function_name, "reverse_gravitation_active")) keys[key_code] = reverse_gravitation_active;
			else if (!strcmp(function_name, "move_up_active")) keys[key_code] = move_up_active;
			else if (!strcmp(function_name, "move_down_acive")) keys[key_code] = move_down_active;
			else if (!strcmp(function_name, "move_left_active")) keys[key_code] = move_left_active;
			else if (!strcmp(function_name, "move_right_active")) keys[key_code] = move_right_active;
			else if (!strcmp(function_name, "move_forward")) keys[key_code] = move_forward;
			else if (!strcmp(function_name, "move_backward")) keys[key_code] = move_backward;
			else printf("unknown function name \"%s\" on line %d\n", function_name, line_number);
		}
	}
	else
	{
		keys[106] = next_system;
		keys[59] = previous_system;

		keys[32] = stop_unstop_active;

		keys[114] = reverse_velocities_active;
		keys[82] = reverse_velocities_active;
		keys[103] = reverse_gravitation_active;
		keys[81] = reverse_gravitation_active;

		keys[119] = move_up_active;
		keys[115] = move_down_active;
		keys[97] = move_left_active;
		keys[100] = move_right_active;
		keys[107] = move_forward;
		keys[108] = move_backward;
	}

	if (!keys[106]) printf("FOUND MISTAKE\n");
	void (**k)() = keys;
	for (; k < keys+223; k++) if (!*k) *k = nothing;
}

void key(unsigned char c, int x, int y) { keys[c%223]; }
