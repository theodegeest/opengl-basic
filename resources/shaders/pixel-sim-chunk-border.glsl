#shader vertex
#version 460 core
layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texId;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * aPos;
};

#shader fragment
#version 460 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1, 0, 0, 1);
};
