#version 100

precision highp float;

uniform highp mat4 u_mvpMatrix;
uniform vec3 u_carPos;

attribute vec3 a_position;
attribute vec2 a_texcoord;

varying vec2 v_tc1;
varying vec3 v_carDiff;

void main()
{
    gl_Position = u_mvpMatrix * vec4(a_position, 1.0);
        
    v_tc1 = a_texcoord;
    v_carDiff = (a_position - u_carPos) / 10.0;
}
