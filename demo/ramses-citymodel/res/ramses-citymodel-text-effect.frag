#version 100

precision highp float;

uniform sampler2D u_texture;
uniform float u_a;

varying vec2 v_texcoord;

void main(void)
{
    float a = texture2D(u_texture, v_texcoord).r;
    gl_FragColor = vec4(1.0, 1.0, 1.0, a * u_a);
}
