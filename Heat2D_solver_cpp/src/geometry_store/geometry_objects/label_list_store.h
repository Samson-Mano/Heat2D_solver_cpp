#pragma once
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "../geometry_buffers/gBuffers.h"
#include "../geometry_buffers/font_atlas.h"
#include "../geom_parameters.h"

struct label_text
{
	// Store the individual label
	std::string label = "";
	glm::vec2 label_loc = glm::vec2(0);
	glm::vec2 label_offset = glm::vec2(0);
	glm::vec3 label_color = glm::vec3(0);
	double label_angle = 0.0;
	bool label_above_loc = false;
	bool is_offset = false;
};


class label_list_store
{
	// Stores all the labels
public:
	geom_parameters* geom_param_ptr = nullptr;
	unsigned int total_char_count = 0;
	std::vector<label_text> labels;

	label_list_store();
	~label_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_text(std::string& label, glm::vec2& label_loc, glm::vec2 label_offset, glm::vec3& label_color,
		double label_angle, bool above_point, bool is_offset);
	void set_buffer();
	void paint_text();
	void clear_labels();
	void update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation,
		bool set_zoomtranslation, bool set_transparency, bool set_deflscale);
private:
	gBuffers label_buffers;
	Shader label_shader;

	void get_label_buffer(label_text& lb, float* vertices,
		unsigned int& vertex_index, unsigned int* indices, unsigned int& indices_index);
	glm::vec2 rotate_pt(glm::vec2& rotate_about, glm::vec2 pt, double& rotation_angle);
};

