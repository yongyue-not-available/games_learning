#pragma once

#include "barb.h"
#include "sword.h"
#include "character.h"

class Enemy : public Character {
public:
	Enemy();
	~Enemy();

	void on_update(float delta) override;
	void on_render() override;

	void on_hurt() override;

	bool get_facing_left() const {
		return is_facing_left;
	}

	void set_facing_left(bool flag) {
		is_facing_left = flag;
	}

	bool get_dashing_in_air() const {
		return is_dashing_in_air;
	}

	void set_dashing_in_air(bool flag) {
		is_dashing_in_air = flag;
	}

	bool get_dashing_on_floor() const {
		return is_dashing_on_floor;
	}

	void set_dashing_on_floor(bool flag) {
		is_dashing_on_floor = flag;
	}

	bool get_throwing_silk() const {
		return is_throwing_silk;
	}

	void set_throwing_silk(bool flag) {
		is_throwing_silk = flag;
	}

	void throw_barbs();
	void throw_sword();

	void on_dash();
	void on_throw_silk();

private:
	bool is_dashing_in_air = false;
	bool is_dashing_on_floor = false;
	bool is_throwing_silk = false;

	Animation animation_silk;
	AnimationGroup animation_dash_in_air_vfx;
	AnimationGroup animation_dash_on_floor_vfx;
	Animation* curr_dash_animation = nullptr;

	std::vector<Barb*> barbs;
	std::vector<Sword*> swords;
	CollisionBox* collision_box_silk = nullptr;

};