#version 100

precision highp float;

uniform highp mat4 mvpMatrix;

attribute vec2 a_position;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;

void main()
{
    gl_Position = mvpMatrix * vec4(a_position, 0.0, 1.0);
    v_texcoord = a_texcoord;    
}
