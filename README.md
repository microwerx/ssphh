# SSPHH

This is a program that implements the Scalable Spherical Harmonics Hierarchies algorithm.

It relies on the *Fluxions 3D Engine* which is a research graphics library. It also relies on the *StarFish* libraries which are support libraries handling a variety of different tools.

* `Damselfish` is a scripting language (in design) and JSON library
* `Hatchetfish` is a logging library
* `Sunfish` is a ray tracing library
* `Toadfish` is an audio library using SDL_Mixer
* `Unicornfish` is a network library using CZMQ
* `Viperfish` is a Graphical User Interface library
* `Jellyfish` is a turntable library (in design)

The libraries above depend on the following libraries:

* `openexr`
* `czmq` and `zeromq`
* `curl`
* `GLFW`
* `GLEW`
* `SDL2`
* `SDL2_image`
* `SDL2_mixer`

## Physically Based Rendering

The process of the PBR process is centered around the rendering equation

$$L_o (\mathbf{x} \to \omega_o) = L_i ( \mathbf{x} \to \omega_o ) + \int_\Omega {f_r(\omega_i, \omega_o)\ L_i(\omega_i \to \mathbf{x})\ \langle\omega_i, \omega_g\rangle\ d\omega_i}\ .$$

The light paths that we consider include *directional lights*, *point lights*, and *anisotropic lights*. SSPHH uses two main directional lights from the sun and moon.

### Disney BRDF

```glsl

```

## Vertex Shader

### Uniforms

```glsl
layout(std140) uniform CameraBlock {
	mat4 ProjectionMatrix;
	mat4 CameraMatrix;
};

uniform mat4 DirToShadowMatrix[MAX_DIRTOLIGHT_COUNT];
uniform mat4 WorldMatrix;
```

### Inputs

```glsl
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aColor;
```

### Outputs

```glsl
out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vColor;
out vec3 vCameraPosition;
out vec4 vShadowCoords[MAX_DIRTOLIGHT_COUNT];
```

### Vertex Shader `main()`

```glsl
void main(void)
{
	vPosition = (WorldMatrix * vec4(aPosition, 1.0)).xyz;
	vCameraPosition = CameraMatrix[3].xyz;
	vTexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
	vNormal = mat3(WorldMatrix) * aNormal;
	vColor = aColor;

	vShadowCoords[0] = DirToShadowMatrix[0] * vec4(vPosition, 1.0);
	vShadowCoords[1] = DirToShadowMatrix[1] * vec4(vPosition, 1.0);
	vShadowCoords[2] = DirToShadowMatrix[2] * vec4(vPosition, 1.0);
	vShadowCoords[3] = DirToShadowMatrix[3] * vec4(vPosition, 1.0);

	mat4 pcw = ProjectionMatrix * CameraMatrix * WorldMatrix;
	gl_Position = pcw * vec4(aPosition, 1.0);
}
```

## Fragment Shader

### Material Uniform Blocks

```glsl
struct Material {
	vec4 Kd;
	vec4 Ks;
	vec4 Ke;
	vec4 Kdroughness;
	vec4 Ksroughness;
	vec4 Kior;
	vec4 KmetallicSpecular;
	vec4 KclearcoatSheen;
};

layout(std140) uniform MaterialBlock {
	Material materials[MAX_MATERIAL_COUNT];
};
```

### DirTo Light Uniform Blocks

```glsl
struct DirToLight {
	vec4 dirTo;
	vec4 shadow;
	vec4 shape;
	vec4 E0;
};

layout(std140) uniform DirToLightBlock {
	DirToLight DirtoLights[MAX_DIRTOLIGHT_COUNT];
};

uniform sampler2D DirToLightColorMaps[MAX_DIRTOLIGHT_COUNT];
uniform sampler2D DirToLightDepthMaps[MAX_DIRTOLIGHT_COUNT];
```

### Point Light Uniform Blocks

```glsl
struct PointLight {
	vec3 position;
	vec3 E0;
	float falloffRadius;
};

layout(std140) uniform PointLightBlock {
	PointLight pointLights[MAX_POINTLIGHT_COUNT];
};

uniform samplerCube PointLightColorMaps[MAX_POINTLIGHT_COUNT];
uniform samplerCube PointLightDepthMaps[MAX_POINTLIGHT_COUNT];
```

### Aniso Light Uniform Blocks

```glsl
// TODO
```

### Samplers and other Uniforms

```glsl
uniform sampler2D MapKd;
uniform sampler2D MapKs;
uniform int MtlID;

uniform sampler2D MapTest2D;
```
