uniform sampler2D u_texture;

layout(std140) uniform ImGui
{
    mat4 u_projection;
};

#ifdef VERT_SHADER /*/////////////////////////////////////////////////////////*/

layout (location = 0) in vec2 i_position;
layout (location = 1) in vec2 i_texcoord;
layout (location = 2) in vec4 i_color;

out vec4 v_color;
out vec2 v_texcoord;

void main()
{
    gl_Position = u_projection * vec4(i_position,0,1);
    v_color = i_color;
    v_texcoord = i_texcoord;
}

#endif /* VERT_SHADER ////////////////////////////////////////////////////////*/

#ifdef FRAG_SHADER /*/////////////////////////////////////////////////////////*/

in vec4 v_color;
in vec2 v_texcoord;

out vec4 o_fragcolor;

void main()
{
    o_fragcolor = v_color * texture(u_texture, v_texcoord);
}

#endif /* FRAG_SHADER ////////////////////////////////////////////////////////*/
