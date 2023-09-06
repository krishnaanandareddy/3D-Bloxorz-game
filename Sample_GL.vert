#version 330 core

//Reference: https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/2.2.basic_lighting_specular/basic_lighting_specular.cpp

// input data : sent from main program
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 MVP;

uniform mat4 model=mat4(1.0f);
uniform mat4 view;
uniform mat4 projection;
// output data : used by fragment shader
out vec3 fragColor;
out vec3 Normal;

void main ()
{
    vec4 v = vec4(vertexPosition, 1); // Transform an homogeneous 4D vector

    // The color of each vertex will be interpolated
    // to produce the color of each fragment
    fragColor = vertexColor;

    Normal = mat3(transpose(inverse(model))) * aNormal;  

    // Output position of the vertex, in clip space : MVP * position
    gl_Position = MVP * v;
}