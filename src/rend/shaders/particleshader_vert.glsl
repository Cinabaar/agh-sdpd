#version 330

layout(location = 0) in vec3 sphereCenter;
layout(location = 1) in vec3 sphereColor;
layout(location = 2) in float sphereRadius;


layout(std140) uniform Camera
{
    mat4 worldToCamera;
    mat4 cameraToProjection;
};

out VertexData
{
	vec3 sphereCenter_camera;
	float sphereRadius;
	vec4 sphereColor;
} outData;


void main () {
    vec4 sphereCenter_camera = worldToCamera * vec4(sphereCenter, 1.0);


    outData.sphereCenter_camera = vec3(sphereCenter_camera);
    outData.sphereRadius = sphereRadius;
    outData.sphereColor = vec4(sphereColor, 1.0f);


    gl_Position = sphereCenter_camera;
}