#pragma once

#include "vector2.h"
#include "animation.h"
#include "collision_box.h"
#include "state_machine.h"

#include <string>
#include <graphics.h>
#include <unordered_map>

class Character {
public:
	Character();
	~Character();

	void decrease_hp();

	int get_hp() const {
		return hp;
	}

	void set_position(const Vector2& position) {
		this->position = position;
	}

	const Vector2& get_position() const {
		return position;
	}

	void set_velocity(const Vector2& velocity) {
		this->velocity = velocity;
	}

	const Vector2& get_velocity() const {
		return velocity;
	}

	Vector2 get_logic_height() {
		return Vector2(position.x, position.y - logic_height / 2);
	}

	void set_enable_gravity(bool flag) {
		enable_gravity = flag;
	}

	CollisionBox* get_hit_box() {
		return hit_box;
	}

	CollisionBox* get_hurt_box() {
		return hurt_box;
	}

	bool is_on_floor() const {
		return position.y >= FLOOR_Y;
	}

	float get_floor_y() const {
		return FLOOR_Y;
	}

	void make_invulnerable() {
		is_invulnerable = true;
		timer_invulnerable.restart();
	}

	virtual void on_input(const ExMessage& msg);
	virtual void on_update(float delta);
	virtual void on_render();

	virtual void on_hurt();

	void switch_state(const std::string& id);
	void set_animation(const std::string& id);

protected:
	struct AnimationGroup
	{
		Animation left;
		Animation right;
	};

protected:
	const float FLOOR_Y = 620;
	const float GRAVITY = 980 * 2;

protected:
	int hp = 10;
	Vector2 position;
	Vector2 velocity;
	float logic_height = 0;
	bool is_facing_left = true;
	StateMachine state_machine;
	bool enable_gravity = true;
	bool is_invulnerable = false;
	bool is_blink_invisible = false;
	Timer timer_invulnerable;
	Timer timer_invulnerable_blink;
	CollisionBox* hit_box = nullptr;
	CollisionBox* hurt_box = nullptr;
	AnimationGroup* curr_animation = nullptr;
	std::unordered_map<std::string, AnimationGroup> animation_pool;
};