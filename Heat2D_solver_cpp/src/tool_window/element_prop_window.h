#pragma once
#include <iostream>
#include <cstring>
#include <unordered_map>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../geometry_store/geom_parameters.h"

class element_prop_window
{
public:
	bool is_show_window = false;
	bool is_selection_changed = false;
	bool is_selected_count = false;
	bool apply_element_properties = false;
	int execute_delete_materialid = -1;

	int selected_material_option = 0;
	std::unordered_map<int,material_data> material_list;
	std::vector<int> selected_elements;

	element_prop_window();
	~element_prop_window();

	void init();
	void render_window();
	void add_to_element_list(const std::vector<int>& selected_elements, const bool& is_right);
private:
	int selected_list_option = 0;
	int get_unique_material_id();

};
