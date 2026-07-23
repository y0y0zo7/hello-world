#version 300 es

/**
 * @file HUDVertex.glsl
 * @brief Projects pixel-space HUD vertices with their per-vertex colors.
 * @author Zoey <t.zoey@digipen.edu>
 */

precision highp float;

layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec4 VertexColor;

uniform mat4 PROJECTIONMATRIX;

out vec4 Color;

void main()
{
    Color = VertexColor;
    gl_Position = PROJECTIONMATRIX * vec4(VertexPosition, 0.0, 1.0);
}
