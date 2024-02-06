#pragma once
#include "../geometry_buffers/gBuffers.h"
#include "../geom_parameters.h"

struct dynamic_tri_store
{
	// store the individual point
	int tri_id = 0;

	// Location
	glm::vec2 tript1_loc = glm::vec2(0);
	glm::vec2 tript2_loc = glm::vec2(0);
	glm::vec2 tript3_loc = glm::vec2(0);

	// Normalized magnitude length
	float tript1_mag_len = 0;
	float tript2_mag_len = 0;
	float tript3_mag_len = 0;

};


class dynamic_tri_list_store
{
public:
	geom_parameters* geom_param_ptr = nullptr;
	unsigned int tri_count = 0;
	std::vector<dynamic_tri_store> dyn_triMap;

	dynamic_tri_list_store();
	~dynamic_tri_list_store();


	void init(geom_parameters* geom_param_ptr);
	void add_tri(int& tri_id, const glm::vec2& tript1_loc, const glm::vec2& tript2_loc, const glm::vec2& tript3_loc,
		float tript1_mag_len, float tript2_mag_len, float tript3_mag_len);
	void set_buffer();
	void paint_triangles();
	void clear_triangles();
	void update_opengl_uniforms(bool set_modelmatrix, bool set_pantranslation, bool set_zoomtranslation, bool set_transparency, bool set_deflscale);
private:
	gBuffers tri_buffer;
	Shader tri_shader;

	void get_tri_buffer(dynamic_tri_store& dyn_tri, float* tri_vertices, unsigned int& tri_v_index, unsigned int* tri_vertex_indices, unsigned int& tri_i_index);

};
