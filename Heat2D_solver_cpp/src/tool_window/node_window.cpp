#include "node_window.h"

node_window::node_window()
{
	// Empty constructor
}

node_window::~node_window()
{
	// Empty destructor
}

void node_window::init()
{
	// Clear the selected nodes
	selected_nodes.clear();
}

void node_window::render_window()
{
	if (is_show_window == false)
		return;


	ImGui::Begin("Nodal Constraints");


	ImGui::Spacing();
	ImGui::Spacing();

	// Options for the path (Nodal heat source or Nodal fixed temperature)
	if (ImGui::RadioButton("Apply Nodal Heat Source", selected_constraint_option == 0))
	{
		selected_constraint_option = 0;
	}
	// ImGui::SameLine();
	if (ImGui::RadioButton("Apply Nodal Temperature", selected_constraint_option == 1))
	{
		selected_constraint_option = 1;
	}

	// Display the selected option
	ImGui::Text("Selected Option: %s", (selected_constraint_option == 0) ? "Nodal Heat Source" : "Nodal Temperature");

	//_________________________________________________________________________________________
			 // Buffers to store the input value
	static char heatSourceValue[256] = "";
	static char temperatureValue[256] = "";

	if (selected_constraint_option == 0)
	{
		// Option 1: Apply Nodal Heat Source
		ImGui::Text("Nodal heat source q =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		
		ImGui::InputText("##HeatSource", heatSourceValue, sizeof(heatSourceValue), ImGuiInputTextFlags_CharsDecimal);
	}
	else
	{
		// Option 2: Apply Nodal Temperature
		ImGui::Text("Nodal specified temperature T =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		
		ImGui::InputText("##SpecifiedTemperature", temperatureValue, sizeof(temperatureValue), ImGuiInputTextFlags_CharsDecimal);
	}

	//__________________________________________________________________________________________
	// Selected edge list
	ImGui::Spacing();
	ImGui::Spacing();

	static char nodeNumbers[1024] = ""; // Increase the buffer size to accommodate more characters

	geom_parameters::copyNodenumberlistToCharArray(selected_nodes, nodeNumbers, 1024);

	ImGui::Text("Selected Nodes: ");
	ImGui::Spacing();

	// Begin a child window with ImGuiWindowFlags_HorizontalScrollbar to enable vertical scrollbar ImGuiWindowFlags_AlwaysVerticalScrollbar
	ImGui::BeginChild("Node Numbers", ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10), true);

	// Assuming 'nodeNumbers' is a char array or a string
	ImGui::TextWrapped("%s", nodeNumbers);

	// End the child window
	ImGui::EndChild();


	ImGui::Spacing();
	ImGui::Spacing();
	// Apply constraint button
	if (ImGui::Button("Apply"))
	{
		// Get the data
		if (selected_constraint_option == 0)
		{
			// Option 1: Apply Element Heat Source
			try
			{
				heatsource_q = std::stod(heatSourceValue);
				apply_nodal_constraint = true; // set the flag to apply to the constraint
			}
			catch (const std::invalid_argument& e)
			{
				// case where conversion fails (e.g., non-numeric input)
				heatsource_q = 0.0;
			}
		}
		else if (selected_constraint_option == 1)
		{
			// Option 2: Apply Element Temperature
			try
			{
				specifiedTemp_T = std::stod(temperatureValue);
				apply_nodal_constraint = true; // set the flag to apply to the constraint
			}
			catch (const std::invalid_argument& e)
			{
				// case where conversion fails (e.g., non-numeric input)
				specifiedTemp_T = 0.0;
			}
		}
	}

	ImGui::SameLine();

	// Close button
	if (ImGui::Button("Close"))
	{
		// Clear the selected nodes
		this->selected_nodes.clear();
		is_selected_count = false; // Number of selected nodes 0
		is_selection_changed = false; // Set the selection changed

		apply_nodal_constraint = false;
		delete_all_nodal_constraint = false;
		is_show_window = false; // set the flag to close the window
	}

	ImGui::Spacing();
	// Delete All button
	if (ImGui::Button("Delete All"))
	{
		delete_all_nodal_constraint = true; // set the flag to delete all the nodal constraint
	}

	//_________________________________________________________________________________________

	ImGui::End();

}

void node_window::add_to_node_list(const std::vector<int>& selected_nodes, const bool& is_right)
{
	if (is_right == false)
	{
		// Add to the selected node list
		for (int node : selected_nodes)
		{
			// Check whether nodes are already in the list or not
			if (std::find(this->selected_nodes.begin(), this->selected_nodes.end(), node) == this->selected_nodes.end())
			{
				// Add to selected nodes
				this->selected_nodes.push_back(node);

				// Selection changed flag
				this->is_selection_changed = true;
			}
		}
	}
	else
	{
		// Remove from the selected node list
		for (int node : selected_nodes)
		{
			// Erase the node which is found in the list
			this->selected_nodes.erase(std::remove(this->selected_nodes.begin(), this->selected_nodes.end(), node),
				this->selected_nodes.end());

			// Selection changed flag
			this->is_selection_changed = true;
		}

	}

	// Number of selected nodes
	this->is_selected_count = false;
	if (static_cast<int>(this->selected_nodes.size()) > 0)
	{
		this->is_selected_count = true;
	}

}

