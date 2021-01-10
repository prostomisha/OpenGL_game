#version 430 core

uniform sampler2D textureSampler;
uniform vec3 lightDir;

in vec3 interpNormal;
in vec2 interpTexCoord;

float cellevels = 3.0;

void main()
{
	vec2 modifiedTexCoord = vec2(interpTexCoord.x, 1.0 - interpTexCoord.y); // Poprawka dla tekstur Ziemi, ktore bez tego wyswietlaja sie 'do gory nogami'
	vec3 color = texture(textureSampler, modifiedTexCoord).rgb;
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);

	float amount = (color.r+color.g+color.b)/3.0;
	diffuse = floor(diffuse * cellevels)/cellevels;
	//diffuse *= amount;

	//color.rgb = vec3(mix(color.r, color.g, color.b));

	color = amount * normal;

	gl_FragColor = vec4(color * diffuse, 1.0);
}
