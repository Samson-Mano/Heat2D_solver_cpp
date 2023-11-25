#pragma once
#include <iostream>
#include "../ImGui/imgui.h"
#include "../ImGui/implot.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../geometry_store/geom_parameters.h"

class analysis_window
{
public:
	bool is_show_window = false;
	bool execute_heat_analysis = false; // Main solver run event flag
	bool execute_open = false; // Solver window execute opening event flag
	bool execute_close = false; // Closing of solution window event flag

	// analysis results
	bool heat_analysis_complete = false;
	bool show_model = true; // show undeformed model 



	analysis_window();
	~analysis_window();
	void init();
	void render_window();
	void set_maxmin(const double& contour_maxvalue, const double& contour_minvalue);
private:
	double contour_maxvalue = 100.0;
	double contour_minvalue = 0.0;
};