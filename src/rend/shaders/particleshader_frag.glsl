#version 330

in FragData
{
	flat vec3 sphereCenter_camera;
	float sphereRadius;
	vec2 mapping;
	vec4 sphereColor;
};

layout(std140) uniform Camera
{
    mat4 worldToCamera;
    mat4 cameraToProjection;
};

out vec4 frag_color;

void Impostor(out vec3 fragmentPos_camera, out vec3 fragmentNormal_camera)
{
	vec3 planePos_camera = vec3(mapping * sphereRadius, 0.0) + sphereCenter_camera;
	vec3 rayDirection_camera = normalize(planePos_camera);

	float B = 2.0 * dot(rayDirection_camera, -sphereCenter_camera);
	float C = dot(sphereCenter_camera, sphereCenter_camera) -
		(sphereRadius * sphereRadius);

	float det = (B * B) - (4 * C);
	if(det < 0.0)
		discard;

	float sqrtDet = sqrt(det);
	float posT = (-B + sqrtDet)/2;
	float negT = (-B - sqrtDet)/2;

	float intersectT = min(posT, negT);
	fragmentPos_camera = rayDirection_camera * intersectT;
	fragmentNormal_camera = normalize(fragmentPos_camera - sphereCenter_camera);
}

void main () {
    vec3 fragmentPos_camera;
    vec3 fragmentNormal_camera;
    Impostor(fragmentPos_camera, fragmentNormal_camera);

    vec4 fragmentPos_clip = cameraToProjection * vec4(fragmentPos_camera, 1.0);
    float ndcDepth = fragmentPos_clip.z / fragmentPos_clip.w;
    gl_FragDepth = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;

    frag_color = sphereColor;

}