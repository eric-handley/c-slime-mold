#version 430 core

out vec4 FragColor;
	
in vec2 TexCoords;
uniform sampler2D screenTexture;

const float fadeFactor = 0.001;

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

    float totalWeight = 0;
    vec3 sum = vec3(0, 0, 0);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ivec2 xy = center + offsets[i][j];
            sum += texelFetch(screenTexture, xy, 0).rgb * weights[i][j] - vec3(fadeFactor);
            totalWeight += weights[i][j];
        }
    }

    vec3 outColour = sum / totalWeight;

    FragColor = vec4(max(outColour, vec3(0.0)), 1.0);
}