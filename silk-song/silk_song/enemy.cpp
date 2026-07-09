#include "enemy.h"
#include "resources_manager.h"
#include "collision_manager.h"
#include "character_manager.h"
#include "enemy_state_node.h"

Enemy::Enemy() {
	is_facing_left = true;
	position = { 1050, 200 };
	logic_height = 150;

	hit_box->set_size({ 50, 80 });
	hurt_box->set_size({ 100, 180 });

	hit_box->set_layer_src(CollisionLayer::None);
	hit_box->set_layer_dst(CollisionLayer::Player);

	hurt_box->set_layer_src(CollisionLayer::Enemy);
	hurt_box->set_layer_dst(CollisionLayer::None);

	hurt_box->set_on_collide([&]() {
		decrease_hp();
		});

	collision_box_silk = CollisionManager::instance()->create_collision_box();
	collision_box_silk->set_layer_src(CollisionLayer::None);
	collision_box_silk->set_layer_dst(CollisionLayer::Player);
	collision_box_silk->set_size({ 225,225 });
	collision_box_silk->set_enable(false);

	// === 角色动画 ===
	{
		{
			AnimationGroup& animation_aim = animation_pool["aim"];

			Animation& animation_aim_left = animation_aim.left;
			animation_aim_left.set_interval(0.05f);
			animation_aim_left.set_loop(false);
			animation_aim_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_aim_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_aim_left"));

			Animation& animation_aim_right = animation_aim.right;
			animation_aim_right.set_interval(0.05f);
			animation_aim_right.set_loop(false);
			animation_aim_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_aim_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_aim_right"));
		}
		{
			AnimationGroup& animation_dash_in_air = animation_pool["dash_in_air"];

			Animation& animation_dash_in_air_left = animation_dash_in_air.left;
			animation_dash_in_air_left.set_interval(0.05f);
			animation_dash_in_air_left.set_loop(true);
			animation_dash_in_air_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_dash_in_air_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_dash_in_air_left"));

			Animation& animation_dash_in_air_right = animation_dash_in_air.right;
			animation_dash_in_air_right.set_interval(0.05f);
			animation_dash_in_air_right.set_loop(true);
			animation_dash_in_air_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_dash_in_air_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_dash_in_air_right"));
		}
		{
			AnimationGroup& animation_dash_on_floor = animation_pool["dash_on_floor"];

			Animation& animation_dash_on_floor_left = animation_dash_on_floor.left;
			animation_dash_on_floor_left.set_interval(0.05f);
			animation_dash_on_floor_left.set_loop(true);
			animation_dash_on_floor_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_dash_on_floor_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_dash_on_floor_left"));

			Animation& animation_dash_on_floor_right = animation_dash_on_floor.right;
			animation_dash_on_floor_right.set_interval(0.05f);
			animation_dash_on_floor_right.set_loop(true);
			animation_dash_on_floor_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_dash_on_floor_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_dash_on_floor_right"));
		}
		{
			AnimationGroup& animation_fall = animation_pool["fall"];

			Animation& animation_fall_left = animation_fall.left;
			animation_fall_left.set_interval(0.1f);
			animation_fall_left.set_loop(true);
			animation_fall_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_fall_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_fall_left"));

			Animation& animation_fall_right = animation_fall.right;
			animation_fall_right.set_interval(0.1f);
			animation_fall_right.set_loop(true);
			animation_fall_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_fall_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_fall_right"));
		}
		{
			AnimationGroup& animation_idle = animation_pool["idle"];

			Animation& animation_idle_left = animation_idle.left;
			animation_idle_left.set_interval(0.1f);
			animation_idle_left.set_loop(true);
			animation_idle_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_idle_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_idle_left"));

			Animation& animation_idle_right = animation_idle.right;
			animation_idle_right.set_interval(0.1f);
			animation_idle_right.set_loop(true);
			animation_idle_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_idle_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_idle_right"));
		}
		{
			AnimationGroup& animation_jump = animation_pool["jump"];

			Animation& animation_jump_left = animation_jump.left;
			animation_jump_left.set_interval(0.1f);
			animation_jump_left.set_loop(false);
			animation_jump_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_jump_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_jump_left"));

			Animation& animation_jump_right = animation_jump.right;
			animation_jump_right.set_interval(0.1f);
			animation_jump_right.set_loop(false);
			animation_jump_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_jump_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_jump_right"));
		}
		{
			AnimationGroup& animation_run = animation_pool["run"];

			Animation& animation_run_left = animation_run.left;
			animation_run_left.set_interval(0.05f);
			animation_run_left.set_loop(true);
			animation_run_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_run_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_run_left"));

			Animation& animation_run_right = animation_run.right;
			animation_run_right.set_interval(0.05f);
			animation_run_right.set_loop(true);
			animation_run_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_run_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_run_right"));
		}
		{
			AnimationGroup& animation_squat = animation_pool["squat"];

			Animation& animation_squat_left = animation_squat.left;
			animation_squat_left.set_interval(0.05f);
			animation_squat_left.set_loop(false);
			animation_squat_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_squat_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_squat_left"));

			Animation& animation_squat_right = animation_squat.right;
			animation_squat_right.set_interval(0.05f);
			animation_squat_right.set_loop(false);
			animation_squat_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_squat_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_squat_right"));
		}
		{
			AnimationGroup& animation_throw_barb = animation_pool["throw_barb"];

			Animation& animation_throw_barb_left = animation_throw_barb.left;
			animation_throw_barb_left.set_interval(0.05f);
			animation_throw_barb_left.set_loop(false);
			animation_throw_barb_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_throw_barb_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_throw_barb_left"));

			Animation& animation_throw_barb_right = animation_throw_barb.right;
			animation_throw_barb_right.set_interval(0.05f);
			animation_throw_barb_right.set_loop(false);
			animation_throw_barb_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_throw_barb_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_throw_barb_right"));
		}
		{
			AnimationGroup& animation_throw_silk = animation_pool["throw_silk"];

			Animation& animation_throw_silk_left = animation_throw_silk.left;
			animation_throw_silk_left.set_interval(0.05f);
			animation_throw_silk_left.set_loop(false);
			animation_throw_silk_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_throw_silk_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_throw_silk_left"));

			Animation& animation_throw_silk_right = animation_throw_silk.right;
			animation_throw_silk_right.set_interval(0.05f);
			animation_throw_silk_right.set_loop(false);
			animation_throw_silk_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_throw_silk_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_throw_silk_right"));
		}
		{
			AnimationGroup& animation_throw_sword = animation_pool["throw_sword"];

			Animation& animation_throw_sword_left = animation_throw_sword.left;
			animation_throw_sword_left.set_interval(0.05f);
			animation_throw_sword_left.set_loop(false);
			animation_throw_sword_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_throw_sword_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_throw_sword_left"));

			Animation& animation_throw_sword_right = animation_throw_sword.right;
			animation_throw_sword_right.set_interval(0.05f);
			animation_throw_sword_right.set_loop(false);
			animation_throw_sword_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
			animation_throw_sword_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_throw_sword_right"));
		}
	}
	{
		animation_silk.set_interval(0.1f);
		animation_silk.set_loop(false);
		animation_silk.set_anchor_mode(Animation::AnchorMode::Centered);
		animation_silk.add_frame(ResourcesManager::instance()->find_atlas("silk"));

		Animation& animation_dash_in_air_left = animation_dash_in_air_vfx.left;
		animation_dash_in_air_left.set_interval(0.1f);
		animation_dash_in_air_left.set_loop(false);
		animation_dash_in_air_left.set_anchor_mode(Animation::AnchorMode::Centered);
		animation_dash_in_air_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_vfx_dash_in_air_left"));

		Animation& animation_dash_in_air_right = animation_dash_in_air_vfx.right;
		animation_dash_in_air_right.set_interval(0.1f);
		animation_dash_in_air_right.set_loop(false);
		animation_dash_in_air_right.set_anchor_mode(Animation::AnchorMode::Centered);
		animation_dash_in_air_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_vfx_dash_in_air_right"));

		Animation& animation_dash_on_floor_left = animation_dash_on_floor_vfx.left;
		animation_dash_on_floor_left.set_interval(0.1f);
		animation_dash_on_floor_left.set_loop(false);
		animation_dash_on_floor_left.set_anchor_mode(Animation::AnchorMode::BottomCentered);
		animation_dash_on_floor_left.add_frame(ResourcesManager::instance()->find_atlas("enemy_vfx_dash_on_floor_left"));

		Animation& animation_dash_on_floor_right = animation_dash_on_floor_vfx.right;
		animation_dash_on_floor_right.set_interval(0.1f);
		animation_dash_on_floor_right.set_loop(false);
		animation_dash_on_floor_right.set_anchor_mode(Animation::AnchorMode::BottomCentered);
		animation_dash_on_floor_right.add_frame(ResourcesManager::instance()->find_atlas("enemy_vfx_dash_on_floor_right"));
	}
	{
		state_machine.register_state("idle",        new EnemyIdleState());
		state_machine.register_state("run",         new EnemyRunState());
		state_machine.register_state("jump",        new EnemyJumpState());
		state_machine.register_state("fall",        new EnemyFallState());
		state_machine.register_state("aim",         new EnemyAimState());
		state_machine.register_state("dash_in_air", new EnemyDashInAirState());
		state_machine.register_state("dash_on_floor",new EnemyDashOnFloorState());
		state_machine.register_state("squat",       new EnemySquatState());
		state_machine.register_state("throw_barb",  new EnemyThrowBarbState());
		state_machine.register_state("throw_silk",  new EnemyThrowSilkState());
		state_machine.register_state("throw_sword", new EnemyThrowSwordState());
		state_machine.register_state("dead",        new EnemyDeadState());

		state_machine.set_entry("idle");
	}
}

Enemy::~Enemy() {
	CollisionManager::instance()->destroy_collision_box(collision_box_silk);
}

void Enemy::on_update(float delta) {
	if (velocity.x >= 0.0001f)
		is_facing_left = (velocity.x < 0);

	Character::on_update(delta);

	hit_box->set_position(get_logic_height());

	if (is_throwing_silk)
	{
		collision_box_silk->set_position(get_logic_height());
		animation_silk.set_position(get_logic_height());
		animation_silk.on_update(delta);
	}

	if ((is_dashing_in_air || is_dashing_on_floor) && curr_dash_animation)
	{
		curr_dash_animation->set_position(is_dashing_in_air ? get_logic_height() : position);
		curr_dash_animation->on_update(delta);
	}

	for (Barb* barb : barbs)
		if (barb->check_valid()) barb->on_update(delta);
	for (Sword* sword : swords)
		if (sword->check_valid()) sword->on_update(delta);

	barbs.erase(std::remove_if(
		barbs.begin(), barbs.end(),
		[](Barb* barb)
		{
			bool can_remove = !barb->check_valid();
			if (can_remove) delete barb;
			return can_remove;
		}),
		barbs.end());
	swords.erase(std::remove_if(
		swords.begin(), swords.end(),
		[](Sword* sword)
		{
			bool can_remove = !sword->check_valid();
			if (can_remove) delete sword;
			return can_remove;
		}),
		swords.end());
}

void Enemy::on_render() {
	for (Barb* barb : barbs)
		if (barb->check_valid()) barb->on_render();
	for (Sword* sword : swords)
		if (sword->check_valid()) sword->on_render();

	Character::on_render();

	if (is_throwing_silk)
		animation_silk.on_render();
	if ((is_dashing_in_air || is_dashing_on_floor) && curr_dash_animation)
		curr_dash_animation->on_render();
}

void Enemy::on_hurt() {
	switch (range_random(1, 3)) {
	case 1:
		play_audio(_T("enemy_hurt_1"), false);
		break;
	case 2:
		play_audio(_T("enemy_hurt_2"), false);
		break;
	case 3:
		play_audio(_T("enemy_hurt_3"), false);
		break;
	}
}

void Enemy::throw_barbs() {
	int new_barb_num = range_random(3, 6);
	if (barbs.size() >= 10)	new_barb_num = 1;
	int width_grid = getwidth() / new_barb_num;

	for (int i = 0; i < new_barb_num; i++) {
		Barb* new_barb = new Barb();
		int rand_x = range_random(width_grid * i, width_grid * (i + 1));
		int rand_y = range_random(250, 500);
		new_barb->set_position({ (float)rand_x, (float)rand_y });
		barbs.push_back(new_barb);
	}
}

void Enemy::throw_sword() {
	Sword* new_sword = new Sword(get_logic_height(), is_facing_left);
	swords.push_back(new_sword);
}

void Enemy::on_dash() {
	if (is_dashing_in_air)
		curr_dash_animation = velocity.x < 0 ? &animation_dash_in_air_vfx.left : &animation_dash_in_air_vfx.right;
	else
		curr_dash_animation = velocity.x < 0 ? &animation_dash_on_floor_vfx.left : &animation_dash_on_floor_vfx.right;

	curr_dash_animation->reset();
}

void Enemy::on_throw_silk() {
	animation_silk.reset();
}