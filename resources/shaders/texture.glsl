#shader vertex
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 texCoord;
out vec2 v_TexCoord;

uniform mat4 u_MVP;

void main()
{
   gl_Position = u_MVP * aPos;
   v_TexCoord = texCoord;
};

#shader fragment
#version 460 core
in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
   vec4 texColor = texture(u_Texture, v_TexCoord);
   FragColor = texColor;
};
