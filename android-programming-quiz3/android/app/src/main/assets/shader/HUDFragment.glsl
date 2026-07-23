#version 300 es

/**
 * @file HUDFragment.glsl
 * @brief Outputs flat HUD colors without textures or font dependencies.
 * @author Zoey <t.zoey@digipen.edu>
 */

precision mediump float;

in vec4 Color;
out vec4 FragColor;

void main()
{
    FragColor = Color;
}
