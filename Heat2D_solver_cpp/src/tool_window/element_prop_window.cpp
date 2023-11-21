#include "element_prop_window.h"


element_prop_window::element_prop_window()
{
	// Empty Constructor
}

element_prop_window::~element_prop_window()
{
	// Empty Destructor
}

void element_prop_window::init()
{

}

void element_prop_window::render_window()
{

	if (is_show_window == false)
		return;

	if (material_list.size() == 0)
	{
		// Avoid opening window with no material in the list
		is_show_window = false;
		return;
	}


	ImGui::Begin("Materials");

	std::vector<const char*> material_names;
	std::unordered_map<int,int> material_id_selected_option;
	int i = 0;

	for (const auto& mat : material_list)
	{
		material_id_selected_option[i] = mat.first;
		material_names.push_back(mat.second.material_name.c_str());
		i++;
	}


	ImGui::ListBox("Select Material", &selected_list_option, material_names.data(), static_cast<unsigned int>(material_names.size()), 4);

	ImGui::Spacing();

	selected_material_option = material_id_selected_option[selected_list_option];

	// Get selected material
	const material_data& selected_material_data = material_list[selected_material_option];

	// Get the color for this material
	glm::vec3 std_color = geom_parameters::get_standard_color(selected_material_data.material_id);
	ImVec4 text_color = ImVec4(std_color.x, std_color.y, std_color.z, 1.0f);

	ImGui::TextColored(text_color, "Material ID: %i", selected_material_data.material_id);
	ImGui::TextColored(text_color, "Selected Material: %s", selected_material_data.material_name.c_str());
	ImGui::TextColored(text_color, "Thermal Conductivity Kx: %.3f", selected_material_data.thermal_conductivity_kx);
	ImGui::TextColored(text_color, "Thermal Conductivity Ky: %.3f", selected_material_data.thermal_conductivity_ky);
	ImGui::TextColored(text_color, "Element Thickness: %.3f", selected_material_data.element_thickness);

	// Diable delete if the selected option is Default (0)
	const bool is_delete_button_disabled = selected_list_option == 0 ? true : false;
	ImGui::BeginDisabled(is_delete_button_disabled);
	if (ImGui::Button("Delete Material")) {
		// Delete material
		execute_delete_materialid = selected_material_data.material_id;
		material_list.erase(selected_material_data.material_id);
		selected_list_option = 0;
	}
	ImGui::EndDisabled();

	// List the selected Elements
		//__________________________________________________________________________________________
	// Selected Element list
	ImGui::Spacing();
	ImGui::Spacing();

	static char elementNumbers[1024] = ""; // Increase the buffer size to accommodate more characters

	geom_parameters::copyNodenumberlistToCharArray(selected_elements, elementNumbers, 1024);

	ImGui::Text("Selected Elements: ");
	ImGui::Spacing();

	// Begin a child window with ImGuiWindowFlags_HorizontalScrollbar to enable vertical scrollbar ImGuiWindowFlags_AlwaysVerticalScrollbar
	ImGui::BeginChild("Element Numbers", ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10), true);

	// Assuming 'elementNumbers' is a char array or a string
	ImGui::TextWrapped("%s", elementNumbers);

	// End the child window
	ImGui::EndChild();

	// Add some spacing before the "Create Material" header
	ImGui::Spacing();
	ImGui::Spacing();

	// Assign material dropdown
		// Apply element properties button
	if (ImGui::Button("Apply"))
	{
		apply_element_properties = true; // set the flag to apply to the constraint
	}

	ImGui::Spacing();

	// Create material dropdown
	if (ImGui::CollapsingHeader("Create Material  ", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static char new_material_name[256] = "New Material";
		static double new_material_thermalKx = 100; // W/m degC
		static double new_material_thermalKy = 100; // W/m degC
		static double new_material_thickness = 0.2; // cm

		ImGui::InputText("Material Name", new_material_name, IM_ARRAYSIZE(new_material_name));
		ImGui::InputDouble("Thermal Conductivity Kx", &new_material_thermalKx, 0, 0, "%.3f");
		ImGui::InputDouble("Thermal Conductivity Ky", &new_material_thermalKy, 0, 0, "%.3f");
		ImGui::InputDouble("Element Thickness", &new_material_thickness, 0, 0, "%.3f");

		if (ImGui::Button("Create Material"))
		{
			// Add the new material to the material list
			material_data new_material;
			new_material.material_id = get_unique_material_id();
			new_material.material_name = new_material_name;
			new_material.thermal_conductivity_kx = new_material_thermalKx;
			new_material.thermal_conductivity_ky = new_material_thermalKy;
			new_material.element_thickness = new_material_thickness;
			material_list[new_material.material_id] = new_material;

			// Update the combo box
			selected_list_option = static_cast<unsigned int>(material_list.size()) - 1;
		}
	}

	ImGui::Spacing();

	// Add a "Close" button
	if (ImGui::Button("Close"))
	{
		// Clear the selected elements
		this->selected_elements.clear();
		is_selected_count = false; // Number of selected elements 0
		is_selection_changed = false; // Set the selection changed

		apply_element_properties = false;
		is_show_window = false;
	}

	ImGui::End();

}

int element_prop_window::get_unique_material_id()
{
	// Add all the ids to a int list
	std::vector<int> all_ids;
	for (auto& mat : material_list)
	{
		all_ids.push_back(mat.first);
	}

	if (all_ids.size() != 0)
	{
		int i;
		std::sort(all_ids.begin(), all_ids.end());

		// Find if any of the nodes are missing in an ordered int
		for (i = 0; i < all_ids.size(); i++)
		{
			if (all_ids[i] != i)
			{
				return i;
			}
		}

		// no node id is missing in an ordered list so add to the end
		return static_cast<unsigned int>(all_ids.size());
	}

	// id for the first node is 0
	return 0;
}



void element_prop_window::add_to_element_list(const std::vector<int>& selected_elements, const bool& is_right)
{
	if (is_right == false)
	{
		// Add to the selected elements list
		for (int elmnt : selected_elements)
		{
			// Check whether elements are already in the list or not
			if (std::find(this->selected_elements.begin(), this->selected_elements.end(), elmnt) == this->selected_elements.end())
			{
				// Add to selected elements
				this->selected_elements.push_back(elmnt);

				// Selection changed flag
				this->is_selection_changed = true;
			}
		}
	}
	else
	{
		// Remove from the selected elements list
		for (int elmnt : selected_elements)
		{
			// Erase the elements which is found in the list
			this->selected_elements.erase(std::remove(this->selected_elements.begin(), this->selected_elements.end(), elmnt),
				this->selected_elements.end());

			// Selection changed flag
			this->is_selection_changed = true;
		}
	}

	// Number of selected elements
	this->is_selected_count = false;
	if (static_cast<int>(this->selected_elements.size()) > 0)
	{
		this->is_selected_count = true;
	}
}
