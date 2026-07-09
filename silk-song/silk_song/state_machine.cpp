#include "state_machine.h"

StateMachine::StateMachine() = default;
StateMachine::~StateMachine() = default;

void StateMachine::on_update(float delta) {
	if (!curr_state) {
		return;
	}

	if (need_init) {
		curr_state->on_enter();
		need_init = false;
	}

	curr_state->on_update(delta);
}

void StateMachine::set_entry(const std::string& id) {
	curr_state = state_pool[id];
}

void StateMachine::switch_to(const std::string& id) {
	if (curr_state) {
		curr_state->on_exit();
	}
	curr_state = state_pool[id];
	if (curr_state) {
		curr_state->on_enter();
	}
}

void StateMachine::register_state(const std::string& id, StateNode* state_node) {
	state_pool[id] = state_node;
}