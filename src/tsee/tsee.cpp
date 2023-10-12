#include "tsee.hpp"

using namespace tsee;

TSEE::TSEE() {
	typedef std::chrono::high_resolution_clock c;
	std::chrono::time_point last_time = c::now();
	std::chrono::duration since_epoch = last_time.time_since_epoch();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(since_epoch);
	this->last_time = microseconds.count();
	this->current_time = microseconds.count();
	this->dt = 0;
	this->last_window_id = 0;
	// this->LoadSettings();
	return;
}

TSEE::~TSEE() {
	// Clear settings
	std::ofstream file;
	file.open("settings.ini");
	file.close();
	// Each window will save its sizes
	for (window::Window w : this->windows) {
		w.Destroy();
	}
}

window::Window TSEE::CreateWindow(int width, int height, std::string title) {
	window::Window win(++this->last_window_id, width, height, title);
	this->windows.push_back(win);
	return win;
}

window::Window TSEE::CreateWindow(int width, int height) {
	window::Window win(++this->last_window_id, width, height);
	this->windows.push_back(win);
	return win;
}

window::Window TSEE::CreateWindow() {
	window::Window win(++this->last_window_id);
	this->windows.push_back(win);
	return win;
}

void TSEE::RenderAll() {
	for (window::Window w : this->windows) {
		w.Render();
	}
}