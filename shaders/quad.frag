#version 430 core

out vec4 FragColor;
	
in vec2 TexCoords;
uniform sampler2D screenTexture;

const float fadeFactor = 0.00025;
	
void main() {             
    vec3 texCol = texture(screenTexture, TexCoords).rgb;
    texCol = vec3(
        texCol.r - fadeFactor, 
        texCol.g - fadeFactor, 
        texCol.b - fadeFactor
    );
    FragColor = vec4(texCol, 1.0);
}