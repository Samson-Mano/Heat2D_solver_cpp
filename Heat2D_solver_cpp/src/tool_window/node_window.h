#pragma once
#include <iostream>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../geometry_store/geom_parameters.h"

class node_window
{
public:
	bool is_show_window = false;
	bool apply_nodal_constraint = false;
	int selected_constraint_option = 0; // type of constraint option (Nodal heat source or Nodal specified temperature)


	node_window();
	~node_window();
	void init();
	void render_window();

private:

};
