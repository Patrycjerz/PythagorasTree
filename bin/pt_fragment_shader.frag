#version 150

uniform vec3 first_color;
uniform vec3 last_color;
uniform bool is_directed_light;
uniform float ratio;
uniform vec3 light_direction;

in vec4 inout_normal;

out vec3 out_color;

void main()
{
	vec3 object_color = mix(last_color, first_color, ratio);

	if (is_directed_light)
	{
		float light_cos = -dot(normalize(light_direction), normalize(inout_normal.xyz));
		vec3 diffuse = vec3(1.0, 1.0, 1.0) * (light_cos > 0.0 ? light_cos : 0.0);
		out_color = object_color * 0.7 * (object_color + diffuse);
	}
	else
		out_color = object_color;
}