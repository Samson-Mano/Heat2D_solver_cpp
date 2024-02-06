#version 330 core

in float v_defl_length;
in float v_transparency;

out vec4 f_Color; // fragment's final color (out to the fragment shader)


vec3 jetHeatmap(float value) 
{

    return clamp(vec3(1.5) - abs(4.0 * vec3(value) + vec3(-3, -2, -1)), vec3(0), vec3(1));
}


void main() 
{
    // Get the input transparency
    float transparency = v_transparency;
    
    /*
    // Calculate transparency based on v_defl_length
    // Contour lines
    if(v_defl_length<0.101 && v_defl_length > 0.099)
    {
       transparency = 1.0f;
    }
    else if(v_defl_length<0.201 && v_defl_length > 0.199)
    {
       transparency = 1.0f;
    }
    else if(v_defl_length<0.301 && v_defl_length > 0.299)
    {
       transparency = 1.0f;
    }
    else if(v_defl_length<0.401 && v_defl_length > 0.399)
    {
       transparency = 1.0f;
    }
    else if(v_defl_length<0.501 && v_defl_length > 0.499)
    {
       transparency = 1.0f;
    }
    else if(v_defl_length<0.601 && v_defl_length > 0.599)
    {
       transparency = 1.0f;
    }
    else if(v_defl_length<0.701 && v_defl_length > 0.699)
    {
       transparency = 1.0f;
    }
    else if(v_defl_length<0.801 && v_defl_length > 0.799)
    {
       transparency = 1.0f;
    }
    else if(v_defl_length<0.901 && v_defl_length > 0.899)
    {
       transparency = 1.0f;
    }
    */

    // Calculate vertex color using jetHeatmap
    vec3 vertexColor = jetHeatmap(v_defl_length);

    // Set the final color
    f_Color = vec4(vertexColor, transparency); // Set the final color
}
