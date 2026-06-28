#include <graphics.h>
#include <string>
#include <vector>

#pragma comment(lib, "Msimg32.lib")

inline void putimage_alpha(int x, int y, IMAGE* img){
	int width = img->getwidth();
	int height = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, width, height, GetImageHDC(img), 0, 0, width, height, {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA});
}

class Animation {
public:
	Animation(LPCTSTR path, int num, int interval) {
		interval_ms = interval;

		TCHAR buffer[256];
		for(size_t i = 0; i < num; i++){
			_stprintf_s(buffer, path, i);
			IMAGE* img = new IMAGE();
			loadimage(img, buffer);
			frame_list.push_back(img);
		}
	}
	~Animation() {
		for(size_t i = 0; i < frame_list.size(); i++){
			delete frame_list[i];
		}
	}
	void play(int x, int y, int elapsed_time) {
		timer += elapsed_time;
		if(timer >= interval_ms){
			timer = 0;
			current_frame = (current_frame + 1) % frame_list.size();
		}

		putimage_alpha(x, y, frame_list[current_frame]);
	}
	int get_current_frame() const {
		return current_frame;
	}
private:
	int timer = 0;
	int current_frame = 0;
	int interval_ms = 0;
	std::vector<IMAGE*> frame_list;
};

class Player {
public:
	Player() {
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(_T("img/paimon_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/paimon_right_%d.png"), 6, 45);
	}

	~Player() {
		delete anim_left;
		delete anim_right;
	}

	void process_input(const ExMessage& msg) {
		switch (msg.message) {
		case WM_KEYDOWN:
				switch (msg.vkcode) {
				case VK_LEFT:
					is_moving_left = true;
					break;
				case VK_RIGHT:
					is_moving_right = true;
					break;
				case VK_UP:
					is_moving_up = true;
					break;
				case VK_DOWN:
					is_moving_down = true;
					break;
				}
				break;
			case WM_KEYUP:
				switch (msg.vkcode) {
				case VK_LEFT:
					is_moving_left = false;
					break;
				case VK_RIGHT:
					is_moving_right = false;
					break;
				case VK_UP:
					is_moving_up = false;
					break;
				case VK_DOWN:
					is_moving_down = false;
					break;
				}
				break;
		}
	}

	void move() {
		int dx = is_moving_right - is_moving_left;
		int dy = is_moving_down - is_moving_up;
		double length = sqrt(dx * dx + dy * dy);
		if(length > 0){
			player_pos.x += static_cast<int>(dx / length * speed);
			player_pos.y += static_cast<int>(dy / length * speed);
		}
		if (player_pos.x < 0) player_pos.x = 0;
		if (player_pos.x > 1280 - player_width) player_pos.x = 1280 - player_width;
		if (player_pos.y < 0) player_pos.y = 0;
		if (player_pos.y > 720 - player_height) player_pos.y = 720 - player_height;
	}

	void draw(int elapsed_time) {
		int shadow_x = player_pos.x + (player_width - player_shadow_width) / 2;
		int shadow_y = player_pos.y + player_height - 8;
		putimage_alpha(shadow_x, shadow_y, &img_shadow);

		static bool face_left = false;
		int dir_x = is_moving_right - is_moving_left;
		if (dir_x < 0) {
			face_left = true;
		}
		else if (dir_x > 0) {
			face_left = false;
		}
		if (face_left) {
			anim_left -> play(player_pos.x, player_pos.y, elapsed_time);
		}
		else {
			anim_right -> play(player_pos.x, player_pos.y, elapsed_time);
		}
	}

	POINT get_position() const {
		return player_pos;
	}

private:
	const int speed = 5;
	const int player_width = 80;
	const int player_height = 80;
	const int player_shadow_width = 32;

	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT player_pos = { 500, 500 };
	bool is_moving_left = false;
	bool is_moving_right = false;
	bool is_moving_up = false;
	bool is_moving_down = false;
};

class Bullet {
public:
	POINT pos = { 0, 0 };
public:
	Bullet() = default;
	~Bullet() = default;

	void draw() const {
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(pos.x, pos.y, RADIUS);
	}

private:
	const int RADIUS = 10;
};

class Enemy {
public:
	Enemy() {
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(_T("img/boar_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/boar_right_%d.png"), 6, 45);

		enum class spown_side { left, right, top, bottom };

		spown_side side = static_cast<spown_side>(rand() % 4);
		switch (side) {
		case spown_side::left:
			pos.x = -enemy_width;
			pos.y = rand() % (720 - enemy_height);
			face_left = false;
			break;
		case spown_side::right:
			pos.x = 1280;
			pos.y = rand() % (720 - enemy_height);
			face_left = true;
			break;
		case spown_side::top:
			pos.x = rand() % (1280 - enemy_width);
			pos.y = -enemy_height;
			face_left = rand() % 2 == 0;
			break;
		case spown_side::bottom:
			pos.x = rand() % (1280 - enemy_width);
			pos.y = 720;
			face_left = rand() % 2 == 0;
			break;
		default:
			break;
		}
	}

	bool check_bullet_collision(const Bullet& bullet) const {
		bool is_overlap_x = bullet.pos.x >= pos.x && bullet.pos.x  <= pos.x + enemy_width;
		bool is_overlap_y = bullet.pos.y >= pos.y && bullet.pos.y <= pos.y + enemy_height;
		return is_overlap_x && is_overlap_y;
	}

	bool check_player_collision(const Player& player) const {
		POINT check_pos = {	pos.x + enemy_width / 2, pos.y + enemy_height / 2 };
		bool is_overlap_x = check_pos.x >= player.get_position().x && check_pos.x <= player.get_position().x + 80;
		bool is_overlap_y = check_pos.y >= player.get_position().y && check_pos.y <= player.get_position().y + 80;
		return is_overlap_x && is_overlap_y;
	}

	void move(const Player& player) {
		const POINT& player_pos = player.get_position();
		int dx = player_pos.x - pos.x;
		int dy = player_pos.y - pos.y;
		double length = sqrt(dx * dx + dy * dy);
		if(length > 0){
			pos.x += static_cast<int>(dx / length * speed);
			pos.y += static_cast<int>(dy / length * speed);
		}

		if(dx < 0) {
			face_left = true;
		}
		else if(dx > 0) {
			face_left = false;
		}
	}

	void draw(int elapsed_time) {
		int shadow_x = pos.x + (enemy_width - enemy_shadow_width) / 2;
		int shadow_y = pos.y + enemy_height - 35;
		putimage_alpha(shadow_x, shadow_y, &img_shadow);
		if (face_left) {
			anim_left -> play(pos.x, pos.y, elapsed_time);
		}
		else {
			anim_right -> play(pos.x, pos.y, elapsed_time);
		}
	}

	void hurt() {
		alive = false;
	}

	bool is_alive() const {
		return alive;
	}

	~Enemy() {
		delete anim_left;
		delete anim_right;
	}
private:
	const int speed = 2;
	const int enemy_width = 80;
	const int enemy_height = 80;
	const int enemy_shadow_width = 48;

	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT pos = { 0, 0 };
	bool face_left = false;
	bool alive = true;
};

void try_spawn_enemy(std::vector<Enemy*>& enemies) {
	const int spawn_interval = 100;
	static int count = 0;
	if ((++count) % spawn_interval == 0) {
		enemies.push_back(new Enemy());
	}
}

void update_bullets(std::vector<Bullet>& bullets, const Player& player) {
	const double radial_speed = 0.0045;
	const double tangent_speed = 0.0055;
	double radian_interval = 2 * 3.14159265358979323846 / bullets.size();
	POINT player_pos = player.get_position();
	double radius = 100 + 25 * sin(GetTickCount() * radial_speed);
	for (int i = 0; i < 3; ++i) {
		double angle = GetTickCount() * tangent_speed + i * radian_interval;
		bullets[i].pos.x = static_cast<int>(player_pos.x + 40 + radius * cos(angle));
		bullets[i].pos.y = static_cast<int>(player_pos.y + 40 + radius * sin(angle));
	}
}

int main(){
	initgraph(1280, 720);

	bool flag = true;

	ExMessage msg;
	IMAGE img_background;

	loadimage(&img_background, _T("img/background.png"), 1280, 720);
	Player player;
	std::vector<Enemy*> enemies;
	std::vector<Bullet> bullets(3);

	BeginBatchDraw();

	while(flag){
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg)) {
			player.process_input(msg);
			if(msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE) {
				flag = false;
			}
		}

		player.move();
		update_bullets(bullets, player);
		try_spawn_enemy(enemies);
		for(Enemy* enemy : enemies) {
			enemy->move(player);
		}

		for(Enemy* enemy : enemies) {
			if(enemy->check_player_collision(player)) {
				MessageBox(GetHWnd(), _T("Game Over!"), _T("Game Over"), MB_OK);
				flag = false;
				break;
			}
		}

		for(Enemy* enemy : enemies) {
			for(Bullet bullet : bullets) {
				if(enemy->check_bullet_collision(bullet)) {
					enemy->hurt();
				}
			}
		}

		for(size_t i = 0; i < enemies.size(); i++) {
			if(!enemies[i]->is_alive()) {
				Enemy* enemy = enemies[i];
				std::swap(enemies[i], enemies.back());
				enemies.pop_back();
				delete enemy;
			}
		}

		cleardevice();

		putimage(0, 0, &img_background);
		player.draw(1000 / 144);
		for(Enemy* enemy : enemies) {
			enemy->draw(1000 / 144);
		}
		for (const Bullet& bullet : bullets) {
			bullet.draw();
		}

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD elapsed_time = end_time - start_time;
		if(elapsed_time < 1000 / 144){
			Sleep(1000 / 144 - elapsed_time);
		}
	}

	EndBatchDraw();

	return 0;
}