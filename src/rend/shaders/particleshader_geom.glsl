#version 330

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

layout(std140) uniform Camera
{
    mat4 worldToCamera;
    mat4 cameraToProjection;
};


in VertexData
{
	vec3 sphereCenter_camera;
	float sphereRadius;
	vec4 sphereColor;
} inData[];

out FragData
{
	flat vec3 sphereCenter_camera;
	float sphereRadius;
	vec2 mapping;
	vec4 sphereColor;
};

const float g_boxCorrection = 1.5;


void main()
{
	vec4 cameraCornerPos;
	//Bottom-left
	mapping = vec2(-1.0, -1.0) * g_boxCorrection;
	sphereCenter_camera = vec3(inData[0].sphereCenter_camera);
	sphereRadius = inData[0].sphereRadius;
	sphereColor = inData[0].sphereColor;
	cameraCornerPos = vec4(inData[0].sphereCenter_camera, 1.0);
	cameraCornerPos.xy += vec2(-inData[0].sphereRadius, -inData[0].sphereRadius) * g_boxCorrection;
	gl_Position = cameraToProjection * cameraCornerPos;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	//Top-left
	mapping = vec2(-1.0, 1.0) * g_boxCorrection;
	sphereCenter_camera = vec3(inData[0].sphereCenter_camera);
	sphereRadius = inData[0].sphereRadius;
	sphereColor = inData[0].sphereColor;
	cameraCornerPos = vec4(inData[0].sphereCenter_camera, 1.0);
	cameraCornerPos.xy += vec2(-inData[0].sphereRadius, inData[0].sphereRadius) * g_boxCorrection;
	gl_Position = cameraToProjection * cameraCornerPos;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	//Bottom-right
	mapping = vec2(1.0, -1.0) * g_boxCorrection;
	sphereCenter_camera = vec3(inData[0].sphereCenter_camera);
	sphereRadius = inData[0].sphereRadius;
	sphereColor = inData[0].sphereColor;
	cameraCornerPos = vec4(inData[0].sphereCenter_camera, 1.0);
	cameraCornerPos.xy += vec2(inData[0].sphereRadius, -inData[0].sphereRadius) * g_boxCorrection;
	gl_Position = cameraToProjection * cameraCornerPos;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	//Top-right
	mapping = vec2(1.0, 1.0) * g_boxCorrection;
	sphereCenter_camera = vec3(inData[0].sphereCenter_camera);
	sphereRadius = inData[0].sphereRadius;
	sphereColor = inData[0].sphereColor;
	cameraCornerPos = vec4(inData[0].sphereCenter_camera, 1.0);
	cameraCornerPos.xy += vec2(inData[0].sphereRadius, inData[0].sphereRadius) * g_boxCorrection;
	gl_Position = cameraToProjection * cameraCornerPos;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();
}