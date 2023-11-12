#include "wave_analysis_result_store.h"

wave_analysis_result_store::wave_analysis_result_store()
{
	// Empty Constructor
}

wave_analysis_result_store::~wave_analysis_result_store()
{
	// Empty Destructor
}

void wave_analysis_result_store::set_analysis_setting(const int& time_step_count, const double& time_interval, const double& total_simulation_time)
{
	// Set the analysis parameters
	this->time_step_count = time_step_count;
	this->time_interval = time_interval;
	this->total_simulation_time = total_simulation_time;
}

void wave_analysis_result_store::clear_results()
{
	// Clear the analysis parameters
	time_step_count = 0;
	time_interval = 0.0;
	total_simulation_time = 0.0;
}
