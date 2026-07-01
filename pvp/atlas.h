#pragma once

#include <vector>
#include <graphics.h>

class Atlas {
public:
	Atlas() = default;
	~Atlas() = default;

	void load_from_file(LPCTSTR path_template, int num) {
		image_list.clear();
		image_list.resize(num);

		TCHAR path_file[256];
		for (int i = 0; i < num; i++) {
			_stprintf_s(path_file, path_template, i + 1);
			loadimage(&image_list[i], path_file);
		}
	}

	void clear() {
		image_list.clear();
	}

	int get_size() {
		return (int)image_list.size();
	}

	IMAGE* get_image(int idx) {
		if (idx < 0 || idx >= image_list.size()) {
			return nullptr;
		}

		return &image_list[idx];
	}

	void add_image(const IMAGE& img) {
		image_list.push_back(img);
	}

private:
	std::vector<IMAGE> image_list;
};