#pragma once

#include "player.h"
#include "pea_bullet.h"

#include <iostream>

extern Atlas atlas_peashooter_idle_left;
extern Atlas atlas_peashooter_idle_right;
extern Atlas atlas_peashooter_run_left;
extern Atlas atlas_peashooter_run_right;
extern Atlas atlas_peashooter_attack_ex_left;
extern Atlas atlas_peashooter_attack_ex_right;
extern Atlas atlas_peashooter_die_left;
extern Atlas atlas_peashooter_die_right;

class PeashooterPlayer: public Player {
public:
	PeashooterPlayer(bool facing_right = true): Player(facing_right) {
		animation_idle_left.set_atlas(&atlas_peashooter_idle_left);
		animation_idle_right.set_atlas(&atlas_peashooter_idle_right);
		animation_run_left.set_atlas(&atlas_peashooter_run_left);
		animation_run_right.set_atlas(&atlas_peashooter_run_right);
		animation_attack_ex_left.set_atlas(&atlas_peashooter_attack_ex_left);
		animation_attack_ex_right.set_atlas(&atlas_peashooter_attack_ex_right);
		animation_die_left.set_atlas(&atlas_peashooter_die_left);
		animation_die_right.set_atlas(&atlas_peashooter_die_right);

		animation_idle_left.set_interval(75);
		animation_idle_right.set_interval(75);
		animation_run_left.set_interval(75);
		animation_run_right.set_interval(75);
		animation_attack_ex_left.set_interval(75);
		animation_attack_ex_right.set_interval(75);
		animation_die_left.set_interval(150);
		animation_die_right.set_interval(150);

		animation_die_left.set_loop(false);
		animation_die_right.set_loop(false);

		size.x = 96, size.y = 96;

		attack_ex_timer.set_wait_time(attack_ex_duration);
		attack_ex_timer.set_one_shot(true);
		attack_ex_timer.set_callback([&]() {
			is_attack_ex = false;
			});

		spawn_pea_ex_timer.set_wait_time(100);
		spawn_pea_ex_timer.set_callback([&]() {
			spawn_pea_bullet(ex_bullet_speed);
			});

		attack_cooldown = 100;
	}

	~PeashooterPlayer() = default;

	void on_update(int delta) {
		Player::on_update(delta);
		
		if(is_attack_ex) {
			main_camera.shake(5, 100);
			attack_ex_timer.on_update(delta);
			spawn_pea_ex_timer.on_update(delta);
		}
	}

	void on_attack() override {
		// Implement the attack logic for PeashooterPlayer
		spawn_pea_bullet(bullet_speed);

		switch (rand() % 2) {
		case 0:
			mciSendString(_T("play pea_shoot_1 from 0"), NULL, 0, NULL);
			break;
		case 1:
			mciSendString(_T("play pea_shoot_2 from 0"), NULL, 0, NULL);
			break;
		}
	}

	void on_attack_ex() override {
		// Implement the special attack logic for PeashooterPlayer
		is_attack_ex = true;
		attack_ex_timer.restart();

		is_facing_right ? animation_attack_ex_right.reset() : animation_attack_ex_left.reset();

		mciSendString(_T("play pea_shoot_ex from 0"), NULL, 0, NULL);
	}

private:
	const float bullet_speed = 0.75f;
	const float ex_bullet_speed = 1.5f;
	const int attack_ex_duration = 2500;

	Timer attack_ex_timer;
	Timer spawn_pea_ex_timer;

private:
	void spawn_pea_bullet(float speed) {
		// Implement the logic to spawn a pea bullet
		Bullet* bullet = new PeaBullet();

		Vector2 bullet_position, bullet_velocity;
		const Vector2& bullet_size = bullet->get_size();
		bullet_position.x = is_facing_right ? position.x + size.x - bullet_size.x / 2 : position.x - bullet_size.x;
		bullet_position.y = position.y;
		bullet_velocity.x = is_facing_right ? speed : -speed;
		bullet_velocity.y = 0;

		bullet->set_position(bullet_position.x, bullet_position.y);
		bullet->set_velocity(bullet_velocity.x, bullet_velocity.y);

		bullet->set_target(player_id == PlayerID::P1 ? PlayerID::P2 : PlayerID::P1);

		bullet->set_callback([&]() {
			mp += 25;
			});

		bullets.push_back(bullet);
	}
};