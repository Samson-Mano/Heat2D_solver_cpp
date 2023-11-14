#include "edge_window.h"

edge_window::edge_window()
{
	// Empty constructor
}

edge_window::~edge_window()
{
	// Empty destructor
}

void edge_window::init()
{
	// Clear the selected edges
	selected_edges.clear();
}

void edge_window::render_window()
{
	if (is_show_window == false)
		return;


	ImGui::Begin("Edge Constraints");


	ImGui::Spacing();
	ImGui::Spacing();

	// Options for the path (Edge heat source or Edge fixed temperature)
	if (ImGui::RadioButton("Apply Edge Heat Source", selected_constraint_option == 0))
	{
		selected_constraint_option = 0;
	}

	if (ImGui::RadioButton("Apply Edge Temperature", selected_constraint_option == 1))
	{
		selected_constraint_option = 1;
	}

	if (ImGui::RadioButton("Apply Edge Convection", selected_constraint_option == 2))
	{
		selected_constraint_option = 2;
	}



	// Display the selected option
	ImGui::Text("Selected Option: %s", (selected_constraint_option == 0) ? "Edge Heat Source" :
		(selected_constraint_option == 1) ? "Edge Temperature" : "Edge Convection");

	//_________________________________________________________________________________________


	if (selected_constraint_option == 0)
	{
		// Option 1: Apply Edge Heat Source
		ImGui::Text("Edge heat source q =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char heatSourceValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##HeatSource", heatSourceValue, sizeof(heatSourceValue), ImGuiInputTextFlags_CharsDecimal);
	}
	else if (selected_constraint_option == 1)
	{
		// Option 2: Apply Edge Temperature
		ImGui::Text("Edge specified temperature T =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char temperatureValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##SpecifiedTemperature", temperatureValue, sizeof(temperatureValue), ImGuiInputTextFlags_CharsDecimal);
	}
	else
	{
		// Option 3: Apply Edge Convection
		ImGui::Text("Edge heat transfer co-efficient h =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char heattransfercoeffValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##HeatTransferCoeff", heattransfercoeffValue, sizeof(heattransfercoeffValue), ImGuiInputTextFlags_CharsDecimal);

		//______________________________________________________

		ImGui::Text("Edge ambient temperature T_inf =");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SameLine();
		static char ambienttemperatureValue[256] = ""; // Buffer to store the input value
		ImGui::InputText("##AmbientTemperature", ambienttemperatureValue, sizeof(ambienttemperatureValue), ImGuiInputTextFlags_CharsDecimal);
	}



	//__________________________________________________________________________________________
	// Selected edge list
	ImGui::Spacing();
	ImGui::Spacing();

	static char edgeNumbers[1024] = ""; // Increase the buffer size to accommodate more characters

	geom_parameters::copyNodenumberlistToCharArray(selected_edges, edgeNumbers, 1024);

	ImGui::Text("Selected Edges: ");
	ImGui::Spacing();

	// Begin a child window with ImGuiWindowFlags_HorizontalScrollbar to enable vertical scrollbar ImGuiWindowFlags_AlwaysVerticalScrollbar
	ImGui::BeginChild("Edge Numbers", ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10), true);

	// Assuming 'edgeNumbers' is a char array or a string
	ImGui::TextWrapped("%s", edgeNumbers);

	// End the child window
	ImGui::EndChild();


	ImGui::Spacing();
	ImGui::Spacing();
	// Apply constraint button
	if (ImGui::Button("Apply"))
	{
		apply_edge_constraint = true; // set the flag to apply to the constraint
	}

	ImGui::SameLine();

	// Close button
	if (ImGui::Button("Close"))
	{
		// Clear the selected edges
		this->selected_edges.clear();

		apply_edge_constraint = false;
		delete_all_edge_constraint = false;
		is_show_window = false; // set the flag to close the window
	}

	ImGui::Spacing();
	// Delete All button
	if (ImGui::Button("Delete All"))
	{
		delete_all_edge_constraint = true; // set the flag to delete all the edge constraint
	}

	//_________________________________________________________________________________________

	ImGui::End();
}

void edge_window::add_to_edge_list(const std::vector<int>& selected_edges, const bool& is_right)
{
	if (is_right == false)
	{
		// Add to the selected edges list
		for (int edge : selected_edges)
		{
			// Check whether edges are already in the list or not
			if (std::find(this->selected_edges.begin(), this->selected_edges.end(), edge) == this->selected_edges.end())
			{
				this->selected_edges.push_back(edge);
			}
		}
	}
	else
	{
		// Remove from the selected edges list
		for (int edge : selected_edges)
		{
			// Erase the edges which is found in the list
			this->selected_edges.erase(std::remove(this->selected_edges.begin(), this->selected_edges.end(), edge),
				this->selected_edges.end());
		}

	}
}
