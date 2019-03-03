#version 330 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fColor;
in vec2 fTexCoord;

out vec4 FragColor;

struct Material
{
	sampler2D albedo;
};

struct Light
{
	vec3 direction;
	vec3 intensity;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;

uniform int levels;

vec3 toonShade(vec3 ambient, vec3 diffuse, float scaleFactor)
{
    vec3 normal = normalize(fNormal);
    vec3 lightDir = normalize(-light.direction);
    vec3 reflectDir = normalize(reflect(normal, -lightDir));
    vec3 viewDir = normalize(fPosition - viewPos);
       
    float cosine = max(0.0, dot(reflectDir, normal));

    vec3 diffuseColor = diffuse * floor(cosine * levels) * scaleFactor;
    
    vec3 toonColor = light.intensity * (ambient + diffuseColor);
    //toonColor = mix(toonColor, vec3(1.0, 1.0, 1.0), spec);
    
    return toonColor;
}

void main()
{
    float scaleFactor = 1.0 / levels;

	vec4 albedo = texture(material.albedo, fTexCoord);
	if (albedo.a < 0.1)
		discard;

	vec3 diffuse = vec3(albedo);
	vec3 ambient = diffuse;
	vec3 toon = toonShade(ambient, diffuse, scaleFactor);
	FragColor = vec4(toon, albedo.a);
}