#version 300 es

/**
 * @file GroundFragment.glsl
 * @brief Combines a procedural checkerboard with per-fragment Phong lighting.
 * @author Zoey <t.zoey@digipen.edu>
 */

precision highp float;

in vec3 WorldPosition;
in vec3 WorldNormal;

uniform vec3 LIGHTPOSITION;
uniform vec3 CAMERAPOSITION;

out vec4 FragColor;

void main()
{
    vec2 cell = floor(WorldPosition.xz / 0.8);
    float parity = mod(cell.x + cell.y, 2.0);
    vec3 material = mix(
        vec3(0.075, 0.095, 0.13),
        vec3(0.64, 0.69, 0.76),
        parity);

    vec3 normal = normalize(WorldNormal);
    vec3 lightDirection = normalize(LIGHTPOSITION - WorldPosition);
    vec3 viewDirection = normalize(CAMERAPOSITION - WorldPosition);
    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float diffuseStrength = max(dot(normal, lightDirection), 0.0);
    float specularStrength = diffuseStrength > 0.0
        ? pow(max(dot(viewDirection, reflectionDirection), 0.0), 20.0)
        : 0.0;

    vec3 ambient = 0.22 * material;
    vec3 diffuse = 0.76 * diffuseStrength * material;
    vec3 specular = 0.36 * specularStrength * vec3(1.0);

    // Fade toward the clear colour so the large quad reads as an endless floor.
    float distanceFade = smoothstep(12.0, 24.0, length(WorldPosition.xz));
    vec3 lit = ambient + diffuse + specular;
    lit = mix(lit, vec3(0.025, 0.035, 0.055), distanceFade);
    FragColor = vec4(lit, 1.0);
}
