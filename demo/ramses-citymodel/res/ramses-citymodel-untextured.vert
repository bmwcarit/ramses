#version 100

precision highp float;

uniform highp mat4 u_mvpMatrix;
uniform highp mat4 u_mvMatrix;

attribute vec3 a_position;
attribute vec3 a_normal;

varying float v_brightness;

void main()
{
    const vec3 light_dir = vec3(0.58, 0.58, 0.58);
    const float ambientBrightness = 0.5 * 0.3;

    gl_Position = u_mvpMatrix * vec4(a_position, 1.0);

    vec3 normal_eye = mat3(u_mvMatrix) * a_normal;
    float ndotl = max(0.0, dot(light_dir, normalize(normal_eye)));
    v_brightness = ambientBrightness + ndotl;
}
