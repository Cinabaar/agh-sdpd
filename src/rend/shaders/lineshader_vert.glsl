#version 330

layout(location = 0) in vec3 pos;


layout(std140) uniform Camera
{
    mat4 worldToCamera;
    mat4 cameraToProjection;
};

void main () {
    gl_Position = cameraToProjection * worldToCamera * vec4(pos, 1.0);
}
