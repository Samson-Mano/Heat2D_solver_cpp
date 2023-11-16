#include "geom_parameters.h"

geom_parameters::geom_parameters()
{
	// Empty Constructor
}

geom_parameters::~geom_parameters()
{
	// Empty Destructor
}

void geom_parameters::init()
{
	// Initialize the paramters
	resourcePath = std::filesystem::current_path();
	std::cout << "Current path of application is " << resourcePath << std::endl;

	// Create the font atlas
	main_font.create_atlas(resourcePath);

	// Initialize the color theme
	geom_colors.background_color = glm::vec3(0.62f, 0.62f, 0.62f);
	geom_colors.node_color = glm::vec3(0.0f, 0.0f, 0.4f);
	geom_colors.node_selected_color = glm::vec3(0.8039f, 0.3608f, 0.3608f);

	geom_colors.line_color = glm::vec3(0.0f, 0.2f, 0.6f);
	geom_colors.constraint_color = glm::vec3(0.6f, 0.0f, 0.6f);
	geom_colors.load_color = glm::vec3(0.0f, 1.0f, 0.0f);
	geom_colors.ptmass_color = glm::vec3(0.82f, 0.77f, 0.92f);
	geom_colors.inlcond_displ_color = glm::vec3(0.96f, 0.5f, 0.1f);
	geom_colors.inlcond_velo_color = glm::vec3(0.54f, 0.06f, 0.31f);

	// Traingle mesh
	geom_colors.triangle_color = glm::vec3(0.82f, 0.77f, 0.92f);
	geom_colors.triangle_boundary = geom_colors.triangle_color * 0.8f;
	geom_colors.triangle_node = geom_colors.triangle_color * 0.6f;
}


bool geom_parameters::isPointInsideRectangle(const glm::vec2& rect_cpt1, const glm::vec2& rect_cpt2, const glm::vec2& pt)
{
	return (pt.x >= std::min(rect_cpt1.x, rect_cpt2.x) &&
		pt.x <= std::max(rect_cpt1.x, rect_cpt2.x) &&
		pt.y >= std::min(rect_cpt1.y, rect_cpt2.y) &&
		pt.y <= std::max(rect_cpt1.y, rect_cpt2.y));
}

glm::vec2 geom_parameters::linear_interpolation(const glm::vec2& pt1, const glm::vec2& pt2, const double& param_t)
{
	return glm::vec2(pt1.x * (1.0 - param_t) + (pt2.x * param_t),
					 pt1.y * (1.0 - param_t) + (pt2.y * param_t));

}

void geom_parameters::copyNodenumberlistToCharArray(const std::vector<int>& vec, char* charArray, size_t bufferSize)
{
	// Use std::ostringstream to build the comma-separated string
	std::ostringstream oss;
	for (size_t i = 0; i < vec.size(); ++i)
	{
		if (i > 0)
		{
			oss << ", "; // Add a comma and space for each element except the first one
		}
		oss << vec[i];
	}

	// Copy the resulting string to the char array
	std::string resultString = oss.str();

	if (resultString.size() + 1 > bufferSize)
	{
		// Truncate 15 character
		resultString.erase(bufferSize - 16);
		resultString += "..exceeds limit";
	}

	strncpy_s(charArray, bufferSize, resultString.c_str(), _TRUNCATE);

}


glm::vec3 geom_parameters::get_standard_color(int color_index)
{
	// Red, Green, Blue, Yellow, Magenta, Cyan, Orange, Purple, Lime, Pink
	static const std::vector<glm::vec3> colorSet = {
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 0.5f, 0.0f),
			glm::vec3(0.5f, 0.0f, 1.0f),
			glm::vec3(0.5f, 1.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.5f)
	};

	int index = color_index % colorSet.size();
	return colorSet[index];
}


glm::vec2 geom_parameters::findGeometricCenter(const std::vector<glm::vec2>& all_pts)
{
	// Function returns the geometric center of the nodes
		// Initialize the sum with zero
	glm::vec2 sum(0);

	// Sum the points
	for (auto it = all_pts.begin(); it != all_pts.end(); ++it)
	{
		sum += it;
	}
	return sum / static_cast<float>(all_pts.size());
}


// Stop watch
void Stopwatch::reset_time()
{
	m_startTime = std::chrono::high_resolution_clock::now();
}

double Stopwatch::current_elapsed() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_startTime).count() / 1000.0;
}