#version 430 core

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform vec3 cameraPos;

in vec3 interpNormal;
in vec3 fragPos;

const float cellevels = 5.0;

void main()
{
	vec3 L = -lightDir;
	vec3 V = normalize(cameraPos - fragPos);
	vec3 N = normalize(interpNormal);
	vec3 R = reflect(-normalize(L), N);

	float diffuse = max(0, dot(N, L));
	diffuse = floor(diffuse * cellevels)/cellevels;  //cel-shading
	
	float specular_pow = 10;
	//specular_pow = floor(specular_pow * cellevels)/cellevels;	////cel-shading
	float specular = pow(max(0, dot(R, V)), specular_pow);
	//specular = floor(specular * cellevels)/cellevels;

	vec3 lightColor =  vec3(1);
	vec3 shadedColor = objectColor * diffuse + lightColor * specular;
	
	float ambient = 0.2;
	gl_FragColor = vec4(mix(objectColor, shadedColor, 1.0 - ambient), 1.0);
}
