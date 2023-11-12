#pragma once
#include <iostream>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "../geometry_store/geom_parameters.h"

class analysis_window
{
public:
	bool is_show_window = false;
	bool execute_dynamic_analysis = false; // Main solver run event flag
	bool execute_open = false; // Solver window execute opening event flag
	bool execute_close = false; // Closing of solution window event flag
	int solver_type = 0; // Solver type - 0 Regular solve, 1 Variable mass solve

	// analysis results
	bool wave_analysis_complete = false;

	bool show_undeformed_model = true; // show undeformed model 

	// Inputs for response calculation
	double total_simulation_time = 30.0;
	double time_interval = 0.005;

	// Animation control
	bool animate_play = true;
	bool animate_pause = false;
	double deformation_scale_max = 40.0;
	double animation_speed = 1.0;

	// Time step control
	double time_interval_atrun = 0.0; // Value of time interval used in the pulse response 
	int time_step_count = 0;
	int time_step = 0;


	analysis_window();
	~analysis_window();
	void init();
	void render_window();
private:
	Stopwatch stopwatch;
};