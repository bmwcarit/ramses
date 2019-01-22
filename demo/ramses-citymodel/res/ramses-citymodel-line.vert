#version 100

precision highp float;

uniform highp mat4 u_mvpMatrix;

attribute vec3 a_position;
attribute vec3 a_normal;

varying vec3 v_distance;

void main()
{
    gl_Position = u_mvpMatrix * vec4(a_position, 1.0);
    
    float dotp = a_normal.x + a_normal.y;
    
    float absp = abs(a_normal.x);
    
    v_distance = vec3(a_normal.z, dotp, absp);
}
