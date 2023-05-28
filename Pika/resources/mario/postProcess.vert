#version 130

in vec2 quad_positions;
in vec4 quad_colors;
in vec2 texturePositions;
out vec4 v_color;
out vec2 v_texture;

void main()
{
	gl_Position = vec4(quad_positions, 0, 1);
	v_color = quad_colors;
	v_texture = texturePositions;
}