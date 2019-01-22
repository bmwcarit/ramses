#version 100

precision highp float;

uniform sampler2D u_texture;

varying vec2 v_tc1;
varying vec3 v_carDiff;
varying vec3 v_carDiffScaled;
uniform float u_lightConeScale;

void main(void)
{
    vec3 color = texture2D(u_texture, v_tc1).rgb;

    float carDist = length(v_carDiff);
    float r2      = carDist;
    float r3      = min(r2, 1.0);

    color = mix(vec3(1, 1, 1), color, r3);

    float r4 = clamp(2.0 - carDist * u_lightConeScale, 0.0, 1.0);
    gl_FragColor = vec4(color * r4, 1.0);
}
