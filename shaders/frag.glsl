#version 450

uniform float fGlobalTime; // in seconds
uniform vec2 v2Resolution; // viewport resolution (in pixels)

out vec4 fragColor;

void main()
{
    float ar = v2Resolution.y/v2Resolution.x;
    vec2 pos = (gl_FragCoord.xy / v2Resolution.xy - vec2(0.5,0.5)) / vec2(ar, 1.0);
    pos += vec2( cos(fGlobalTime/1000), sin(fGlobalTime/1000) ) / 4;

    vec2 uv1 = step(vec2(-0.1,-0.5), pos) * step(pos, vec2(0.1,0.5)) * smoothstep(0.5, -0.1, abs(pos));
    vec2 uv2 = step(vec2(-0.5/ar,-0.1), pos) * step(pos, vec2(0.5/ar,0.1)) * smoothstep(0.5, -0.1, abs(pos));

    fragColor = mix(vec4(0.0), vec4(0.0,1.0,0.5,0.0), (uv1.x * uv1.y)/2 + (uv2.x * uv2.y)/2);
}
