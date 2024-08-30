#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;
out vec3 vertexColor;
out vec2 v_TexCoord;
void main()
{
   gl_Position = vec4(aPos, 1.0);
   vertexColor = aPos; // Pass position to the fragment shader
   v_TexCoord = texCoord;
};

#shader fragment
#version 460 core
in vec3 vertexColor;
in vec2 v_TexCoord;
out vec4 FragColor;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
   // Map the position to a color
   // FragColor = vec4(vertexColor * 0.5 + 0.5, 1.0); // Color based on position
   vec4 texColor = texture(u_Texture, v_TexCoord);
   FragColor = texColor;
};
