#version 430 core

layout(binding = 0, rgba32f) uniform image2D framebuffer;

//=============================CAMERA==PARAMETERS=================================
uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray01;
uniform vec3 ray10;
uniform vec3 ray11;
//================================================================================

uniform vec3 backColor;
uniform int time;
uniform int samples;
const int bounces = 0;

ivec2 pix;
#define PI 3.14159265358979323846
#define OneOverPI 0.31830988618379067153

#define USE_SHADOWS
//#undef USE_SHADOWS

#define LIGHT_SAMPLE_COUNT 8

//===========================PRIMITIVE==DEFINITIONS===============================
struct Material
{
	float roughness;
	float metalness;
	float emissivity;

	vec3 color;
};

struct vertex
{
	vec3 position;
	vec3 normal;
};

struct Triangle
{
	vertex v1;
	vertex v2;
	vertex v3;

	Material material;
};

struct SphericalLight
{
	vec3 position;
	float intensity;
	vec3 color;
	float radius;
};

#define MAX_SCENE_BOUNDS 100.0

#define BOX_COUNT 1

#define TRIANGLE_COUNT 2 + BOX_COUNT * 12

#define LIGHT_COUNT 1

#define BOX(x1, y1, z1, x2, y2, z2, material)\
{{vec3(x1, y1, z2), vec3(0, 0, 1)}, {vec3(x2, y1, z2), vec3(0, 0, 1)}, {vec3(x1, y2, z2), vec3(0, 0, 1)}, material},\
{{vec3(x1, y2, z2), vec3(0, 0, 1)}, {vec3(x2, y1, z2), vec3(0, 0, 1)}, {vec3(x2, y2, z2), vec3(0, 0, 1)}, material},\
{{vec3(x2, y1, z2), vec3(1, 0, 0)}, {vec3(x2, y1, z1), vec3(1, 0, 0)}, {vec3(x2, y2, z2), vec3(1, 0, 0)}, material},\
{{vec3(x2, y2, z2), vec3(1, 0, 0)}, {vec3(x2, y1, z1), vec3(1, 0, 0)}, {vec3(x2, y2, z1), vec3(1, 0, 0)}, material},\
{{vec3(x2, y1, z1), vec3(0, 0, -1)}, {vec3(x1, y1, z1), vec3(0, 0, -1)}, {vec3(x2, y2, z1), vec3(0, 0, -1)}, material},\
{{vec3(x2, y2, z1), vec3(0, 0, -1)}, {vec3(x1, y1, z1), vec3(0, 0, -1)}, {vec3(x1, y2, z1), vec3(0, 0, -1)}, material},\
{{vec3(x1, y1, z1), vec3(-1, 0, 0)}, {vec3(x1, y1, z2), vec3(-1, 0, 0)}, {vec3(x1, y2, z1), vec3(-1, 0, 0)}, material},\
{{vec3(x1, y2, z1), vec3(-1, 0, 0)}, {vec3(x1, y1, z2), vec3(-1, 0, 0)}, {vec3(x1, y2, z2), vec3(-1, 0, 0)}, material},\
{{vec3(x1, y1, z2), vec3(0, -1, 0)}, {vec3(x2, y1, z2), vec3(0, -1, 0)}, {vec3(x1, y1, z1), vec3(0, -1, 0)}, material},\
{{vec3(x1, y1, z1), vec3(0, -1, 0)}, {vec3(x2, y1, z2), vec3(0, -1, 0)}, {vec3(x2, y1, z1), vec3(0, -1, 0)}, material},\
{{vec3(x1, y2, z2), vec3(0, 1, 0)}, {vec3(x2, y2, z2), vec3(0, 1, 0)}, {vec3(x1, y2, z1), vec3(0, 1, 0)}, material},\
{{vec3(x1, y2, z1), vec3(0, 1, 0)}, {vec3(x2, y2, z2), vec3(0, 1, 0)}, {vec3(x2, y2, z1), vec3(0, 1, 0)}, material}

const Triangle triangles[] =
{
	{{vec3(-5, 0, 5), vec3(0, 1, 0)}, {vec3(5, 0, 5), vec3(0, 1, 0)}, {vec3(-5, 0, -5), vec3(0, 1, 0)}, {0.9, 0, 0, vec3(1, 1, 1)}},
	{{vec3(-5, 0, -5), vec3(0, 1, 0)}, {vec3(5, 0, 5), vec3(0, 1, 0)}, {vec3(5, 0, -5), vec3(0, 1, 0)}, {0.9, 0, 0, vec3(1, 1, 1)}},
	BOX(-0.5, 0, -0.5, 0.5, 1, 0.5, Material(0.9, 0, 0, vec3(1, 0.2, 0.2))),
};

const SphericalLight sphericalLights[] =
{
	SphericalLight(vec3(3, 1.6, 2), 100, vec3(1, 1, 1), 0.1)
};

struct HitInfo
{
	vec3 at;
	Material material;
	vec3 normal;

	vec3 dir;
	bool end;
};

//================================================================================

//==================================RANDOM========================================
bool rand_init = true;
uvec2 rand_stage;
#define TEA_ITERATION_COUNT 32
uvec2 tea(uvec2 v)
{
	uint k[4] = { 0xA341316C, 0xC8013EA4, 0xAD90777D, 0x7E95761E };
	uint sum = 0;
	const uint delta = 0x9E3779B9;

	for (uint i = 0; i < TEA_ITERATION_COUNT; ++i)
	{
		sum += delta;
		v[0] += ((v[1] << 4) + k[0]) & (v[1] + sum) & ((v[1] >> 5) + k[1]);
		v[1] += ((v[0] << 4) + k[2]) & (v[0] + sum) & ((v[0] >> 5) + k[3]);
	}
	return v;
}

vec2 rand()
{
	if (rand_init)
	{
		rand_init = false;
		rand_stage = tea(uvec2(abs(pix.x * 3 + 13 * gl_GlobalInvocationID.z), abs(pix.y + 19 * gl_GlobalInvocationID.z)));
		rand_stage = tea(rand_stage);
		return rand_stage / float(uint(0xFFFFFFFF));
	}
	rand_stage = tea(rand_stage);
	return rand_stage / float(uint(0xFFFFFFFF));
}
//================================================================================

//=======================PRIMITIVE==INTERSECTIONS=================================

float intersectTriangle(vec3 origin, vec3 dir, int i)
{
	vec3 e1 = triangles[i].v2.position - triangles[i].v1.position;
    vec3 e2 = triangles[i].v3.position - triangles[i].v1.position;
    vec3 p = cross(dir, e2);
    float a = dot(e1, p);

    if(a == 0)
        return -1;

    float f = 1.0f / a;

    vec3 s = origin - triangles[i].v1.position;
    float u = f * dot(s, p);
    if(u < 0.0f || u > 1.0f)
        return -2;

    vec3 q = cross(s, e1);
    float v = f * dot(dir, q);

    if(v < 0.0f || u + v > 1.0f)
        return -3;

    float res = f * dot(e2, q);

    return res;
}

vec3 nearestLinePoint(vec3 b, vec3 a, vec3 v1)
{
	float nom = v1.x * (b.x - a.x) + v1.y * (b.y - a.y) + v1.z * (b.z - a.z);
	float denom = v1.x * v1.x + v1.y * v1.y + v1.z * v1.z;

	float k = nom / denom;

	return a + k * v1;
}

float linePointDistance(vec3 b, vec3 a, vec3 v1)
{
	return length(nearestLinePoint(b, a, v1) - b);
}

float triangleArea(vec3 v1, vec3 v2, vec3 v3)
{
	return length(v1 - v2) * linePointDistance(v3, v1, v2 - v1) / 2.0f;
}

vec3 interpolateNormal(int i, vec3 hit)
{
	float area = triangleArea(triangles[i].v1.position, triangles[i].v2.position, triangles[i].v3.position);
	float bar1 = triangleArea(triangles[i].v2.position, triangles[i].v3.position, hit) / area;
	float bar2 = triangleArea(triangles[i].v3.position, triangles[i].v1.position, hit) / area;
	float bar3 = 1 - bar1 - bar2;

	return normalize(triangles[i].v1.normal * bar1 + triangles[i].v2.normal * bar2 + triangles[i].v3.normal * bar3);
};

float IntersectSphere(vec3 origin, vec3 dir, int k)
{
	float t0, t1;
	vec3 L = sphericalLights[k].position - origin;
	float tca = dot(L, dir);
	if (tca < 0) 
		return -1;
	float d2 = dot(L, L) - tca * tca;

	float radius2 = sphericalLights[k].radius * sphericalLights[k].radius;
	if (d2 > radius2) 
		return -2;

	float thc = sqrt(radius2 - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	return t0 < t1 ? t0 : t1;
}

const float margin = 0;
bool intersectPrimitives(vec3 origin, vec3 dir, out HitInfo info)
{
	float smallest = MAX_SCENE_BOUNDS;
	bool found = false;
	
	for (int i = 0; i < TRIANGLE_COUNT; ++i)
	{
		float t = intersectTriangle(origin, dir, i);
		vec3 hit = origin + t * dir;
		
		if (t > margin && t < smallest)
		{
			vec3 normal = interpolateNormal(i, hit);
			if (dot(-dir, normal) > 0)
			{
				info.at = hit;
				info.material = triangles[i].material;
				info.normal = normal;
				smallest = t;
				found = true;
			}
		}
	}

	//for (int i = 0; i < LIGHT_COUNT; ++i)
	//{
	//	float t = IntersectSphere(origin, dir, i);
	//	vec3 hit = origin + t * dir;
	//
	//	if (t > margin && t < smallest)
	//	{
	//		vec3 normal = normalize(hit - sphericalLights[i].position);
	//		if (dot(-dir, normal) > 0)
	//		{
	//			info.at = hit;
	//			info.material = Material(0, 0, sphericalLights[i].intensity, sphericalLights[i].color);
	//			info.normal = normal;
	//			smallest = t;
	//			found = true;
	//		}
	//	}
	//}

	return found;
}

bool CheckVisibility(vec3 origin, vec3 dir, float length)
{
	for (int i = 0; i < TRIANGLE_COUNT; ++i)
	{
		float t = intersectTriangle(origin, dir, i);
		
		if (t > 0.01 && t < length)
		{
			return false;
		}
	}

	return true;
}
//================================================================================

//=================================SAMPLE=========================================
vec3 cosineSample()
{
	vec2 rv = rand();
	float sin_theta = sqrt(rv[0]);   
	float cos_theta = sqrt(1 - rv[0]);

	float psi = rv[1] * 2 * PI;
 
	return vec3(sin_theta * cos(psi), cos_theta, sin_theta * sin(psi));
}
//================================================================================

//==================================TRACE=========================================
vec3 getDiffuseColor(const Material material)
{
	// A metallic surface has no diffuse color, dielectrics have the albedo diffuse color
	return mix(material.color, vec3(0), material.metalness);
}

vec3 getSpecularColor(const Material material)
{
	// A metallic surface reflects the world in the color of its albedo while a dielectric has a weak white color
	return mix(vec3(0.1), material.color, material.metalness);
}

float getSphericalLightArea(int k)
{
	return 4 * PI * sphericalLights[k].radius * sphericalLights[k].radius;
}

HitInfo sampleSphericalLight(int k)
{
	vec2 r = rand();

	float theta = 2 * PI * r.x;	
	float phi = acos(1 - 2 * r.y);

	HitInfo hit;
	hit.at = vec3(sin(phi) * cos(theta), cos(phi), sin(phi) * sin(theta)) * sphericalLights[k].radius + sphericalLights[k].position;
	hit.normal = normalize(hit.at - sphericalLights[k].position);
	hit.material.color = sphericalLights[k].color;
	hit.material.emissivity = sphericalLights[k].intensity;
	
	return hit;
}

float TorranceSparrowDistributionTerm(const HitInfo surfaceHit, vec3 halfNormal)
{
	float NDotM = dot(halfNormal, surfaceHit.normal);
	if (NDotM == 0)
		return 1;

	float a2 = NDotM * NDotM;
	float tang = (a2 - 1) / a2;
	float r2 = surfaceHit.material.roughness * surfaceHit.material.roughness;

	float denom = PI * r2 * a2 * a2;
	float nom = exp(tang * r2);

	return nom / denom;
}

float TorranceSparrowGeometryTerm(const HitInfo surfaceHit, vec3 wo, vec3 wi, vec3 halfNormal, float cosTheta)
{
	float NDotM = dot(surfaceHit.normal, halfNormal);
	float g1 = abs(2 * NDotM * dot(surfaceHit.normal, wo) / cosTheta);
	float g2 = abs(2 * NDotM * dot(surfaceHit.normal, wi) / cosTheta);

	return min(1, min(g1, g2));
}

vec4 TorranceSparrowBRDF(const HitInfo surfaceHit, vec3 wo, vec3 wi, vec3 halfNormal)
{
	float cosTheta = dot(wo, halfNormal);

	float D = TorranceSparrowDistributionTerm(surfaceHit, halfNormal);
	float G = TorranceSparrowGeometryTerm(surfaceHit, wo, wi, halfNormal, cosTheta);
	vec3 F0 = vec3(0.9);
	vec3 F = F0 + (1 - F0) * pow(1 - cosTheta, 5);

	vec3 res = D * G * F / abs(4 * dot(surfaceHit.normal, wi) * dot(surfaceHit.normal, wo));

	return vec4(res + getDiffuseColor(surfaceHit.material), D);
}

vec3 solveLight(const HitInfo surfaceHit, const HitInfo lightHit, vec3 wo, vec3 wi, float area)
{
	vec3 halfNormal = normalize(wi + wo);
	vec4 torranceSparrow = TorranceSparrowBRDF(surfaceHit, wo, wi, halfNormal);
	vec3 brdf = torranceSparrow.rgb;
	float dist = length(surfaceHit.at - lightHit.at);
	float pdf = dist * dist / (area * abs(dot(lightHit.normal, -wi)));

	return brdf * lightHit.material.color * lightHit.material.emissivity * max(0, dot(wi, surfaceHit.normal)) / pdf;
}

vec3 getContributions(const HitInfo h, vec3 wo)
{
	vec3 color = vec3(0);
	for (int i = 0; i < LIGHT_COUNT; ++i)
	{
		vec3 thisLightColor = vec3(0);
		float area = getSphericalLightArea(i);
		// find any obstructing geometry between the surface point and the light
		for (int j = 0; j < LIGHT_SAMPLE_COUNT; ++j)
		{
			HitInfo lightHit = sampleSphericalLight(i);
			// 'wi' points towards the light
			vec3 wi = normalize(lightHit.at - h.at);
			if (dot(lightHit.normal, wi) < 0 && CheckVisibility(h.at, wi, length(lightHit.at - h.at)))
			{
				thisLightColor += solveLight(h, lightHit, wo, wi, area);
			}
		}
		color += thisLightColor / LIGHT_SAMPLE_COUNT;
	}

	return color;
}

HitInfo rayCast(vec3 origin, vec3 dir)
{
	HitInfo h;
	h.end = true;
	if (intersectPrimitives(origin, dir, h)) 
	{
		h.end = false;
	}
	return h;
}

struct frame
{
	vec3 x;
	vec3 y;
	vec3 z;
};

// Presume normal is normalized
frame createFrame(vec3 normal)
{
	vec3 tangent = (normal.x > 0.99f) ? vec3(0, 1, 0) : vec3(1, 0, 0);
	if (normal.x < -0.99f)
		tangent = vec3(0, -1, 0);
	vec3 bitangent = cross(normal, tangent);
	tangent = cross(bitangent, normal);
	return frame(tangent, normal, bitangent);
}

vec3 transformFromFrame(const frame f, vec3 v)
{
	return f.x * v.x + f.y * v.y + f.z * v.z;
}

vec3 transformToFrame(const frame f, vec3 v)
{
	return vec3(dot(v, f.x), dot(v, f.y), dot(v, f.z));
}

float pdfCosine(vec3 normal, vec3 wi)
{
	// 'normal' and 'wi' both need to be normalized
	return dot(normal, wi) * OneOverPI;
}

vec3 reflectUniform(const HitInfo h)
{
	return cosineSample();
}

float getReflectance(vec3 color)
{
	return max(max(color.r, color.g), color.b);
}

vec3 sampleTracingRelativeNormal(const HitInfo hit)
{
	vec2 r = rand();
	float theta = atan(sqrt(-hit.material.roughness * hit.material.roughness * log(1 - r.x)));
	float phi = 2 * PI * r.y;

	return normalize(vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi)));
}

float getTracingDirectionProbability(float D, vec3 halfNormal, vec3 wi)
{
	float cos = dot(halfNormal, wi);
	return max(0.0001, D / (4 * abs(cos)));
}

vec4 accumulateColor(vec3 origin, vec3 dir)
{	
	//const int maxDepth = 50;
	int hitCount = -1;
	vec3 accumulatedColor = vec3(0);
	vec3 throughput = vec3(1);

	//HitInfo hit = rayCast(origin, dir);
	//if (hit.end)
	//	return vec4(0, 0, 0, 1);
	//else
	//	return vec4(getContributions(hit, -dir) + hit.material.emissivity * hit.material.color, 1);
	
	float roulette = 0, refl = 1;

	while (roulette <= refl)
	{
		HitInfo hit = rayCast(origin, dir);
		frame f = createFrame(hit.normal);
		vec3 localDir = transformToFrame(f, dir);
		++hitCount;

		// The 'hitCount' cap needs to be here for the shaders to work (there will not be that many bounces anyway)
		if (!hit.end && hitCount < 50)
		{
			accumulatedColor += throughput * (getContributions(hit, -dir));

			vec3 halfNormalLocal = sampleTracingRelativeNormal(hit);
			vec3 halfNormal = normalize(transformFromFrame(f, halfNormalLocal));
			vec3 reflectDir = reflect(dir, halfNormal);

			vec4 torranceSparrow = TorranceSparrowBRDF(hit, -dir, reflectDir, halfNormal);
			vec3 brdf = torranceSparrow.rgb;
			float D = torranceSparrow.a;
			float pdf = getTracingDirectionProbability(D, halfNormal, reflectDir);

			throughput *= brdf * max(0, dot(hit.normal, reflectDir)) / pdf;

			// Russian roulette
			vec2 r = rand();
			roulette = r.x;
			refl = getReflectance(throughput);

			throughput /= refl;
			
			origin = hit.at;
			dir = reflectDir;
		}
		else
		{
			break;
		}
	}
	if (hitCount > 0)
		accumulatedColor /= hitCount;

	return vec4(accumulatedColor, 1);
}

vec4 pixelSample(vec3 origin, vec3 dirUpLeft, vec3 dirUpRight, vec3 dirDownLeft, vec3 dirDownRight)
{
	vec2 r = rand();
	vec3 dir = normalize(mix(mix(dirUpLeft, dirDownLeft, r.y), mix(dirUpRight, dirDownRight, r.y), r.x));
	return accumulateColor(origin, dir);
}
//================================================================================

#define SAMPLE_COUNT 64
#define LOCAL_SIZES 1

shared vec4 sampleBuffer[LOCAL_SIZES][LOCAL_SIZES][SAMPLE_COUNT];

//===================================MAIN=========================================
layout (local_size_x = LOCAL_SIZES, local_size_y = LOCAL_SIZES, local_size_z = SAMPLE_COUNT) in;
void main(void) 
{	
	pix = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(framebuffer);
	
	if (pix.x >= size.x || pix.y >= size.y)
		return;

	vec2 pos = vec2(pix) / vec2(size.x - 1, size.y - 1);

	vec2 posUp = vec2(pix.x, pix.y + 1) / vec2(size.x - 1, size.y - 1);
	vec2 posDown = vec2(pix.x, pix.y - 1) / vec2(size.x - 1, size.y - 1);
	vec2 posLeft = vec2(pix.x - 1, pix.y) / vec2(size.x - 1, size.y - 1);
	vec2 posRight = vec2(pix.x + 1, pix.y) / vec2(size.x - 1, size.y - 1);

	vec2 posUpLeft = (posUp + posLeft) * 0.5;
	vec2 posUpRight = (posUp + posRight) * 0.5;
	vec2 posDownLeft = (posDown + posLeft) * 0.5;
	vec2 posDownRight = (posDown + posRight) * 0.5;

	vec3 dirUpLeft = normalize(mix(mix(ray00, ray01, posUpLeft.y), mix(ray10, ray11, posUpLeft.y), posUpLeft.x));
	vec3 dirUpRight = normalize(mix(mix(ray00, ray01, posUpRight.y), mix(ray10, ray11, posUpRight.y), posUpRight.x));
	vec3 dirDownLeft = normalize(mix(mix(ray00, ray01, posDownLeft.y), mix(ray10, ray11, posDownLeft.y), posDownLeft.x));
	vec3 dirDownRight = normalize(mix(mix(ray00, ray01, posDownRight.y), mix(ray10, ray11, posDownRight.y), posDownRight.x));
	vec4 color = pixelSample(eye, dirUpLeft, dirUpRight, dirDownLeft, dirDownRight);

	sampleBuffer[gl_GlobalInvocationID.x % LOCAL_SIZES][gl_GlobalInvocationID.y % LOCAL_SIZES][gl_GlobalInvocationID.z % SAMPLE_COUNT] = color;

	barrier();

	if (gl_GlobalInvocationID.z % LOCAL_SIZES == 0)
	{
		for (int i = 1; i < SAMPLE_COUNT; ++i)
		{
			color += sampleBuffer[gl_GlobalInvocationID.x % LOCAL_SIZES][gl_GlobalInvocationID.y % LOCAL_SIZES][i];
		}

		imageStore(framebuffer, pix, color / SAMPLE_COUNT);
	}
}
//================================================================================
