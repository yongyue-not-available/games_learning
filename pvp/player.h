#pragma once

#include "camera.h"
#include "vector.h"
#include "platform.h"
#include "particle.h"
#include "animation.h"
#include "player_id.h"

#include <graphics.h>

extern bool is_debug;

extern Atlas atlas_run_effect;
extern Atlas atlas_jump_effect;
extern Atlas atlas_land_effect;

extern std::vector<Bullet*> bullets;
extern std::vector<Platform> platforms;

class Player {
public:
	Player(bool facing_right = true): is_facing_right(facing_right) {
		current_animation = is_facing_right ? &animation_idle_right : &animation_idle_left;

		attack_cooldown_timer.set_wait_time(attack_cooldown);
		attack_cooldown_timer.set_one_shot(true);
		attack_cooldown_timer.set_callback([&]() {
			can_attack = true;
			});

		timer_invulnerable.set_wait_time(750);
		timer_invulnerable.set_one_shot(true);
		timer_invulnerable.set_callback([&]() {
			is_invulnerable = false;
			});

		timer_invulnerable_blink.set_wait_time(75);
		timer_invulnerable_blink.set_callback([&]() {
			is_showing_sketch_frame = !is_showing_sketch_frame;
			});

		timer_run_effect_generation.set_wait_time(75);
		timer_run_effect_generation.set_callback([&]() {
			Vector2 particle_position;
			IMAGE* frame = atlas_run_effect.get_image(0);
			particle_position.x = position.x + (size.x - frame->getwidth()) / 2;
			particle_position.y = position.y + size.y - frame->getheight();
			particles.emplace_back(particle_position, &atlas_run_effect, 45);
			});

		timer_die_effect_generation.set_wait_time(35);
		timer_die_effect_generation.set_callback([&]() {
			Vector2 particle_position;
			IMAGE* frame = atlas_run_effect.get_image(0);
			particle_position.x = position.x + (size.x - frame->getwidth()) / 2;
			particle_position.y = position.y + size.y - frame->getheight();
			particles.emplace_back(particle_position, &atlas_run_effect, 150);
			});

		animation_jump_effect.set_atlas(&atlas_jump_effect);
		animation_jump_effect.set_interval(25);
		animation_jump_effect.set_loop(false);
		animation_jump_effect.set_callback([&]() {
			is_jump_effect_affect = false;
			});

		animation_land_effect.set_atlas(&atlas_land_effect);
		animation_land_effect.set_interval(25);
		animation_land_effect.set_loop(false);
		animation_land_effect.set_callback([&]() {
			is_land_effect_affect = false;
			});
	}

	~Player() = default;

	virtual void on_attack() = 0;
	virtual void on_attack_ex() = 0;

	virtual void on_update(int delta) {
		int direction = is_right_key_down - is_left_key_down;

		if(direction != 0) {
			if (!is_attack_ex) {
				is_facing_right = direction > 0;
			}
			current_animation = is_facing_right ? &animation_run_right : &animation_run_left;
			float move_distance = speed * delta * direction;
			on_run(move_distance);
		}
		else {
			current_animation = is_facing_right ? &animation_idle_right : &animation_idle_left;
			timer_run_effect_generation.pause();
		}

		if (is_attack_ex) {
			current_animation = is_facing_right ? &animation_attack_ex_right : &animation_attack_ex_left;
		}

		if (hp <= 0) {
			current_animation = last_hurt_direction.x > 0 ? &animation_die_right : &animation_die_left;
		}

		current_animation->on_update(delta);
		animation_jump_effect.on_update(delta);
		animation_land_effect.on_update(delta);

		attack_cooldown_timer.on_update(delta);
		timer_invulnerable.on_update(delta);
		timer_invulnerable_blink.on_update(delta);
		timer_run_effect_generation.on_update(delta);

		if (hp <= 0) {
			timer_die_effect_generation.on_update(delta);
		}

		particles.erase(std::remove_if(
			particles.begin(), particles.end(),
			[](const Particle& particle) {
				return !particle.check_valid();
			}), 
			particles.end());
		for (Particle& particle : particles) {
			particle.on_update(delta);
		}

		if (is_showing_sketch_frame) {
			sketch_image(current_animation->get_frame(), &img_sketch);
		}

		move_and_collide(delta);
	}

	virtual void on_run(float move_distance) {
		if(is_attack_ex) {
			return;
		}

		position.x += move_distance;
		timer_run_effect_generation.resume();
	}

	virtual void on_jump() {
		if(velocity.y != 0 || is_attack_ex) {
			return;
		}
		velocity.y += jump_speed;
		is_jump_effect_affect = true;
		animation_jump_effect.reset();

		IMAGE* frame = animation_jump_effect.get_frame();
		position_jump_effect.x = position.x + (size.x - frame->getwidth()) / 2;
		position_jump_effect.y = position.y + size.y - frame->getheight();
	}

	virtual void on_land() {
		is_land_effect_affect = true;
		animation_land_effect.reset();

		IMAGE* frame = animation_land_effect.get_frame();
		position_land_effect.x = position.x + (size.x - frame->getwidth()) / 2;
		position_land_effect.y = position.y + size.y - frame->getheight();
	}

	virtual void on_draw(const Camera& camera) {
		if (is_jump_effect_affect) {
			animation_jump_effect.on_draw(camera, (int)position_jump_effect.x, (int)position_jump_effect.y);
		}
		if (is_land_effect_affect) {
			animation_land_effect.on_draw(camera, (int)position_land_effect.x, (int)position_land_effect.y);
		}

		for (const Particle& particle : particles) {
			particle.on_draw(camera);
		}

		if (hp > 0 && is_invulnerable && is_showing_sketch_frame) {
			putimage_alpha(camera, (int)position.x, (int)position.y, &img_sketch);
		}
		else {
			current_animation->on_draw(camera, (int)position.x, (int)position.y);
		}

		if (is_debug) {
			setlinecolor(RGB(0, 125, 255));
			rectangle((int)position.x, (int)position.y, (int)(position.x + size.x), (int)(position.y + size.y));
		}
	}

	virtual void on_input(const ExMessage& msg) {
		switch (msg.message) {
		case WM_KEYDOWN:
			switch (player_id) {
			case PlayerID::P1:
				switch (msg.vkcode) {
				case 0x41: // A
					is_left_key_down = true;
					break;
				case 0x44: // D
					is_right_key_down = true;
					break;
				case 0x57:
					on_jump();
					break;
				case 0x46:
					if (can_attack) {
						can_attack = false;
						attack_cooldown_timer.restart();
						on_attack();
					}
					break;
				case 0x47:
					if (mp >= 100) {
						on_attack_ex();
						mp = 0;
					}
					break;
				}
				break;
			case PlayerID::P2:
				switch (msg.vkcode) {
				case VK_LEFT: // A
					is_left_key_down = true;
					break;
				case VK_RIGHT: // D
					is_right_key_down = true;
					break;
				case VK_UP: // D
					on_jump();
					break;
				case VK_OEM_PERIOD:
					if (can_attack) {
						can_attack = false;
						attack_cooldown_timer.restart();
						on_attack();
					}
					break;
				case VK_OEM_2:
					if (mp >= 100) {
						on_attack_ex();
						mp = 0;
					}
					break;
				}
				break;
			}
			break;
		case WM_KEYUP:
			switch (player_id) {
			case PlayerID::P1:
				switch (msg.vkcode) {
				case 0x41: // A
					is_left_key_down = false;
					break;
				case 0x44: // D
					is_right_key_down = false;
					break;
				}
				break;
			case PlayerID::P2:
				switch (msg.vkcode) {
				case VK_LEFT: // A
					is_left_key_down = false;
					break;
				case VK_RIGHT: // D
					is_right_key_down = false;
					break;
				}
				break;
			}
			break;
		}
	}

	void make_invulnerable() {
		is_invulnerable = true;
		timer_invulnerable.restart();
	}

	void set_id(PlayerID id) {
		player_id = id;
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

	void set_hp(int val) {
		hp = val;
	}

	int get_hp() const {
		return hp;
	}

	int get_mp() const {
		return mp;
	}

protected:
	void move_and_collide(int delta) {
		float last_velocity_y = velocity.y;

		velocity.y += gravity * delta;
		position += velocity * (float)delta;

		if (hp <= 0) {
			return;
		}

		if(velocity.y > 0) {
			for (const Platform platform : platforms) {
				const Platform::CollisionShape shape = platform.shape;
				bool is_collide_x = (max(position.x + size.x, shape.right) - min(position.x, shape.left)
					<= size.x + (shape.right - shape.left));
				bool is_collide_y = (shape.y >= position.y && shape.y <= position.y + size.y);

				if(is_collide_x && is_collide_y) {
					float delta_pos_y = velocity.y * (float)delta;
					float last_time_pos_y = position.y + size.y - delta_pos_y;
					if(last_time_pos_y <= shape.y) {
						position.y = shape.y - size.y;
						velocity.y = 0;

						if (last_velocity_y != 0) {
							on_land();
						}

						break;
					}
				}
			}
		}

		if (!is_invulnerable) {
			for (Bullet* bullet : bullets) {
				if (!bullet->get_valid() || bullet->get_target() != player_id) {
					continue;
				}

				if (bullet->check_collision(position, size)) {
					make_invulnerable();
					bullet->on_collide();
					bullet->set_valid(false);
					hp -= bullet->get_damage();
					last_hurt_direction = bullet->get_position() - position;
					if (hp <= 0) {
						velocity.x = last_hurt_direction.x < 0 ? 0.35f : -0.35f;
						velocity.y = -1.0f;
					}
				}
			}
		}
	}

protected:
	const float gravity = 1.6e-3f;
	const float speed = 0.55f;
	const float jump_speed = -0.85f;

protected:
	int hp = 100;
	int mp = 0;

	Vector2 size;
	Vector2 position;
	Vector2 velocity;

	Animation animation_idle_left;
	Animation animation_idle_right;
	Animation animation_run_left;
	Animation animation_run_right;
	Animation animation_attack_ex_left;
	Animation animation_attack_ex_right;
	Animation animation_jump_effect;
	Animation animation_land_effect;
	Animation animation_die_left;
	Animation animation_die_right;

	bool is_jump_effect_affect = false;
	bool is_land_effect_affect = false;

	Vector2 position_jump_effect;
	Vector2 position_land_effect;

	Animation* current_animation = nullptr;

	PlayerID player_id = PlayerID::P1;

	bool is_left_key_down = false;
	bool is_right_key_down = false;

	bool is_facing_right = true;

	bool can_attack = true;
	int attack_cooldown = 500;
	Timer attack_cooldown_timer;

	bool is_attack_ex = false;

	IMAGE img_sketch;
	bool is_invulnerable = false;
	bool is_showing_sketch_frame = false;
	Timer timer_invulnerable;
	Timer timer_invulnerable_blink;

	std::vector<Particle> particles;

	Timer timer_run_effect_generation;
	Timer timer_die_effect_generation;

	Vector2 last_hurt_direction = { 0, 0 };
};
