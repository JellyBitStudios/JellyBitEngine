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

// Approximates the relative surface area of microfacets exactly aligned to the halfway vector
// n: surface normal
// h: halfway vector
// a: surface's roughness
float DistributionTrowbridgeReitzGGX(vec3 n, vec3 h, float a)
{
    float a2 = a * a;
    float nhDot = max(0.0, dot(n, h));
    float nhDot2 = nhDot * nhDot;
    
    float nom = a2;
    float denom = nhDot2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    
    return nom / denom;
}

// Approximates the relative surface area where its micro surface-details overSHADOW each other causing light rays to be occluded
float GeometrySchlickGGX(float nvdot, float k)
{
    float nom = nvDot;
    float denom = nvDot * (1.0 - k) + k;
    
    return nom / denom;
}

// n: surface normal
// v: view direction (geometry obstruction)
// l: light direction (geometry shadowing)
// k: remapping of a
float GeometrySmith(vec3 n, vec3 v, vec3 l, float k)
{
    float nvDot = max(0.0, dot(n, v));
    float nlDot = max(0.0, dot(n, l));
    
    float ggx1 = GeometrySchlickGGX(nvDot, k);
    float ggx2 = GeometrySchlickGGX(nlDot, k);
    
    return ggx1 * ggx2;
}

// Ratio of light that gets REFLECTED over the light that gets refracted
// Fresnel: 90 degree: reflections become much more apparent
// f0: base reflectivity of the surface (at a 0 degree angle as if looking directly onto a surface)
  // view direction
 
vec3 FresnelSchlick(float cosTheta, vec3 f0) // view direction
{
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
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
	float kd = light that gets refracted
	float ks = light that gets reflected
	
	/// Diffuse
	vec3 c = diffuse; // albedo or surface color
	flambert = c / PI; // Lambertian diffuse
	
	/// Specular
	fcooktorrance = 
	
	//// Normal distribution function: Trowbridge-Reitz GGX
	DistributionGGX()
	
	//// Geometry function: Smith's Schlick-GGX
	GeometrySmith()	
	
	//// Fresnel equation: Fresnel-Schlick
	vec3 viewDir = normalize(viewPos - fPosition);    
    float cosTheta = dot(viewDir, fNormal);
	
	vec3 f0 = vec3(0.04); // base reflectivity for most dielectrics
    f0 = mix(f0, albedo.rgb, metalness);

    ks = FresnelSchlick(cosTheta, f0);
}