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
	bool is_selection_changed = false;
	bool is_selected_count = false;
	bool apply_element_constraint = false;
	bool delete_all_element_constraint = false;
	int selected_constraint_option = 0; // type of constraint option (Element heat source or Element specified temperature or Element convection)
	std::vector<int> selected_elements;


	element_window();
	~element_window();
	void init();
	void render_window();
	void add_to_element_list(const std::vector<int>& selected_elements, const bool& is_right);

private:

};
