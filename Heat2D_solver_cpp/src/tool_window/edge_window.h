#pragma once
#include <iostream>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../geometry_store/geom_parameters.h"

class edge_window
{
public:
	bool is_show_window = false;
	bool apply_edge_constraint = false;
	bool delete_all_edge_constraint = false;
	int selected_constraint_option = 0; // type of constraint option (Edge heat source or Edge specified temperature or Edge Apply convection)
	std::vector<int> selected_edges;


	edge_window();
	~edge_window();
	void init();
	void render_window();
	void add_to_edge_list(const std::vector<int>& selected_edges, const bool& is_right);

private:
	
};
