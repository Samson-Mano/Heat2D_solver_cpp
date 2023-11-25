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

glm::vec3 geom_parameters::getHeatMapColor(float value)
{
	float hsl_h = value * 240;
	const int alpha_i = 120;
	const float hsl_s = 1.0;
	const float hsl_l = 0.5;

	double r = 0;
	double g = 0;
	double b = 0;


	if (hsl_s == 0)
	{
		r = g = b = (hsl_l * 255);
	}
	else
	{
		double v1, v2;
		double hue = hsl_h / 360;

		v2 = (hsl_l < 0.5) ? (hsl_l * (1 + hsl_s)) : ((hsl_l + hsl_s) - (hsl_l * hsl_s));
		v1 = (2 * hsl_l) - v2;

		r = (255 * HueToRGB(v1, v2, hue + (1.0f / 3)));
		g = (255 * HueToRGB(v1, v2, hue));
		b = (255 * HueToRGB(v1, v2, hue - (1.0f / 3)));
	}

	glm::vec3 color = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
	return color;
}

double geom_parameters::HueToRGB(double v1, double v2, double vH)
{
	if (vH < 0)
		vH += 1;

	if (vH > 1)
		vH -= 1;

	if ((6 * vH) < 1)
		return (v1 + (v2 - v1) * 6 * vH);

	if ((2 * vH) < 1)
		return v2;

	if ((3 * vH) < 2)
		return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

	return v1;
}


glm::vec3 geom_parameters::getContourColor_d(float value)
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



double geom_parameters::get_triangle_area(const glm::vec2& pt1, const glm::vec2& pt2, const glm::vec2& pt3)
{
	double x1 = static_cast<double>(pt1.x);
	double y1 = static_cast<double>(pt1.y);
	double x2 = static_cast<double>(pt2.x);
	double y2 = static_cast<double>(pt2.y);
	double x3 = static_cast<double>(pt3.x);
	double y3 = static_cast<double>(pt3.y);

	// Shoelace formula
	double area = 0.5 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

	return area;
}

double geom_parameters::get_line_length(const glm::vec2& pt1, const glm::vec2& pt2)
{
	// Length of line
	double length = std::sqrt(std::pow(pt1.x - pt2.x, 2) + std::pow(pt1.y - pt2.y, 2));

	return length;
}


glm::vec2 geom_parameters::calculateCatmullRomPoint(const std::vector<glm::vec2>& controlPoints, float t)
{
	// Function to calculate a point on a Catmull-Rom spline
	int n = static_cast<int>(controlPoints.size()) - 1;
	int i = static_cast<int>(t * n);
	t = t * n - i;

	glm::vec2 p0 = i > 0 ? controlPoints[i - 1] : controlPoints[0];
	glm::vec2 p1 = controlPoints[i];
	glm::vec2 p2 = controlPoints[i + 1];
	glm::vec2 p3 = (i + 2 < n) ? controlPoints[i + 2] : controlPoints[n];

	return 0.5f * (
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * (t * t * t) +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * (t * t) +
		(-p0 + p2) * t +
		2.0f * p1
		);
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