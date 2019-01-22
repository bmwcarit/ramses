#version 100

precision highp float;

varying float v_brightness;
uniform vec4 u_color;

void main(void)
{
    gl_FragColor = vec4(v_brightness * u_color.rgb, u_color.a);
}
