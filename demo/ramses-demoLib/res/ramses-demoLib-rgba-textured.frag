#version 300 es

precision highp float;

uniform sampler2D textureSampler;
uniform vec4 u_color;

in vec2 v_texcoord;
out vec4 fragmentColor;

void main(void)
{
    fragmentColor = texture(textureSampler, v_texcoord);
    fragmentColor.a *= u_color.a;
}
