#pragma once

#include "timer.h"
#include "vector.h"

class Camera {
public:
	float x = 0.0;
	float y = 0.0;
public:
	Camera() {
		shake_timer.set_one_shot(true);
		shake_timer.set_callback([&]() {
			is_shaking = false;
			reset();
			});
	}
	~Camera() = default;

	void shake(float strength, int duration) {
		is_shaking = true;
		shaking_strength = strength;

		shake_timer.set_wait_time(duration);
		shake_timer.restart();
	}

	const Vector2& get_position() const {
		return position;
	}

	void reset() {
		position.x = 0;
		position.y = 0;
	}

	void on_update(int delta) {
		shake_timer.on_update(delta);

		if (is_shaking) {
			position.x = (-50 + rand() % 100) / 50.0f * shaking_strength;
			position.y = (-50 + rand() % 100) / 50.0f * shaking_strength;
		}
	}

	void on_enter() {

	}

private:
	Vector2 position;
	Timer shake_timer;
	bool is_shaking = false;
	float shaking_strength = 0;
};