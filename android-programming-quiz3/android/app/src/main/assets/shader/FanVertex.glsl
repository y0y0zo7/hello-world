#version 300 es

/**
 * @file FanVertex.glsl
 * @brief Transforms fan positions and inverse-transpose-corrected normals.
 * @author Zoey <t.zoey@digipen.edu>
 */

precision highp float;

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 WorldPosition;
out vec3 WorldNormal;

void main()
{
    vec4 world = MODELMATRIX * vec4(VertexPosition, 1.0);
    WorldPosition = world.xyz;
    // The inverse-transpose includes every fan-part scale and blade rotation.
    WorldNormal = normalize(NORMALMATRIX * VertexNormal);
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(VertexPosition, 1.0);
}
