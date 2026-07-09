#pragma once

#include "animation.h"
#include "collision_box.h"

class Barb {
public:
	Barb();
	~Barb();

	bool check_valid() const {
		return is_valid;
	}

	void set_position(const Vector2& position) {
		this->base_position = position;
		this->curr_position = position;
	}

	void on_update(float delta);
	void on_render();

private:
	enum class Stage {
		Idle,
		Aim,
		Dash,
		Break
	};

private:
	const float SPEED_DASH = 1500.0f;

private:
	Timer timer_idle;
	Timer timer_aim;
	int diff_period = 0;
	bool is_valid = true;
	float total_delta_time = 0;

	Vector2 base_position;
	Vector2 curr_position;
	Vector2 velocity;

	Animation animation_loose;
	Animation animation_break;
	Animation* curr_animation = nullptr;

	Stage stage = Stage::Idle;
	CollisionBox* collision_box = nullptr;

private:
	void on_break();
};