#pragma once

#include "util.h"
#include "timer.h"
#include "atlas.h"
#include "vector2.h"

#include <functional>
#include <graphics.h>

class Animation {
public:
	enum class AnchorMode {
		Centered,
		BottomCentered
	};

public:
	Animation() {
		timer.set_one_shot(false);
		timer.set_on_timeout([&]() {
			idx_frame++;
			if (idx_frame >= frame_list.size()) {
				idx_frame = is_loop ? 0 : frame_list.size() - 1;
				if (!is_loop && on_finished) {
					on_finished();
				}
			}
			});
	}

	~Animation() = default;

	void reset() {
		timer.restart();
		idx_frame = 0;
	}

	void set_anchor_mode(AnchorMode mode) {
		anchor_mode = mode;
	}

	void set_position(const Vector2 position) {
		this->position = position;
	}

	void set_loop(bool flag) {
		is_loop = flag;
	}

	void set_interval(float ms) {
		timer.set_wait_time(ms);
	}

	void set_on_finished(std::function<void()> callback) {
		this->on_finished = callback;
	}

	void add_frame(IMAGE* img, int num_h) {
		int width = img->getwidth();
		int height = img->getheight();
		int width_frame = width / num_h;

		for (int i = 0; i < num_h; i++) {
			Rect rect_src;
			rect_src.x = i * width_frame, rect_src.y = 0;
			rect_src.w = width_frame, rect_src.h = height;

			frame_list.emplace_back(img, rect_src);
		}
	}

	void add_frame(Atlas* atlas) {
		for (int i = 0; i < atlas->get_size(); i++) {
			IMAGE* img = atlas->get_image(i);
			int width = img->getwidth();
			int height = img->getheight();

			Rect rect_src;
			rect_src.x = 0, rect_src.y = 0;
			rect_src.w = width, rect_src.h = height;

			frame_list.emplace_back(img, rect_src);
		}
	}

	void on_update(float delta) {
		timer.on_update(delta);
	}

	void on_render() {
		const Frame& frame = frame_list[idx_frame];

		Rect rect_dst;
		rect_dst.x = (int)position.x - frame.rect_src.w / 2;
		rect_dst.y = (anchor_mode == AnchorMode::Centered)
			? (int)position.y - frame.rect_src.h / 2 : (int)position.y - frame.rect_src.h;
		rect_dst.w = frame.rect_src.w, rect_dst.h = frame.rect_src.h;

		putimage_ex(frame.img, &rect_dst, &frame.rect_src);
	}

private:
	struct Frame {
		Rect rect_src;
		IMAGE* img = nullptr;

		Frame() = default;
		Frame(IMAGE* img, const Rect rect_src)
			: img(img), rect_src(rect_src){ }

		~Frame() = default;
	};

private:
	Timer timer;
	Vector2 position;
	size_t idx_frame = 0;
	bool is_loop = true;
	std::vector<Frame> frame_list;
	std::function<void()> on_finished;
	AnchorMode anchor_mode = AnchorMode::Centered;
};