#pragma once
#include <iostream>

class wave_analysis_result_store
{
public:	
	int time_step_count = 0;
	  double time_interval = 0.0;
	  double total_simulation_time = 0.0;

	wave_analysis_result_store();
	~wave_analysis_result_store();
	void set_analysis_setting(const int& time_step_count, const double& time_interval, const double& total_simulation_time);
	void clear_results();

private:

};

