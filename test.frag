
#version 410 core

uniform sampler2D texture;

in vec2 texCoord;

out vec4 fragColor;

void main()
{
            // fragColor = texture(texture, texCoord);
    fragColor = vec4(1, 0, 0, 1);
}