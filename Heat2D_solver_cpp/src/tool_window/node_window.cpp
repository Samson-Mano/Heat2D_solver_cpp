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

	ImGui::Text("Selected Nodes: ");
	ImGui::Spacing();
	// Clear all selected nodes
	if (ImGui::Button("Clear All"))
	{
		// apply_nodal_constraint = true; // set the flag to apply to the constraint
	}

	ImGui::InputTextMultiline("Node Numbers", nodeNumbers, sizeof(nodeNumbers),
		ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10), ImGuiInputTextFlags_AllowTabInput);


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
		apply_nodal_constraint = false;
		is_show_window = false; // set the flag to close the window
	}
	//_________________________________________________________________________________________

	ImGui::End();

}
