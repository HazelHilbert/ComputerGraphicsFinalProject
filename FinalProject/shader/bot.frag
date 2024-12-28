#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 TexCoord;

out vec4 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform sampler2D diffuseTexture;

void main()
{
	float diff = max(dot(normalize(worldNormal), normalize(lightPosition)*vec3(-1)), 0.0);
	vec3 diffuse = vec3(diff);
	vec3 ambient = vec3(0.5);

	// 2. Sample the diffuse texture
	vec3 texureColor = texture(diffuseTexture, TexCoord).rgb;

	// 3. Combine texture color with your lighting
	// Tone mapping
	//v = v / (1.0 + v);

	// Final color (with naive multiply for diffuse lighting)
	//vec3 litColor = baseColor * v;

	// Optional gamma correction
	//litColor = pow(litColor, vec3(1.0 / 2.2));

	// Output as a vec4
	finalColor = vec4(ambient * texureColor, 1.0);
}
