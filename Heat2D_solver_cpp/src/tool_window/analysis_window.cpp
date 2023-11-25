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

	ImGui::Begin("Heat Equation Solver");

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

	// Example values
	float minValue = contour_minvalue;
	float maxValue = contour_maxvalue;
	int numLevels = 5;

	// Show contour bars in the ImGui window
	ImGui::Text("Contour Bars");

	// Plot the contour bars
	if (ImPlot::BeginPlot("ContourBars", nullptr, nullptr, ImVec2(-1, 150)))
	{
		ImPlot::SetNextPlotLimitsY(minValue, maxValue);
		for (int i = 0; i < numLevels; ++i)
		{
			float levelValue = minValue + static_cast<float>(i) / (numLevels - 1) * (maxValue - minValue);
			ImPlot::PlotBars("", &levelValue, 1, 0.1f);
		}
		ImPlot::EndPlot();
	}

	ImGui::End();

}

void analysis_window::set_maxmin(const double& contour_maxvalue, const double& contour_minvalue)
{
	// Set the contour maximum and minimum
	this->contour_maxvalue = contour_maxvalue;
	this->contour_minvalue = contour_minvalue;
}

