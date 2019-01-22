#version 100

precision highp float;

uniform highp mat4 u_mvpMatrix;

attribute vec3 a_position;

void main()
{           
    gl_Position = u_mvpMatrix * vec4(a_position, 1.0);    
}
