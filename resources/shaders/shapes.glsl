#shader vertex
#version 460 core
layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texId;
out vec4 v_Color;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * aPos;
    v_Color = Color;
};

#shader fragment
#version 460 core
in vec4 v_Color;
out vec4 FragColor;

void main()
{
    FragColor = v_Color;
};
