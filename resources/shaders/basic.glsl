#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
out vec3 vertexColor;
void main()
{
   gl_Position = vec4(aPos, 1.0);
   vertexColor = aPos; // Pass position to the fragment shader
};

#shader fragment
#version 460 core
in vec3 vertexColor;
out vec4 FragColor;

uniform vec4 u_Color;

void main()
{
   // Map the position to a color
   // FragColor = vec4(vertexColor * 0.5 + 0.5, 1.0); // Color based on position
   FragColor = u_Color;
};
