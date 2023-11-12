#pragma once
#include <iostream>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../geometry_store/geom_parameters.h"

class element_window
{
public:
	bool is_show_window = false;

	element_window();
	~element_window();
	void init();
	void render_window();

private:

};
