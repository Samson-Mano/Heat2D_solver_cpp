#include "mouse_events.h"

mouse_events::mouse_events()
{
	// Empty constructor
}

mouse_events::~mouse_events()
{
	// Empty destructor
}

void mouse_events::init(geom_store* geom, analysis_window* sol_window, options_window* op_window,
	node_window* nd_window, edge_window* edg_window, element_window* elm_window, element_prop_window* elm_prop_window)
{
	// Intialize the geometry and tool window pointers
	this->geom = geom;

	// Tool windows
	this->sol_window = sol_window; // solver window
	this->op_window = op_window; // option window
	this->nd_window = nd_window; // node constraint window
	this->edg_window = edg_window; // edge constraint window
	this->elm_window = elm_window; // element constraint window
	this->elm_prop_window = elm_prop_window; // element properties window
}

void mouse_events::mouse_location(glm::vec2& loc)
{
	// Copy the current mouse location only when for pan or zoom operation
	if (is_pan == true)
	{
		// Pan operation in progress
		glm::vec2 delta_d = click_pt - loc;
		// pan
		// std::cout << "Pan translation "<<delta_d.x<<", " << delta_d.y << std::endl;
		glm::vec2 current_translataion = delta_d / (std::max((geom->geom_param.window_width), (geom->geom_param.window_height)) * 0.5f);
		pan_operation(current_translataion);
	}

	// Select operation in progress
	if (is_select == true)
	{
		select_operation(click_pt, loc);
	}
}

void mouse_events::pan_operation_start(glm::vec2& loc)
{
	// Pan operation start
	is_pan = true;
	// Note the click point when the pan operation start
	click_pt = loc;
	//std::cout << "Pan Operation Start" << std::endl;
}

void mouse_events::pan_operation(glm::vec2& current_translataion)
{
	// Pan operation in progress
	total_translation = (prev_translation + current_translataion);

	geom->update_model_pan(total_translation);
}

void mouse_events::pan_operation_ends()
{
	// Pan operation complete
	prev_translation = total_translation;
	is_pan = false;
	//std::cout << "Pan Operation End" << std::endl;
}

void mouse_events::select_operation_start(glm::vec2& loc, bool is_rightbutton)
{
	// Select operation start
	is_select = true;
	this->is_rightbutton = is_rightbutton;
	// Note the click point when the pan operation start
	click_pt = loc;
}

void mouse_events::select_operation(glm::vec2& click_loc,glm::vec2& current_loc)
{
	// Selection operation in progress
	bool is_paint = true;
	geom->update_selection_rectangle(click_loc, current_loc,is_paint,is_select,is_rightbutton);
}

void mouse_events::select_operation_ends(glm::vec2& current_loc)
{
	// Selection operation completes
	bool is_paint = false;

	geom->update_selection_rectangle(click_pt, current_loc, is_paint,is_select,is_rightbutton);
	is_select = false;
}

void mouse_events::zoom_operation(double& e_delta, glm::vec2& loc)
{
	// Screen point before zoom
	glm::vec2 screen_pt_b4_scale = intellizoom_normalized_screen_pt(loc);

	// Zoom operation
	if ((e_delta) > 0)
	{
		// Scroll Up
		if (zoom_val < 1000)
		{
			zoom_val = zoom_val + 0.1f;
		}
	}
	else if ((e_delta) < 0)
	{
		// Scroll Down
		if (zoom_val > 0.101)
		{
			zoom_val = zoom_val - 0.1f;
		}
	}

	// Hypothetical Screen point after zoom
	glm::vec2 screen_pt_a4_scale = intellizoom_normalized_screen_pt(loc);
	glm::vec2 g_tranl = -0.5f * static_cast<float>(zoom_val) * (screen_pt_b4_scale - screen_pt_a4_scale);

	// Set the zoom
	geom->update_model_zoom(zoom_val);

	// Perform Translation for Intelli Zoom
	pan_operation(g_tranl);
	pan_operation_ends();
}

void mouse_events::zoom_to_fit()
{
	// Zoom to fit the model
	prev_translation = glm::vec2(0);
	zoom_val = 1.0f;
	geom->update_model_zoomfit();
	// std::cout << "Zoom val: " << zoom_val << std::endl;
}

void mouse_events::left_mouse_click(glm::vec2& loc)
{
	/*

	// Left mouse single click
	if ((ct_window->is_add_constraint) == true)
	{
		// Add constraint
		geom->set_nodal_constraint(loc, ct_window->constraint_type, ct_window->constraint_angle, true);
	}

	
	if ((ld_window->is_add_load) == true)
	{
		// Add Loads
		geom->set_member_load(loc,ld_window->load_start_time,ld_window->load_end_time,
			ld_window->load_amplitude,ld_window->load_angle, true);
	}

	if ((mat_window->is_assign_material) == true)
	{
		// Assign material
		geom->set_elementline_material(loc);
	}

	if ((ptm_window->is_add_pointmass) == true)
	{
		// Add Point Mass
		geom->set_nodal_pointmass(loc, ptm_window->mass_x, ptm_window->mass_y, true);
	}

	if ((inl_window->is_add_initial_condition) == true)
	{
		// Add initial condition
		geom->set_nodal_initialcondition(loc, inl_window->initial_displacement_x, inl_window->initial_displacement_y,
			inl_window->initial_velocity_x, inl_window->initial_velocity_y, true);
	}

	*/


	/*
	glm::vec2 mouse_click_loc, double& load_param, double& load_start_time, double& load_end_time,
	double& load_value, double& load_angle, bool is_add)
	*/

	// std::cout << "Left mouse single click" << std::endl;
}

void mouse_events::left_mouse_doubleclick(glm::vec2& loc)
{
	// Left mouse double click
// std::cout << "Left mouse double click" << std::endl;
}

void mouse_events::right_mouse_click(glm::vec2& loc)
{
	/*

	// Right mouse single click
	if ((ct_window->is_add_constraint) == true)
	{
		// Remove constraint
		geom->set_nodal_constraint(loc, ct_window->constraint_type, ct_window->constraint_angle, false);
	}

	if ((ld_window->is_add_load) == true)
	{
		// Remove Loads
		geom->set_member_load(loc, ld_window->load_start_time, ld_window->load_end_time,
			ld_window->load_amplitude, ld_window->load_angle, false);
	}

	if ((ptm_window->is_add_pointmass) == true)
	{
		// Remove Point Mass
		geom->set_nodal_pointmass(loc, ptm_window->mass_x, ptm_window->mass_x,false);
	}


	if ((inl_window->is_add_initial_condition) == true)
	{
		// Remove initial condition
		geom->set_nodal_initialcondition(loc, inl_window->initial_displacement_x, inl_window->initial_displacement_y,
			inl_window->initial_velocity_x, inl_window->initial_velocity_y, false);
	}

	*/

	// std::cout << "Right mouse single click" << std::endl;
}

void mouse_events::right_mouse_doubleclick(glm::vec2& loc)
{
	// Right mouse double click
// std::cout << "Right mouse double click" << std::endl;
}

glm::vec2 mouse_events::intellizoom_normalized_screen_pt(glm::vec2 loc)
{
	// Function returns normalized screen point for zoom operation
	glm::vec2 mid_pt = glm::vec2((geom->geom_param.window_width), (geom->geom_param.window_height)) * 0.5f;
	int min_size = std::min((geom->geom_param.window_width), (geom->geom_param.window_height));

	glm::vec2 mouse_pt = (-1.0f * (loc - mid_pt)) / (static_cast<float>(min_size) * 0.5f);

	return (mouse_pt - (2.0f * prev_translation)) / static_cast<float>(zoom_val);
}
