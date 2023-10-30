#version 460

in vec3 fPos;
in vec2 fUV;

uniform sampler2D colormap;
uniform sampler2D positiondepthmap;
uniform sampler2D normalmap;
uniform sampler2D lightspacepositionmap;
uniform sampler2D shadowmap;

uniform mat4 inverseProjectionMatrix;
uniform mat4 inverseViewMatrix;

uniform vec3 cameraPos;
uniform vec3 sunDirection;

out vec4 fCol;

float ambientFactor = 0.7;
float shininess = 8.0;
float specularStrength = 0.5;

vec3 lightCol = vec3(1.0, 0.9608, 0.8863);
vec3 ambientCol = vec3(0.9098, 0.9098, 1.0);
float lightPower = 3.0;

const float near = 0.01;
const float far = 100.0;
const float fogDepth = 70.0;

vec3 scatteringMultiplier = vec3(1.0, 0.7, 0.7);

float scatteringFunction(float x) {
	return x*x*x*x*x;
}

void main() {
	vec3 col = texture2D(colormap, fUV).xyz;
	vec4 posdepth = texture2D(positiondepthmap, fUV);
	vec3 fPos = posdepth.xyz;
	float depth = posdepth.w;
	vec3 normal = texture2D(normalmap, fUV).xyz;

	// ----- Calculate directions -----
	vec3 lightDir = normalize(sunDirection);
	vec3 cameraDir;
	vec3 originDir;
	if (normal != vec3(0.0, 0.0, 0.0)) {
		cameraDir = normalize(cameraPos - fPos);
		originDir = normalize(-fPos);

	} else {
		vec4 viewSpaceDirection = -normalize(inverseProjectionMatrix*vec4(fUV.x*2.0-1.0, fUV.y*2.0-1.0, 1.0, 1.0));
		cameraDir = normalize(mat3(inverseViewMatrix) * viewSpaceDirection.xyz);
		originDir = normalize((inverseViewMatrix * viewSpaceDirection).xyz);
	}
	// --------------------------------

	// ----- Shadows -----
	vec3 lightSpace = texture2D(lightspacepositionmap, fUV).xyz;
	lightSpace = lightSpace * 0.5 + 0.5;
	float clipUV = float(1.0 >= lightSpace.x && 0.0 <= lightSpace.x && 1.0 >= lightSpace.y && 0.0 <= lightSpace.y && normal != vec3(0.0));
	
	float cameraDepth = lightSpace.z;
	
	const float maxBias = 0.002;
	const float minBias = 0.001;
	float bias = max(maxBias * (1.0 - dot(normal, lightDir)), minBias);
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowmap, 0);
	for (float x = -1.5; x <= 1.5; ++x) {
		for (float y = -1.5; y <= 1.5; ++y) {
			float lightDepth = texture2D(shadowmap, lightSpace.xy + vec2(x, y)*texelSize).x * clipUV + 1.0 - clipUV;
			shadow += float(lightDepth < cameraDepth - bias);
		}
	}
	shadow /= 16.0;

	float inverseShadow = 1.0 - shadow;
	// -------------------

	// Get the scattered light color due to the suns height
	float sunAtHorizon = 1.0 - abs(lightDir.y);
	vec3 scatteredLightCol = mix(lightCol, lightCol*scatteringMultiplier, scatteringFunction(sunAtHorizon)) * lightPower;

	// Get the ambient light intensity due to the suns height
	float ambientIntensity = min(1.0, max(0.1, lightDir.y+1.0));
	
	// ----- Lighting -----
	vec3 diffuse = max(dot(lightDir, normal), 0.0) * scatteredLightCol;
	vec3 specular = pow(max(dot(cameraDir, reflect(-lightDir, normal)), 0.0), shininess) * scatteredLightCol * specularStrength;
	
	vec3 lighting = col*(diffuse*inverseShadow + ambientCol*ambientIntensity*ambientFactor) + specular*inverseShadow;
	// --------------------

	// ----- Fog -----
	float ndc = depth * 2.0 - 1.0;
	float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
	float fog = 1.0 - smoothstep(0.5, 1.0, max(0.0, min(linearDepth / fogDepth, 1.0)));
	// ---------------

	float fragFacingSun = max(0.0, dot(lightDir, -cameraDir));
	float fragAtHorizon = 1.0 - abs(originDir.y);
	
	vec3 scatteredSkyLightCol = mix(lightCol, lightCol*scatteringMultiplier, scatteringFunction(fragAtHorizon));
	vec3 skyCol = mix(ambientCol*ambientIntensity, scatteredSkyLightCol, smoothstep(0.7, 0.99, fragFacingSun));
	vec3 sun = 2.0*scatteredLightCol * smoothstep(0.9, 1.0, pow(fragFacingSun, 64));

	// ----- Tonemapping -----
	float whitePoint = 1.5*lightPower;
	lighting = (lighting*((lighting/(whitePoint*whitePoint)) + 1.0))/(1.0 + lighting);
	// -----------------------

	vec3 outputCol =
		lighting*(fog) +
		(1.0 - fog)*skyCol*0.9 +
		float(normal == vec3(0.0))*(
			sun +
			skyCol*0.1
		);

	fCol = vec4(outputCol, 1.0);
	// fCol = vec4(vec3(DEPTH), 1.0);
	// if (gl_FragCoord.x > 650)
		// fCol = vec4(vec3(cameraDepth), 1.0);
	// if (gl_FragCoord.x > 650 && gl_FragCoord.y > 500)
	// 	fCol = vec4(vec3(texture2D(shadowmap, (fUV-vec2(0.5))*2.0).x), 1.0);
	// if (gl_FragCoord.x > 650 && gl_FragCoord.y <= 500)
		// fCol = vec4(vec3(texture2D(shadowmap, lightSpace.xy).x), 1.0);
	// if (gl_FragCoord.x <= 650 && gl_FragCoord.y <= 500)
		// fCol = vec4(vec3(shadow), 1.0);
}