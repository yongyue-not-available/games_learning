#pragma once

#include "scene.h"

extern Scene* menu_scene;
extern Scene* game_scene;
extern Scene* selector_scene;

class SceneManager {
public:
	enum class StateType {
		Menu,
		Game,
		Selector
	};

	SceneManager() = default;
	~SceneManager() = default;

	void set_curr_state(Scene* scene) {
		curr_scene = scene;
		curr_scene -> on_enter();
	}

	void switch_to(StateType type) {
		curr_scene->on_exit();
		switch (type)
		{
		case SceneManager::StateType::Menu:
			curr_scene = menu_scene;
			break;
		case SceneManager::StateType::Game:
			curr_scene = game_scene;
			break;
		case SceneManager::StateType::Selector:
			curr_scene = selector_scene;
			break;
		default:
			break;
		}
		curr_scene->on_enter();
	}

	void on_update(int delta) {
		curr_scene->on_update(delta);
	}

	void on_draw(const Camera& camera) {
		curr_scene->on_draw(camera);
	}

	void on_input(const ExMessage& msg) {
		curr_scene->on_input(msg);
	}

private:
	Scene* curr_scene = nullptr;
};