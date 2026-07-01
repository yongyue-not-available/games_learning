#pragma once

#include "vector.h"
#include "camera.h"
#include "player_id.h"

#include <functional>
#include <graphics.h>

extern bool is_debug;

class Bullet {
public:
	Bullet() = default;
	virtual ~Bullet() = default;

	void set_damage(int dmg) {
		damage = dmg;
	}

	int get_damage() const {
		return damage;
	}

	void set_position(float x, float y) {
		position.x = x;
		position.y = y;
	}

	const Vector2& get_position() const {
		return position;
	}

	const Vector2& get_size() const {
		return size;
	}

	void set_velocity(float vx, float vy) {
		velocity.x = vx;
		velocity.y = vy;
	}

	void set_target(PlayerID id) {
		target_id = id;
	}

	PlayerID get_target() const {
		return target_id;
	}

	void set_callback(std::function<void()> cb) {
		callback = cb;
	}

	void set_valid(bool is_valid) {
		valid = is_valid;
	}

	bool get_valid() const {
		return valid;
	}

	bool is_can_remove() const {
		return can_remove;
	}

	virtual void on_collide() {
		if (callback) {
			callback();
		}
	}

	virtual bool check_collision(const Vector2& target_position, const Vector2& target_size) {
		return position.x + size.x / 2 >= target_position.x
			&& position.x + size.x / 2 <= target_position.x + target_size.x
			&& position.y + size.y / 2 >= target_position.y 
			&& position.y + size.y / 2 <= target_position.y + target_size.y;
	}

	virtual void on_update(int delta){}
	virtual void on_draw(const Camera& camera) const {
		if (is_debug) {
			setlinecolor(RGB(255, 255, 255));
			setfillcolor(RGB(255, 255, 255));
			rectangle((int)position.x, (int)position.y, (int)(position.x + size.x), (int)(position.y + size.y));
			solidcircle((int)(position.x + size.x / 2), (int)(position.y + size.y / 2), 5);
		}
	}

protected:
	Vector2 size;
	Vector2 position;
	Vector2 velocity;
	int damage = 10;
	bool valid = true;
	bool can_remove = false;
	std::function<void()> callback;
	PlayerID target_id = PlayerID::P1;

protected:
	bool check_if_out_of_bounds() {
		return position.x + size.x <= 0 || position.x >= getwidth() || position.y + size.y <= 0 || position.y > getheight();
	}
};