#shader vertex
#version 460 core
layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texId;
out vec2 v_TexCoord;
out float v_TexId;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * aPos;
    v_TexCoord = texCoord;
    v_TexId = texId;
};

#shader fragment
#version 460 core
in vec2 v_TexCoord;
in float v_TexId;
out vec4 FragColor;

uniform sampler2D u_Texture[2];

void main()
{
    int index = int(v_TexId);
    vec4 texColor = texture(u_Texture[index], v_TexCoord);
    FragColor = texColor;
};
