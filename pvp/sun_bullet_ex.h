#pragma once

#include "bullet.h"
#include "animation.h"

extern Atlas atlas_sun_ex;
extern Atlas atlas_sun_ex_explode;

extern Camera main_camera;

class SunBulletEx : public Bullet {
public:
	SunBulletEx() {
		size.x = 288, size.y = 288;
		damage = 20;

		animation_idle.set_atlas(&atlas_sun_ex);
		animation_idle.set_interval(50);

		animation_explode.set_atlas(&atlas_sun_ex_explode);
		animation_explode.set_interval(50);
		animation_explode.set_loop(false);
		animation_explode.set_callback([&]() {
			can_remove = true;
			});

		IMAGE* frame_idle = animation_idle.get_frame();
		IMAGE* frame_explode = animation_explode.get_frame();
		explode_offset.x = (frame_idle->getwidth() - frame_explode->getwidth()) / 2.0f;
		explode_offset.y = (frame_idle->getheight() - frame_explode->getheight()) / 2.0f;
	}

	~SunBulletEx() = default;

	void on_collide() {
		Bullet::on_collide();

		main_camera.shake(20, 350);

		mciSendString(_T("play sun_explode_ex from 0"), NULL, 0, NULL);
	}

	bool check_collision(const Vector2& target_position, const Vector2& target_size) override {
		// Use a larger collision box for the SunBulletEx
		bool is_collide_x = (max(position.x + size.x, target_position.x + target_size.x) - min(position.x, target_position.x) <= size.x + target_size.x);
		bool is_collide_y = (max(position.y + size.y, target_position.y + target_size.y) - min(position.y, target_position.y) <= size.y + target_size.y);

		return is_collide_x && is_collide_y;
	}

	void on_update(int delta) {
		if (valid) {
			position += velocity * (float)delta;
		}
		if (!valid) {
			animation_explode.on_update(delta);
		}
		else {
			animation_idle.on_update(delta);
		}
		if (check_if_out_of_bounds()) {
			can_remove = true;
		}
	}

	void on_draw(const Camera& camera) const {
		if (valid) {
			animation_idle.on_draw(camera, (int)position.x, (int)position.y);
		}
		else {
			animation_explode.on_draw(camera, (int)(position.x + explode_offset.x), (int)(position.y + explode_offset.y));
		}

		Bullet::on_draw(camera);
	}

private:
	const float gravity = 1e-3f;

private:
	Animation animation_idle;
	Animation animation_explode;
	Vector2 explode_offset;
};
