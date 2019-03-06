#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;

out vec4 FragColor;

struct Material
{
	sampler2D albedo;
	sampler2D specular;
	float shininess;
};

struct Light
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;

vec3 phong(vec3 ambient, vec3 diffuse, vec3 specular, float shininess, bool blinn)
{
	// Ambient
	vec3 a = light.ambient * ambient;

	// Diffuse
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(fNormal, lightDir), 0.0);
	vec3 d = light.diffuse * (diff * diffuse);

	// Specular
	vec3 viewDir = normalize(viewPos - fPosition);
	float spec = 0.0;
	if (blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(fNormal, halfwayDir), 0.0), shininess);
	}
	else
	{
		vec3 reflectDir = reflect(-lightDir, fNormal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	}
	vec3 s = light.specular * (spec * specular);

	return a + d + s;
}

float CalculateSpecular() // view direction
{
    vec3 lightPos = vec3(5.0, 10.0, 1.0);

    vec3 lightDir = normalize(lightPos - fPosition);
	vec3 viewDir = normalize(viewPos - fPosition);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	
	float spec = pow(max(0.0, dot(fNormal, halfwayDir)), shininess);
	return spec;
}

float CalculateReflectance()
{
    vec3 lightPos = vec3(5.0, 10.0, 1.0);

    vec3 lightDir = normalize(lightPos - fPosition);
    
    float cosTheta = dot(lightDir, fNormal);
}


void main()
{
	float specular = CalculateSpecular();
	float diffuse = 1.0 - specular;
	
	// Riemann sum
	float sum = 0.0; // sum of reflected radiance of a point p in direction wo
	vec3 p; // some point
	vec3 wo; // outgoing direction to the viewer
	vec3 n; // surface's normal
	
	int steps = 100;
	float dw = 1.0 / steps;
	
	for (int i = 0; i < steps; ++i)
	{
	     vec3 wi = getNextIncomingLightDir(i); // incoming direction of the light
	     float l = Fr(p, wi, wo) * L(p, wi) * dot(n, wi) * dw; // radiance of p
	     sum += l;
	}
	
	// Cook-Torrance BRDF
	kd = light that gets refracted
	ks = light that gets reflected
	
	/// Diffuse
	vec3 c = diffuse; // albedo or surface color
	flambert = c / pi; // Lambertian diffuse
	
	/// Specular
	fcooktorrance = 
	
	//// Normal distribution function: Trowbridge-Reitz GGX
	
	//// Geometry function: Smith's Schlick-GGX
	
	//// Fresnel equation: Fresnel-Schlick
	
}