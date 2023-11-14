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


	if (selected_constraint_option == 0)
	{
		// Option 1: Apply Nodal Heat Source
		ImGui::Text("Nodal heat source q =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char heatSourceValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##HeatSource", heatSourceValue, sizeof(heatSourceValue), ImGuiInputTextFlags_CharsDecimal);
	}
	else
	{
		// Option 2: Apply Nodal Temperature
		ImGui::Text("Nodal specified temperature T =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char temperatureValue[256] = ""; // Buffer to store the input value
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
		apply_nodal_constraint = true; // set the flag to apply to the constraint
	}

	ImGui::SameLine();

	// Close button
	if (ImGui::Button("Close"))
	{
		// Clear the selected nodes
		this->selected_nodes.clear();

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
				this->selected_nodes.push_back(node);
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
		}

	}
}

//
//void node_window::copyNodenumberlistToCharArray(const std::vector<int>& vec, char* charArray, size_t bufferSize)
//{
//	// Use std::ostringstream to build the comma-separated string
//	std::ostringstream oss;
//	for (size_t i = 0; i < vec.size(); ++i) 
//	{
//		if (i > 0) 
//		{
//			oss << ", "; // Add a comma and space for each element except the first one
//		}
//		oss << vec[i];
//	}
//
//	// Copy the resulting string to the char array
//	std::string resultString = oss.str();
//
//	if (resultString.size() + 1 > bufferSize)
//	{
//		// Truncate 15 character
//		resultString.erase(bufferSize - 16);
//		resultString += "..exceeds limit";
//	}
//
//	strncpy_s(charArray, bufferSize, resultString.c_str(), _TRUNCATE);
//
//}