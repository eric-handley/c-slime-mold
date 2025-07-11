#version 430 core

out vec4 FragColor;
	
in vec2 TexCoords;
uniform sampler2D screenTexture;

const float fadeFactor = 0.0005;

const ivec2 offsets[3][3] = {
    {ivec2(-1,  1), ivec2(0,  1), ivec2(1,  1)},
    {ivec2(-1,  0), ivec2(0,  0), ivec2(1,  0)},
    {ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1)}
};

const float weights[3][3] = {
    {0.50,  0.75,  0.50},
    {0.75,  1.00,  0.75},
    {0.50,  0.75,  0.50},
};
	
void main() {             
    vec3 texCol = texture(screenTexture, TexCoords).rgb;
    ivec2 center = ivec2(TexCoords * vec2(textureSize(screenTexture, 0)));

    float div = 0;
    vec3 avg = vec3(0,0,0);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ivec2 xy = center + offsets[i][j];
            avg += texelFetch(screenTexture, xy, 0).rgb * weights[i][j];
            div += weights[i][j];
        }
    }

    avg /= div;
    vec3 faded = max(avg - vec3(fadeFactor), 0.0);

    FragColor = vec4(faded, 1.0);
}