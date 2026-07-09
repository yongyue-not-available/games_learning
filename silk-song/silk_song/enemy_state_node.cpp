#include "util.h"
#include "enemy.h"
#include "character_manager.h"
#include "enemy_state_node.h"

// Helper macros
#define GET_ENEMY()      ((Enemy*)CharacterManager::instance()->get_enemy())
#define GET_PLAYER_POS() (CharacterManager::instance()->get_player()->get_position())

// ============================================================
// EnemyAimState  ->  dead | dash_in_air
// ============================================================
EnemyAimState::EnemyAimState() {
	timer.set_wait_time(0.5f);
	timer.set_one_shot(true);
	timer.set_on_timeout([&]() {
		GET_ENEMY()->switch_state("dash_in_air");
		});
}

void EnemyAimState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("aim");
	// Hover in place: disable gravity and zero velocity
	enemy->set_enable_gravity(false);
	enemy->set_velocity({ 0, 0 });
	timer.restart();
}

void EnemyAimState::on_update(float delta) {
	timer.on_update(delta);
	Enemy* enemy = GET_ENEMY();
	if (enemy->get_hp() <= 0)
		enemy->switch_state("dead");
}

// ============================================================
// EnemyDashInAirState  ->  dead | idle
// ============================================================
EnemyDashInAirState::EnemyDashInAirState() {
	// Fallback: end dash after 0.6s even if player was not reached
	timer.set_wait_time(0.6f);
	timer.set_one_shot(true);
	timer.set_on_timeout([&]() {
		GET_ENEMY()->switch_state("idle");
		});
}

void EnemyDashInAirState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("dash_in_air");
	enemy->set_enable_gravity(false);
	enemy->set_dashing_in_air(true);

	// Target player foot position (position.y = foot)
	Vector2 player_pos = GET_PLAYER_POS();
	Vector2 to_target = {
		player_pos.x - enemy->get_position().x,
		player_pos.y - enemy->get_position().y
	};
	float len = std::sqrt(to_target.x * to_target.x + to_target.y * to_target.y);
	if (len > 0) {
		to_target.x /= len;
		to_target.y /= len;
	}
	enemy->set_facing_left(to_target.x < 0);
	enemy->set_velocity({ to_target.x * SPEED_DASH, to_target.y * SPEED_DASH });

	// Initialize curr_dash_animation via on_dash()
	enemy->on_dash();

	timer.restart();
	play_audio(_T("enemy_dash"), false);
}

void EnemyDashInAirState::on_update(float delta) {
	timer.on_update(delta);

	Enemy* enemy = GET_ENEMY();

	if (enemy->get_hp() <= 0) {
		enemy->switch_state("dead");
		return;
	}

	Vector2 pos = enemy->get_position();
	Vector2 player_pos = GET_PLAYER_POS();
	float dx = pos.x - player_pos.x;
	float dy = pos.y - player_pos.y;
	float dist = std::sqrt(dx * dx + dy * dy);

	// End dash when: close to player, OR near any map boundary
	// (character.cpp clamps pos to [0, w] and [0, floor_y], so check near-boundary)
	bool near_x_edge = (pos.x <= 5.0f || pos.x >= getwidth() - 5.0f);
	bool near_floor   = (pos.y >= enemy->get_floor_y() - 5.0f);

	if (dist < 100.0f || near_x_edge || near_floor) {
		enemy->switch_state("idle");
	}
}

void EnemyDashInAirState::on_exit() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_enable_gravity(true);
	enemy->set_dashing_in_air(false);
	enemy->set_velocity({ 0, 0 });
}


// ============================================================
// EnemyDashOnFloorState  ->  dead | idle
// ============================================================
EnemyDashOnFloorState::EnemyDashOnFloorState() {
	timer.set_wait_time(0.4f);
	timer.set_one_shot(true);
	timer.set_on_timeout([&]() {
		Enemy* enemy = GET_ENEMY();
		enemy->set_dashing_on_floor(false);
		enemy->switch_state("idle");
		});
}

void EnemyDashOnFloorState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("dash_on_floor");
	enemy->set_dashing_on_floor(true);

	// Face player; direction determines which VFX animation on_dash() selects
	float dir = enemy->get_facing_left() ? -1.0f : 1.0f;
	enemy->set_velocity({ dir * SPEED_DASH, 0 });

	// Initialize curr_dash_animation via on_dash()
	enemy->on_dash();

	timer.restart();
	play_audio(_T("enemy_dash"), false);
}

void EnemyDashOnFloorState::on_update(float delta) {
	timer.on_update(delta);

	Enemy* enemy = GET_ENEMY();
	if (enemy->get_hp() <= 0)
		enemy->switch_state("dead");
}

// ============================================================
// EnemyDeadState
// ============================================================
void EnemyDeadState::on_enter() {
	MessageBox(GetHWnd(), _T("you win"), _T("try again?"), MB_OK);
	exit(0);
}

// ============================================================
// EnemyFallState  ->  dead | idle
// ============================================================
void EnemyFallState::on_enter() {
	GET_ENEMY()->set_animation("fall");
}

void EnemyFallState::on_update(float delta) {
	Enemy* enemy = GET_ENEMY();

	if (enemy->get_hp() <= 0)
		enemy->switch_state("dead");
	else if (enemy->is_on_floor())
		enemy->switch_state("idle");
}

// ============================================================
// EnemyIdleState  ->  dead | fall | run | squat | jump |
//                     throw_sword | throw_silk | throw_barb
// ============================================================
EnemyIdleState::EnemyIdleState() {
	timer.set_one_shot(true);
	timer.set_on_timeout([&]() {
		Enemy* enemy = GET_ENEMY();
		if (enemy->get_hp() <= 0) return;

		Vector2 enemy_pos = enemy->get_position();
		Vector2 player_pos = GET_PLAYER_POS();
		float dx = player_pos.x - enemy_pos.x;
		float dist = std::abs(dx);
		int hp = enemy->get_hp();
		bool is_low_hp = (hp < 5);

		// Face player before deciding next action
		enemy->set_facing_left(dx < 0);

		if (is_low_hp) {
			// Low HP: prioritize ranged/AoE damage; throw_barb available
			int roll = range_random(1, 10);
			if (dist > 400.0f) {
				if (roll <= 3)       enemy->switch_state("throw_sword");
				else if (roll <= 5)  enemy->switch_state("throw_silk");
				else if (roll <= 7)  enemy->switch_state("jump");
				else if (roll <= 9)  enemy->switch_state("throw_barb");
				else                 enemy->switch_state("run");
			} else if (dist > 150.0f) {
				if (roll <= 3)       enemy->switch_state("throw_silk");
				else if (roll <= 5)  enemy->switch_state("throw_sword");
				else if (roll <= 7)  enemy->switch_state("throw_barb");
				else if (roll <= 9)  enemy->switch_state("jump");
				else                 enemy->switch_state("squat");
			} else {
				if (roll <= 4)       enemy->switch_state("throw_silk");
				else if (roll <= 7)  enemy->switch_state("throw_barb");
				else                 enemy->switch_state("squat");
			}
		} else {
			// Normal HP: balanced strategy, no throw_barb
			int roll = range_random(1, 6);
			if (dist > 600.0f) {
				if (roll <= 2)       enemy->switch_state("run");
				else if (roll <= 4)  enemy->switch_state("jump");
				else                 enemy->switch_state("throw_sword");
			} else if (dist > 200.0f) {
				if (roll == 1)       enemy->switch_state("throw_silk");
				else if (roll == 2)  enemy->switch_state("throw_sword");
				else if (roll == 3)  enemy->switch_state("squat");
				else if (roll == 4)  enemy->switch_state("run");
				else                 enemy->switch_state("jump");
			} else {
				if (roll <= 3)       enemy->switch_state("squat");
				else                 enemy->switch_state("throw_silk");
			}
		}
		});
}

void EnemyIdleState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("idle");
	enemy->set_velocity({ 0, 0 });
	// Random wait time based on HP: shorter when low HP
	int hp = enemy->get_hp();
	float wait;
	if (hp < 5)
		wait = 0.5f + (float)range_random(0, 5) * 0.1f;  // 0.5s ~ 1.0s
	else
		wait = 1.0f + (float)range_random(0, 6) * 0.1f;  // 1.0s ~ 1.6s
	timer.set_wait_time(wait);
	timer.restart();
}

void EnemyIdleState::on_update(float delta) {
	timer.on_update(delta);

	Enemy* enemy = GET_ENEMY();

	if (enemy->get_hp() <= 0)
		enemy->switch_state("dead");
	else if (!enemy->is_on_floor() && enemy->get_velocity().y > 0)
		enemy->switch_state("fall");
}

void EnemyIdleState::on_exit() {
	// Correct facing direction toward player when leaving idle
	Enemy* enemy = GET_ENEMY();
	float dx = GET_PLAYER_POS().x - enemy->get_position().x;
	enemy->set_facing_left(dx < 0);
}

// ============================================================
// EnemyJumpState  ->  dead | aim | fall | throw_silk
// ============================================================
void EnemyJumpState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("jump");

	Vector2 player_pos = GET_PLAYER_POS();
	float dx = player_pos.x - enemy->get_position().x;
	enemy->set_facing_left(dx < 0);

	enemy->set_velocity({ 0, -SPEED_JUMP });
}

void EnemyJumpState::on_update(float delta) {
	Enemy* enemy = GET_ENEMY();

	if (enemy->get_hp() <= 0) {
		enemy->switch_state("dead");
		return;
	}

	bool is_low_hp = (enemy->get_hp() < 5);

	if (enemy->get_velocity().y < 0) {
		// Ascending: chance to aim-dash
		if (is_low_hp) {
			if (range_random(1, 2) == 1)
				enemy->switch_state("aim");
		} else {
			if (range_random(1, 4) == 1)
				enemy->switch_state("aim");
		}
	} else if (enemy->get_velocity().y > 0) {
		// Descending: chance to throw silk
		if (is_low_hp) {
			int roll = range_random(1, 3);
			if (roll <= 2)  enemy->switch_state("throw_silk");
			else            enemy->switch_state("fall");
		} else {
			if (range_random(1, 3) == 1)
				enemy->switch_state("throw_silk");
			else
				enemy->switch_state("fall");
		}
	}
}

// ============================================================
// EnemyRunState  ->  dead | squat | throw_silk | idle
// ============================================================
void EnemyRunState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("run");
	play_audio(_T("enemy_run"), true);
}

void EnemyRunState::on_update(float delta) {
	Enemy* enemy = GET_ENEMY();

	if (enemy->get_hp() <= 0) {
		enemy->switch_state("dead");
		return;
	}

	Vector2 enemy_pos = enemy->get_position();
	Vector2 player_pos = GET_PLAYER_POS();
	float dx = player_pos.x - enemy_pos.x;
	float dist = std::abs(dx);

	enemy->set_facing_left(dx < 0);
	float dir = dx < 0 ? -1.0f : 1.0f;
	enemy->set_velocity({ dir * SPEED_RUN, enemy->get_velocity().y });

	if (dist <= MIN_DIS) {
		bool is_low_hp = (enemy->get_hp() < 5);
		if (is_low_hp) {
			int roll = range_random(1, 4);
			if (roll <= 2)       enemy->switch_state("throw_silk");
			else if (roll == 3)  enemy->switch_state("squat");
			else                 enemy->switch_state("throw_barb");
		} else {
			int roll = range_random(1, 3);
			if (roll == 1)       enemy->switch_state("squat");
			else if (roll == 2)  enemy->switch_state("throw_silk");
			else                 enemy->switch_state("idle");
		}
	}
}

void EnemyRunState::on_exit() {
	GET_ENEMY()->set_velocity({ 0, GET_ENEMY()->get_velocity().y });
	stop_audio(_T("enemy_run"));
}

// ============================================================
// EnemySquatState  ->  dead | dash_on_floor
// ============================================================
EnemySquatState::EnemySquatState() {
	timer.set_wait_time(0.5f);
	timer.set_one_shot(true);
	timer.set_on_timeout([&]() {
		GET_ENEMY()->switch_state("dash_on_floor");
		});
}

void EnemySquatState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("squat");
	// Face player before squatting so the dash goes the right way
	float dx = GET_PLAYER_POS().x - enemy->get_position().x;
	enemy->set_facing_left(dx < 0);
	timer.restart();
}

void EnemySquatState::on_update(float delta) {
	timer.on_update(delta);

	Enemy* enemy = GET_ENEMY();
	if (enemy->get_hp() <= 0)
		enemy->switch_state("dead");
}

// ============================================================
// EnemyThrowBarbState  ->  dead | idle
// ============================================================
EnemyThrowBarbState::EnemyThrowBarbState() {
	timer.set_wait_time(0.8f);
	timer.set_one_shot(true);
	timer.set_on_timeout([&]() {
		GET_ENEMY()->throw_barbs();
		GET_ENEMY()->switch_state("idle");
		});
}

void EnemyThrowBarbState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("throw_barb");
	play_audio(_T("enemy_throw_barbs"), false);
	timer.restart();
}

void EnemyThrowBarbState::on_update(float delta) {
	timer.on_update(delta);

	Enemy* enemy = GET_ENEMY();
	if (enemy->get_hp() <= 0)
		enemy->switch_state("dead");
}

// ============================================================
// EnemyThrowSilkState  ->  dead | aim | fall | idle
// ============================================================
EnemyThrowSilkState::EnemyThrowSilkState() {
	timer.set_wait_time(0.9f);
	timer.set_one_shot(true);
	timer.set_on_timeout([&]() {
		Enemy* enemy = GET_ENEMY();
		enemy->set_throwing_silk(false);
		enemy->set_enable_gravity(true);
		bool is_low_hp = (enemy->get_hp() < 5);
		if (is_low_hp) {
			int roll = range_random(1, 4);
			if (roll <= 2)       enemy->switch_state("aim");
			else if (roll == 3)  enemy->switch_state("throw_sword");
			else                 enemy->switch_state("idle");
		} else {
			int roll = range_random(1, 3);
			if (roll == 1)       enemy->switch_state("aim");
			else if (roll == 2)  enemy->switch_state("fall");
			else                 enemy->switch_state("idle");
		}
		});
}

void EnemyThrowSilkState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("throw_silk");
	enemy->set_throwing_silk(true);
	// Hover while throwing: disable gravity and zero velocity
	enemy->set_enable_gravity(false);
	enemy->set_velocity({ 0, 0 });

	float dx = GET_PLAYER_POS().x - enemy->get_position().x;
	enemy->set_facing_left(dx < 0);

	enemy->on_throw_silk();
	play_audio(_T("enemy_throw_silk"), false);
	timer.restart();
}

void EnemyThrowSilkState::on_update(float delta) {
	timer.on_update(delta);

	Enemy* enemy = GET_ENEMY();
	if (enemy->get_hp() <= 0) {
		enemy->set_throwing_silk(false);
		enemy->set_enable_gravity(true);
		enemy->switch_state("dead");
	}
}

// ============================================================
// EnemyThrowSwordState  ->  dead | squat | jump | throw_silk | idle
// ============================================================
EnemyThrowSwordState::EnemyThrowSwordState() {
	// timer_throw: fires the actual sword throw
	timer_throw.set_wait_time(0.65f);
	timer_throw.set_one_shot(true);
	timer_throw.set_on_timeout([&]() {
		GET_ENEMY()->throw_sword();
		play_audio(_T("enemy_throw_sword"), false);
		});

	// timer_switch: switches state after animation ends, based on HP
	timer_switch.set_wait_time(1.0f);
	timer_switch.set_one_shot(true);
	timer_switch.set_on_timeout([&]() {
		Enemy* enemy = GET_ENEMY();
		bool is_low_hp = (enemy->get_hp() < 5);
		if (is_low_hp) {
			int roll = range_random(1, 4);
			if (roll == 1)       enemy->switch_state("throw_silk");
			else if (roll == 2)  enemy->switch_state("squat");
			else if (roll == 3)  enemy->switch_state("jump");
			else                 enemy->switch_state("throw_barb");
		} else {
			int roll = range_random(1, 4);
			if (roll == 1)       enemy->switch_state("squat");
			else if (roll == 2)  enemy->switch_state("jump");
			else if (roll == 3)  enemy->switch_state("throw_silk");
			else                 enemy->switch_state("idle");
		}
		});
}

void EnemyThrowSwordState::on_enter() {
	Enemy* enemy = GET_ENEMY();
	enemy->set_animation("throw_sword");

	float dx = GET_PLAYER_POS().x - enemy->get_position().x;
	enemy->set_facing_left(dx < 0);

	timer_throw.restart();
	timer_switch.restart();
}

void EnemyThrowSwordState::on_update(float delta) {
	timer_throw.on_update(delta);
	timer_switch.on_update(delta);

	Enemy* enemy = GET_ENEMY();
	if (enemy->get_hp() <= 0)
		enemy->switch_state("dead");
}
