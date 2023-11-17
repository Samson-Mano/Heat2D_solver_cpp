#pragma once
#include <iostream>
#include <cstring>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../geometry_store/geom_parameters.h"

class node_window
{
public:
	bool is_show_window = false;
	bool is_selection_changed = false;
	bool is_selected_count = false;
	bool apply_nodal_constraint = false;
	bool delete_all_nodal_constraint = false;
	int selected_constraint_option = 0; // type of constraint option (Nodal heat source or Nodal specified temperature)
	std::vector<int> selected_nodes;

	// Heat Source q
	double heatsource_q = 0.0;

	// Specified Temperature T
	double specifiedTemp_T = 0.0;


	node_window();
	~node_window();
	void init();
	void render_window();
	void add_to_node_list(const std::vector<int>& selected_nodes, const bool& is_right);

private:
	// void copyNodenumberlistToCharArray(const std::vector<int>& vec, char* charArray, size_t bufferSize);
};
