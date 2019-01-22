#version 300 es

precision highp float;
uniform highp mat4 mvpMatrix;

in vec2 a_position;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
    gl_Position = mvpMatrix * vec4(a_position, 0.0, 1.0);
    v_texcoord = a_texcoord;
}

