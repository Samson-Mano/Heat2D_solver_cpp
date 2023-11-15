#include "element_window.h"

element_window::element_window()
{
	// Empty constructor
}

element_window::~element_window()
{
	// Empty destructor
}

void element_window::init()
{
	// Clear the selected elements
	selected_elements.clear();
}

void element_window::render_window()
{
	if (is_show_window == false)
		return;


	ImGui::Begin("Element Constraints");


	ImGui::Spacing();
	ImGui::Spacing();

	// Options for the path (Element heat source or Element fixed temperature)
	if (ImGui::RadioButton("Apply Element Heat Source", selected_constraint_option == 0))
	{
		selected_constraint_option = 0;
	}

	if (ImGui::RadioButton("Apply Element Temperature", selected_constraint_option == 1))
	{
		selected_constraint_option = 1;
	}

	if (ImGui::RadioButton("Apply Element Convection", selected_constraint_option == 2))
	{
		selected_constraint_option = 2;
	}



	// Display the selected option
	ImGui::Text("Selected Option: %s", (selected_constraint_option == 0) ? "Element Heat Source" :
		(selected_constraint_option == 1) ? "Element Temperature" : "Element Convection");

	//_________________________________________________________________________________________


	if (selected_constraint_option == 0)
	{
		// Option 1: Apply Element Heat Source
		ImGui::Text("Element heat source q =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char heatSourceValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##HeatSource", heatSourceValue, sizeof(heatSourceValue), ImGuiInputTextFlags_CharsDecimal);
	}
	else if (selected_constraint_option == 1)
	{
		// Option 2: Apply Element Temperature
		ImGui::Text("Element specified temperature T =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char temperatureValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##SpecifiedTemperature", temperatureValue, sizeof(temperatureValue), ImGuiInputTextFlags_CharsDecimal);
	}
	else
	{
		// Option 3: Apply Element Convection
		ImGui::Text("Element heat transfer co-efficient h =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char heattransfercoeffValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##HeatTransferCoeff", heattransfercoeffValue, sizeof(heattransfercoeffValue), ImGuiInputTextFlags_CharsDecimal);

		//______________________________________________________

		ImGui::Text("Element ambient temperature T_inf =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char ambienttemperatureValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##AmbientTemperature", ambienttemperatureValue, sizeof(ambienttemperatureValue), ImGuiInputTextFlags_CharsDecimal);
	}

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


	ImGui::Spacing();
	ImGui::Spacing();
	// Apply constraint button
	if (ImGui::Button("Apply"))
	{
		apply_element_constraint = true; // set the flag to apply to the constraint
	}

	ImGui::SameLine();

	// Close button
	if (ImGui::Button("Close"))
	{
		// Clear the selected elements
		this->selected_elements.clear();
		is_selected_count = false; // Number of selected elements 0
		is_selection_changed = false; // Set the selection changed

		apply_element_constraint = false;
		delete_all_element_constraint = false;
		is_show_window = false; // set the flag to close the window
	}

	ImGui::Spacing();
	// Delete All button
	if (ImGui::Button("Delete All"))
	{
		delete_all_element_constraint = true; // set the flag to delete all the Element constraint
	}

	//_________________________________________________________________________________________

	ImGui::End();

}


void element_window::add_to_element_list(const std::vector<int>& selected_elements, const bool& is_right)
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
