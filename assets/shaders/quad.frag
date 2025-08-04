#version 430 core

out vec4 FragColor;
	
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform float fadeFactor;
uniform vec2 mousePos;
uniform int mousePressed;

const ivec2 offsets[3][3] = {
    {ivec2(-1,  1), ivec2(0,  1), ivec2(1,  1)},
    {ivec2(-1,  0), ivec2(0,  0), ivec2(1,  0)},
    {ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1)}
};

const float weights[3][3] = {
    {0.25,  0.50,  0.25},
    {0.50,  0.75,  0.50},
    {0.25,  0.50,  0.25},
};
	
void main() {             
    vec3 texCol = texture(screenTexture, TexCoords).rgb;
    ivec2 center = ivec2(TexCoords * vec2(textureSize(screenTexture, 0)));

    // Calculate local fade factor based on mouse distance
    float localFade = fadeFactor;
    if (mousePressed == 1) {
        vec2 pixelPos = TexCoords * vec2(textureSize(screenTexture, 0));
        float mouseDist = length(pixelPos - mousePos);
        const float eraseRadius = 250.0;
        const float eraseStrength = 10.0;
        
        if (mouseDist < eraseRadius) {
            float eraseMultiplier = 1.0 + eraseStrength * (eraseRadius - mouseDist) / eraseRadius;
            localFade *= eraseMultiplier;
        }
    }
    
    float totalWeight = 0;
    vec3 sum = vec3(0, 0, 0);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ivec2 xy = center + offsets[i][j];
            sum += texelFetch(screenTexture, xy, 0).rgb * weights[i][j] - vec3(localFade);
            totalWeight += weights[i][j];
        }
    }

    vec3 baseColor = max(sum / totalWeight, vec3(0.0));
    
    // Simple bloom: adjust threshold based on fade factor to prevent feedback
    float brightness = dot(baseColor, vec3(0.299, 0.587, 0.114));
    vec3 bloom = vec3(0.0);
    
    // Higher threshold when fade is low to prevent accumulation
    float bloomThreshold = 0.8 + (1.0 - fadeFactor * 2000.0) * 0.15;
    bloomThreshold = clamp(bloomThreshold, 0.8, 0.95);
    
    if (brightness > bloomThreshold) {
        // Extract only the brightest part
        vec3 brightColor = max(baseColor - vec3(bloomThreshold), vec3(0.0));
        bloom = brightColor * 0.2;
    }
    
    vec3 finalColor = baseColor + bloom;
    FragColor = vec4(clamp(finalColor, 0.0, 1.0), 1.0);
}