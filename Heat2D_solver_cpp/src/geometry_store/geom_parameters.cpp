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
	geom_colors.background_color = glm::vec3(0.62f, 0.62f, 0.62f); // White
	geom_colors.selection_color = glm::vec3(0.862745f, 0.078431f, 0.23529f); // Crimson
	geom_colors.constraint_color = glm::vec3(0.0f, 0.50196f, 0.0f); // Green

	// Theme 1 
	geom_colors.node_color = glm::vec3(0.54509f, 0.0f, 0.4f); // Dark Red
	geom_colors.line_color = glm::vec3(1.0f, 0.54901f, 0.6f); // Dark Orange
	geom_colors.triangle_color = glm::vec3(0.90196f, 0.90196f, 0.98039f); // Lavender

	//// Theme 2
	//geom_colors.node_color = glm::vec3(135.0f / 255.0f, 206.0f / 255.0f, 250.0f / 255.0f); // Sky Blue
	//geom_colors.line_color = glm::vec3(47.0f / 255.0f, 79.0f / 255.0f, 79.0f / 255.0f); // Dark Slate Gray
	//geom_colors.triangle_color = glm::vec3(240.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f); // Light Coral

	//// Theme 3
	//geom_colors.node_color = glm::vec3(144.0f / 255.0f, 238.0f / 255.0f, 144.0f / 255.0f); // Light Green
	//geom_colors.line_color = glm::vec3(105.0f / 255.0f, 105.0f / 255.0f, 105.0f / 255.0f); // Dim Gray
	//geom_colors.triangle_color = glm::vec3(147.0f / 255.0f, 112.0f / 255.0f, 219.0f / 255.0f); // Medium Purple

	//// Theme 4
	//geom_colors.node_color = glm::vec3(255.0f / 255.0f, 215.0f / 255.0f, 0.0f); // Gold
	//geom_colors.line_color = glm::vec3(85.0f / 255.0f, 107.0f / 255.0f, 47.0f / 255.0f); // Dark Olive Green
	//geom_colors.triangle_color = glm::vec3(218.0f / 255.0f, 112.0f / 255.0f, 214.0f / 255.0f); // Orchid


	/*
	// Initialize the color theme
	geom_colors.background_color = glm::vec3(0.62f, 0.62f, 0.62f); 
	geom_colors.node_color = glm::vec3(0.0f, 0.0f, 0.4f); 
	geom_colors.selection_color = glm::vec3(0.8039f, 0.3608f, 0.3608f); 

	geom_colors.line_color = glm::vec3(0.0f, 0.2f, 0.6f); 
	geom_colors.constraint_color = glm::vec3(0.0f, 0.1f, 0.0f); 
	
	// Traingle mesh
	geom_colors.triangle_color = glm::vec3(0.82f, 0.77f, 0.92f); 
	*/
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
	for (const auto& pt : all_pts)
	{
		sum += pt;
	}
	return sum / static_cast<float>(all_pts.size());
}


std::pair<glm::vec2, glm::vec2> geom_parameters::findMinMaxXY(const std::vector<glm::vec2>& all_pts)
{
	if (static_cast<int>(all_pts.size()) < 1)
	{
		// Null input
		return {glm::vec2(0),glm::vec2(0)};
	}

	// Initialize min and max values to first node in map
	glm::vec2 firstNode = all_pts[0];
	glm::vec2 minXY = glm::vec2(firstNode.x, firstNode.y);
	glm::vec2 maxXY = minXY;

	// Loop through all nodes in map and update min and max values
	for (const auto& pt : all_pts)
	{
		if (pt.x < minXY.x)
		{
			minXY.x = pt.x;
		}
		if (pt.y < minXY.y)
		{
			minXY.y = pt.y;
		}
		if (pt.x > maxXY.x)
		{
			maxXY.x = pt.x;
		}
		if (pt.y > maxXY.y)
		{
			maxXY.y = pt.y;
		}
	}

	// Return pair of min and max values
	return { minXY, maxXY };
}


glm::vec3 geom_parameters::getContourColor(float value)
{
	// return the contour color based on the value (0 to 1)
	glm::vec3 color;
	float r, g, b;

	// Rainbow color map
	float hue = value * 5.0f; // Scale the value to the range of 0 to 5
	float c = 1.0f;
	float x = c * (1.0f - glm::abs(glm::mod(hue / 2.0f, 1.0f) - 1.0f));
	float m = 0.0f;

	if (hue >= 0 && hue < 1) {
		r = c;
		g = x;
		b = m;
	}
	else if (hue >= 1 && hue < 2) {
		r = x;
		g = c;
		b = m;
	}
	else if (hue >= 2 && hue < 3) {
		r = m;
		g = c;
		b = x;
	}
	else if (hue >= 3 && hue < 4) {
		r = m;
		g = x;
		b = c;
	}
	else {
		r = x;
		g = m;
		b = c;
	}

	color = glm::vec3(r, g, b);
	return color;
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