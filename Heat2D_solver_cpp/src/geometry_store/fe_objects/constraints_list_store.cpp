#include "constraints_list_store.h"

constraints_list_store::constraints_list_store()
{
	// Empty constructor
}

constraints_list_store::~constraints_list_store()
{
	// Empty destructor
}

void constraints_list_store::init(geom_parameters* geom_param_ptr)
{
	// Set the geometry parameters
	this->geom_param_ptr = geom_param_ptr;

	// Set the geometry parameters for the labels (and clear the labels)
	constraint_points.init(geom_param_ptr);
	constraint_value_labels.init(geom_param_ptr);

	// Clear the constraints
	constraint_count = 0;
	constraintMap.clear();
}

void constraints_list_store::add_constraints(int constraint_applied_to, int constraint_type,
	std::vector<int>& id_list, std::vector<glm::vec2>& constraint_pts,
	const double& heat_source_q, const double& specified_temperature_T,
	const double& heat_transfer_coeff_h, const double& Ambient_temperature_Tinf)
{
	// Create a temporary constraint
	constraints_store temp_cnst;
	temp_cnst.constraint_id = get_unique_constraint_id(); // constraint ID
	temp_cnst.constraint_applied_to = constraint_applied_to; // constraint Applied To (Node, Edge, Element)
	temp_cnst.constraint_type = constraint_type; // Type of constraint (heat source, specified temp, heat convection)
	temp_cnst.id_list = id_list; // store the id list of constraint applies (Nodes, Edges, Elements)
	temp_cnst.constraint_pts = constraint_pts; // Constraint points
	temp_cnst.average_pt = geom_parameters::findGeometricCenter(constraint_pts); // Average points

	temp_cnst.heat_source_q = heat_source_q; // Heat source
	temp_cnst.specified_temperature_T = specified_temperature_T; // Specified temperature
	temp_cnst.heat_transfer_coeff_h = heat_transfer_coeff_h; // Heat Transfer Co-efficient
	temp_cnst.Ambient_temperature_Tinf = Ambient_temperature_Tinf; // Ambient temperature

	// Insert to the constraint list
	constraintMap.insert({ temp_cnst.constraint_id , temp_cnst });
	constraint_count++;

	// Update the constraints
	update_constraint_pts_labels();
}

void constraints_list_store::delete_constraints(int constraint_applied_to)
{
	// Find the ids of the constraints matching the type
	std::vector<int> constraint_id;

	for (const auto& c_id : constraintMap)
	{
		// get the id
		int cnst_id = c_id.first;

		if (constraintMap[cnst_id].constraint_applied_to == constraint_applied_to)
		{
			// Constraint type matches (Add to the id list)
			constraint_id.push_back(cnst_id);
		}
	}

	// Delete the constraints
	if (static_cast<int>(constraint_id.size()) > 0)
	{
		for (const auto& id : constraint_id)
		{
			// Gothrough every id and delete
			constraintMap.erase(id);
		}

		// Update the constraint points and lables
		update_constraint_pts_labels();
	}
}

void constraints_list_store::paint_constraints()
{
	// Paint the points and labels
	constraint_points.paint_points();
	constraint_value_labels.paint_text();
}

void constraints_list_store::update_geometry_matrices(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale)
{
	// Update model openGL uniforms
	constraint_points.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
	constraint_value_labels.update_opengl_uniforms(set_modelmatrix, set_pantranslation, set_zoomtranslation, set_transparency, set_deflscale);
}

void constraints_list_store::update_constraint_pts_labels()
{
	// Get the constraint color
	glm::vec3 cnst_color = geom_param_ptr->geom_colors.constraint_color;

	// Update the constraint point and labels
	// Constraint Points
	constraint_points.clear_points();

	int pt_id = 0;
	for (const auto& cnst_m : constraintMap)
	{
		// Add the points
		for (auto cnst_pt : cnst_m.second.constraint_pts)
		{
			glm::vec2 temp_pt = glm::vec2(0);
			constraint_points.add_point(pt_id, cnst_pt, temp_pt, cnst_color, false);
		}
	}


	constraint_points.set_buffer();

	// Constraint Labels
	pt_id = 0;
	std::string temp_str = "";
	constraint_value_labels.clear_labels();

	for (const auto& cnst_m : constraintMap)
	{
		constraints_store cnst = cnst_m.second;
		std::stringstream ss;

		// Add the labels
		if (cnst.constraint_type == 0)
		{
			// Heat source q
			ss << std::fixed << std::setprecision(geom_param_ptr->constraint_precision) << cnst.heat_source_q;

			temp_str = "Heat Source q = " + ss.str();
			constraint_value_labels.add_text(temp_str, cnst.average_pt, glm::vec2(0), cnst_color, 0, true, false);

		}
		else if (cnst.constraint_type == 1)
		{
			// Specified Temperature T
			ss << std::fixed << std::setprecision(geom_param_ptr->constraint_precision) << cnst.specified_temperature_T;

			temp_str = "Temperature T = " + ss.str();
			constraint_value_labels.add_text(temp_str, cnst.average_pt, glm::vec2(0), cnst_color, 0, true, false);
		}
		else if (cnst.constraint_type == 2)
		{
			// Heat Convection h
			ss << std::fixed << std::setprecision(geom_param_ptr->constraint_precision) << cnst.heat_transfer_coeff_h;

			temp_str = "Heat transfer co-eff h = " + ss.str();
			constraint_value_labels.add_text(temp_str, cnst.average_pt, glm::vec2(0), cnst_color, 0, true, false);

			// Ambient Temperature Tinf
			ss.str("");
			ss << std::fixed << std::setprecision(geom_param_ptr->constraint_precision) << cnst.Ambient_temperature_Tinf;

			temp_str = "Ambient Temperature Tinf = " + ss.str();
			constraint_value_labels.add_text(temp_str, cnst.average_pt, glm::vec2(0), cnst_color, 0, false, false);

		}
	}

	constraint_value_labels.set_buffer();
}


int constraints_list_store::get_unique_constraint_id()
{
	// Add all the ids to a int list
	std::vector<int> all_ids;
	for (auto& mat : constraintMap)
	{
		all_ids.push_back(mat.second.constraint_id);
	}

	if (all_ids.size() != 0)
	{
		int i;
		std::sort(all_ids.begin(), all_ids.end());

		// Find if any of the nodes are missing in an ordered int
		for (i = 0; i < all_ids.size(); i++)
		{
			if (all_ids[i] != i)
			{
				return i;
			}
		}

		// no node id is missing in an ordered list so add to the end
		return static_cast<unsigned int>(all_ids.size());
	}

	// id for the first node is 0
	return 0;
}