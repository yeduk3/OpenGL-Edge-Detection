#version 410 core

in vec3 worldPosition;
in vec3 normal;

uniform sampler2D renderTex;
uniform float edgeThreshold;
uniform int width;
uniform int height;

// Render Pass
subroutine vec4 renderPassType();
subroutine uniform renderPassType renderPass;

// Phong
uniform vec3 lightPosition;
uniform vec3 eyePosition;
uniform vec3 lightColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

out vec4 fragColor;

vec3 phongModel(vec3 pos, vec3 norm)
{
vec3 l = lightPosition - pos;
vec3 L = normalize(l);
vec3 N = normalize(norm);
vec3 R = 2 * dot(L, N) * N - L;
vec3 V = normalize(eyePosition - worldPosition);
vec3 I = lightColor / dot(l, l);

vec3 ambient = diffuseColor * vec3(0.02);
vec3 diffuse = I * diffuseColor * max(dot(L, N), 0);
vec3 specular = I * specularColor * pow(max(dot(R, V), 0), shininess);

vec3 color = ambient + diffuse + specular;

color = pow(color, vec3(1 / 2.2));

return color;
}

float luma(vec3 color)
{
return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

// Pass #1
subroutine(renderPassType) vec4 pass1()
{
return vec4(phongModel(worldPosition, normal), 1.0);
}

// Pass #2
subroutine(renderPassType) vec4 pass2()
{
float dx = 1.0 / float(width);
float dy = 1.0 / float(height);
vec2 texCoord = gl_FragCoord.xy * vec2(dx, dy);
float s00 = luma(texture(renderTex, texCoord + vec2(- dx, dy)).rgb);
float s10 = luma(texture(renderTex, texCoord + vec2(- dx, 0.0)).rgb);
float s20 = luma(texture(renderTex, texCoord + vec2(- dx, - dy)).rgb);
float s01 = luma(texture(renderTex, texCoord + vec2(0.0, dy)).rgb);
float s21 = luma(texture(renderTex, texCoord + vec2(0.0, - dy)).rgb);
float s02 = luma(texture(renderTex, texCoord + vec2(dx, dy)).rgb);
float s12 = luma(texture(renderTex, texCoord + vec2(dx, 0.0)).rgb);
float s22 = luma(texture(renderTex, texCoord + vec2(dx, - dy)).rgb);

// sobel filter
float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);
float dist = pow(sx, 2) + pow(sy, 2);

if (dist > edgeThreshold) return vec4(1.0);
else return vec4(0.0, 0.0, 0.0, 1.0);
}

void main()
{
fragColor = renderPass();
}