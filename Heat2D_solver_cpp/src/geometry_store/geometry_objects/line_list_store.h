#pragma once
#include "../geometry_buffers/gBuffers.h"
#include "../geom_parameters.h"

struct line_store
{
	// store the individual point
	int line_id = 0;
	glm::vec2 line_startpt_loc = glm::vec2(0);
	glm::vec2 line_endpt_loc = glm::vec2(0);

	glm::vec2 line_startpt_offset = glm::vec2(0);
	glm::vec2 line_endpt_offset = glm::vec2(0);

	glm::vec3 line_startpt_color = glm::vec3(0);
	glm::vec3 line_endpt_color = glm::vec3(0);

	bool is_offset = false;
};

class line_list_store
{
public:
	geom_parameters* geom_param_ptr = nullptr;
	unsigned int line_count = 0;
	std::vector<line_store> lineMap;

	line_list_store();
	~line_list_store();
	void init(geom_parameters* geom_param_ptr);
	void add_line(int& line_id, glm::vec2& line_startpt_loc,glm::vec2& line_endpt_loc,
		glm::vec2 line_startpt_offset, glm::vec2 line_endpt_offset, 
		glm::vec3& line_startpt_color, glm::vec3& line_endpt_color, bool is_offset);
	void set_buffer();
	void paint_lines();
	void clear_lines();
	void update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);
private:
	gBuffers line_buffer;
	Shader line_shader;

	void get_line_buffer(line_store& ln, float* line_vertices, unsigned int& line_v_index, unsigned int* line_vertex_indices, unsigned int& line_i_index);

};

