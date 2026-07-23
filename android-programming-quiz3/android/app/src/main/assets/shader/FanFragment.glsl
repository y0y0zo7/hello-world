#version 300 es

/**
 * @file FanFragment.glsl
 * @brief Applies per-fragment ambient, diffuse, and specular fan lighting.
 * @author Zoey <t.zoey@digipen.edu>
 */

precision highp float;

in vec3 WorldPosition;
in vec3 WorldNormal;

uniform vec3 PARTCOLOR;
uniform vec3 LIGHTPOSITION;
uniform vec3 CAMERAPOSITION;

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(WorldNormal);
    vec3 lightDirection = normalize(LIGHTPOSITION - WorldPosition);
    vec3 viewDirection = normalize(CAMERAPOSITION - WorldPosition);
    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float diffuseStrength = max(dot(normal, lightDirection), 0.0);
    float specularStrength = diffuseStrength > 0.0
        ? pow(max(dot(viewDirection, reflectionDirection), 0.0), 24.0)
        : 0.0;

    vec3 ambient = 0.18 * PARTCOLOR;
    vec3 diffuse = 0.76 * diffuseStrength * PARTCOLOR;
    vec3 specular = 1.05 * specularStrength * vec3(1.0);
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
