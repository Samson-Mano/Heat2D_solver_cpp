#include "analysis_window.h"

analysis_window::analysis_window()
{
	// Empty constructor
}

analysis_window::~analysis_window()
{
	// Empty destructor
}

void analysis_window::init()
{
	is_show_window = false; // Solver window open event flag
	execute_heat_analysis = false; // Main solver run event flag
	execute_open = false; // Solver window execute opening event flag
	execute_close = false; // Closing of solution window event flag

	heat_analysis_complete = false;
}

void analysis_window::render_window()
{
	if (is_show_window == false)
		return;

	ImGui::Begin("Wave Equation Solver");

	//_________________________________________________________________________________________
	// Add a Analysis button
	if (ImGui::Button("Analysis"))
	{
		execute_heat_analysis = true;
	}

	ImGui::Spacing();
	ImGui::Spacing();

	
	// Add check boxes to show the Deformed model
	ImGui::Checkbox("Show Model", &show_model);


	ImGui::Spacing();
	ImGui::Spacing();
	//_________________________________________________________________________________________

		// Close button
	if (ImGui::Button("Close"))
	{
		execute_close = true;
		execute_open = false;
		is_show_window = false; // set the flag to close the window
	}

	ImGui::End();

}
