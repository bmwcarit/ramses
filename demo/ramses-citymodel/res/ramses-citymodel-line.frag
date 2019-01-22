#version 100

precision highp float;

varying vec3 v_distance;

/*
EVector3f interpolatedDistance(paramVector3f("interpolatedDistance"));

EVector3f scale = vec3(0.1f, 1.0f, 10.0f);

EFloat dy = max(interpolatedDistance.y(), -1.0 * interpolatedDistance.y());
EFloat dy2 = max(dy - interpolatedDistance.z(), 0.0);

EVector3f d = vec3(interpolatedDistance.x(), dy2, 0);
EFloat a1 = scale.y() - length(d);

EFloat a2 = clamp(a1, 0.0, 1.0);

EFloat a3 = 1.0 - a2 * a2;

EVector4f finalColor = vec4(0,0,0, a3);
setOutput(finalColor);
*/

void main(void)
{
    float dy = abs(v_distance.y);
    float dy2 = max(dy - v_distance.z, 0.0);

    vec2 d = vec2(v_distance.x, dy2);
    float a2 =clamp(1.0 - length(d), 0.0, 1.0);
    float a3 = 1.0 - a2 * a2;

    gl_FragColor = vec4(0.0, 0.0, 0.0, a3);
}
