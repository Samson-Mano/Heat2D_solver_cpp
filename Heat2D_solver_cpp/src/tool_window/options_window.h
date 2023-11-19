#pragma once
#include <iostream>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"

class options_window
{
public:
	bool is_show_constraint = true;
	bool is_show_modelnodes = true;
	bool is_show_modeledges = true;
	bool is_show_modelelements = true;
	bool is_show_shrunkmesh = false;
	bool is_show_window = false;

	options_window();
	~options_window();
	void init();
	void render_window();
private:

};