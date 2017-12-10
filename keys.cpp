int active_system_index = -1;

void (*keys[223])() = {};
void nothing() {}

void next_system() { active_system_index = (active_system_index + 1) % systems.size(); }
void previous_system()
{
	if (!active_system_index) active_system_index = systems.size() - 1;
	else --active_system_index;
}

void stop_unstop_active() { systems[active_system_index].stop_unstop(); }
void reverse_velocities_active() { systems[active_system_index].reverse_velocities(); }
void reverse_gravitation_active() { systems[active_system_index].reverse_gravitation(); }

void setup_keys()
{
	keys[106] = next_system;
	keys[59] = previous_system;
	keys[32] = stop_unstop_active;
	keys[114] = reverse_velocities_active;
	keys[82] = reverse_velocities_active;
	keys[103] = reverse_gravitation_active;
	keys[81] = reverse_gravitation_active;

	void (**k)() = keys;
	for (; k < keys+223; k++) if (!*k) *k = nothing;
}

void key(unsigned char c, int x, int y) { keys[c](); }

