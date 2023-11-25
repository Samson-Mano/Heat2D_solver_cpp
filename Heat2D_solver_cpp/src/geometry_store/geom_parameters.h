#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <sstream>
#include "geometry_buffers/font_atlas.h"
#include <chrono>

struct geom_color_theme
{
	glm::vec3 background_color = glm::vec3(0);
	glm::vec3 node_color = glm::vec3(0);
	glm::vec3 selection_color = glm::vec3(0);
	glm::vec3 line_color = glm::vec3(0);
	glm::vec3 constraint_color = glm::vec3(0);
	glm::vec3 triangle_color = glm::vec3(0);
};

struct material_data
{
	unsigned int material_id = 0;
	std::string material_name = "";
	double thermal_conductivity_kx = 0.0;
	double thermal_conductivity_ky = 0.0;
	double element_thickness = 0.0;
};

class Stopwatch
{
public:
	void reset_time();
	double current_elapsed() const;

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime = std::chrono::high_resolution_clock::time_point();
	// std::chrono::time_point<std::chrono::high_resolution_clock> m_endTime;
};


class geom_parameters
{
public:
	// Standard sizes
	const float font_size = static_cast<float>(12.0f * std::pow(10, -5));
	const float node_circle_radii = 0.005f;

	// Precision for various values
	const int length_precision = 2;
	const int coord_precision = 2;
	const int constraint_precision = 2;
	
	// Triangle mesh shrunk factor
	const double traingle_shrunk_factor = 0.8;

	// File path
	std::filesystem::path resourcePath = "";

	// Window size
	int window_width = 0;
	int window_height = 0;

	glm::vec2 min_b = glm::vec2(0); // (min_x, min_y)
	glm::vec2 max_b = glm::vec2(0); // (max_x, max_y)
	glm::vec2 geom_bound = glm::vec2(0); // Bound magnitude
	glm::vec2 center = glm::vec2(0); // center of the geometry
	glm::mat4 modelMatrix = glm::mat4(0); // Geometry model matrix
	double geom_scale = 0.0; // Scale of the geometry
	double geom_transparency = 0.0; // Value to control the geometry transparency
	double normalized_defl_scale = 0.0f; // Value of deflection scale
	double defl_scale = 0.0f; // Value of deflection scale

	// Screen transformations
	glm::mat4 panTranslation = glm::mat4(0); // Pan translataion
	double zoom_scale = 0.0; // Zoom scale

	// Standard colors
	geom_color_theme geom_colors;

	font_atlas main_font;

	geom_parameters();
	~geom_parameters();
	void init();

	static bool isPointInsideRectangle(const glm::vec2& rect_cpt1, const glm::vec2& rect_cpt2, const glm::vec2& pt);

	static void copyNodenumberlistToCharArray(const std::vector<int>& vec, char* charArray, size_t bufferSize);

	static glm::vec3 get_standard_color(int color_index);

	static glm::vec2 linear_interpolation(const glm::vec2& pt1, const glm::vec2& pt2, const double& param_t);

	static	glm::vec2 findGeometricCenter(const std::vector<glm::vec2>& all_pts);

	static std::pair<glm::vec2, glm::vec2> findMinMaxXY(const std::vector<glm::vec2>& all_pts);

	static glm::vec3 getHeatMapColor(float value);

	static glm::vec3 getContourColor_d(float value);

	static double get_triangle_area(const glm::vec2& pt1, const glm::vec2& pt2, const glm::vec2& pt3);

	static double get_line_length(const glm::vec2& pt1, const glm::vec2& pt2);

	static glm::vec2 calculateCatmullRomPoint(const std::vector<glm::vec2>& controlPoints, float t);


private:
	static double HueToRGB(double v1, double v2, double vH);

};



