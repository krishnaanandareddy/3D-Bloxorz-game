#version 330 core


//Reference: https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/2.2.basic_lighting_specular/basic_lighting_specular.cpp

// Interpolated values from the vertex shaders
in vec3 fragColor;
in vec3 Normal;  

// output data
out vec3 color;

uniform vec3 lightColor=vec3(1.0, 1.0, 1.0);
uniform vec3 lightPos=vec3(0, 40, 0); 
uniform vec3 viewPos=vec3(3, 3, 3); 

void main()
{
    // ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

   
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - color);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    
        
    vec3 result = (ambient+diffuse) *  fragColor;
    color = result;
}
