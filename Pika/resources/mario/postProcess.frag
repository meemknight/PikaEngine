#version 330

out vec4 color;
in vec4 v_color;
in vec2 v_texture;
uniform sampler2D u_sampler;
void main()
{

	vec2 newSamplePoint = v_texture;
	newSamplePoint *= 2;
	newSamplePoint -= 1;

	vec2 s = sign(newSamplePoint);
	newSamplePoint = abs(newSamplePoint);
	newSamplePoint = 
	mix(pow(newSamplePoint, vec2(1.2)) * abs(cos(newSamplePoint*3.141592*2.f)), newSamplePoint, 
	min(distance(newSamplePoint, vec2(0.2,0.2))/0.2*sqrt(2.f),1));

	newSamplePoint *= s;

	newSamplePoint += 1;
	newSamplePoint /= 2;
	color = v_color * texture2D(u_sampler, newSamplePoint);

	//color.rgb = vec3(length(color.rgb));

	color = texture2D(u_sampler, v_texture);
	color.a = 1;
}