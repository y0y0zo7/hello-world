#version 300 es

precision mediump float;

in float VarShade;
uniform vec3 PARTCOLOR;

out vec4 FragColor;

void main()
{
    FragColor = vec4(PARTCOLOR * VarShade, 1.0);
}
