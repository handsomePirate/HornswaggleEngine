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

//===========================PRIMITIVE==DEFINITIONS===============================
struct material
{
	float roughness;
	float metalness;

	vec3 color;
	float IOR;
};

struct vertex
{
	vec3 position;
	vec3 normal;
};

struct triangle
{
	vertex v1;
	vertex v2;
	vertex v3;

	material mat;
};

struct light
{
	vec3 position;
	float intensity;
	vec3 color;
};

#define CULL_BACKFACES
//#undef CULL_BACKFACES

#define MAX_SCENE_BOUNDS 100.0

#define BOX_COUNT 1

#define TRIANGLE_COUNT 2 + BOX_COUNT * 12

#define LIGHT_COUNT 1

#define BOX(x1, y1, z1, x2, y2, z2, mat)\
{{vec3(x1, y1, z2), vec3(0, 0, 1)}, {vec3(x2, y1, z2), vec3(0, 0, 1)}, {vec3(x1, y2, z2), vec3(0, 0, 1)}, mat},\
{{vec3(x1, y2, z2), vec3(0, 0, 1)}, {vec3(x2, y1, z2), vec3(0, 0, 1)}, {vec3(x2, y2, z2), vec3(0, 0, 1)}, mat},\
{{vec3(x2, y1, z2), vec3(1, 0, 0)}, {vec3(x2, y1, z1), vec3(1, 0, 0)}, {vec3(x2, y2, z2), vec3(1, 0, 0)}, mat},\
{{vec3(x2, y2, z2), vec3(1, 0, 0)}, {vec3(x2, y1, z1), vec3(1, 0, 0)}, {vec3(x2, y2, z1), vec3(1, 0, 0)}, mat},\
{{vec3(x2, y1, z1), vec3(0, 0, -1)}, {vec3(x1, y1, z1), vec3(0, 0, -1)}, {vec3(x2, y2, z1), vec3(0, 0, -1)}, mat},\
{{vec3(x2, y2, z1), vec3(0, 0, -1)}, {vec3(x1, y1, z1), vec3(0, 0, -1)}, {vec3(x1, y2, z1), vec3(0, 0, -1)}, mat},\
{{vec3(x1, y1, z1), vec3(-1, 0, 0)}, {vec3(x1, y1, z2), vec3(-1, 0, 0)}, {vec3(x1, y2, z1), vec3(-1, 0, 0)}, mat},\
{{vec3(x1, y2, z1), vec3(-1, 0, 0)}, {vec3(x1, y1, z2), vec3(-1, 0, 0)}, {vec3(x1, y2, z2), vec3(-1, 0, 0)}, mat},\
{{vec3(x1, y1, z2), vec3(0, -1, 0)}, {vec3(x2, y1, z2), vec3(0, -1, 0)}, {vec3(x1, y1, z1), vec3(0, -1, 0)}, mat},\
{{vec3(x1, y1, z1), vec3(0, -1, 0)}, {vec3(x2, y1, z2), vec3(0, -1, 0)}, {vec3(x2, y1, z1), vec3(0, -1, 0)}, mat},\
{{vec3(x1, y2, z2), vec3(0, 1, 0)}, {vec3(x2, y2, z2), vec3(0, 1, 0)}, {vec3(x1, y2, z1), vec3(0, 1, 0)}, mat},\
{{vec3(x1, y2, z1), vec3(0, 1, 0)}, {vec3(x2, y2, z2), vec3(0, 1, 0)}, {vec3(x2, y2, z1), vec3(0, 1, 0)}, mat}

const triangle triangles[] = 
{
	{{vec3(-5, 0, 5), vec3(0, 1, 0)}, {vec3(5, 0, 5), vec3(0, 1, 0)}, {vec3(-5, 0, -5), vec3(0, 1, 0)}, {0.1, 0, vec3(1, 1, 1), 1.33}},
	{{vec3(-5, 0, -5), vec3(0, 1, 0)}, {vec3(5, 0, 5), vec3(0, 1, 0)}, {vec3(5, 0, -5), vec3(0, 1, 0)}, {0.1, 0, vec3(1, 1, 1), 1.33}},
	BOX(-0.5, 0, -0.5, 0.5, 1, 0.5, material(0.1, 0, vec3(1, 0.2, 0.2), 1.33)),
};

const int time_mod = 20000;
const float time_div = time_mod / (2 * PI);

const float first_light_location_value = (time % time_mod) / time_div + PI * 0.6;
const float second_light_location_value = (time % time_mod) / time_div;

const light lights[] =
{
	light(vec3(3, 1.6, 2), 20, vec3(1, 1, 1))
	//{vec3(cos(first_light_location_value) * 4, 0.3, sin(first_light_location_value) * 4), 10, vec3(0, 1, 0)},
	//{vec3(cos(second_light_location_value) * 3, 0.7, sin(second_light_location_value) * 3), 12, vec3(1)}
};

struct hitinfo
{
	vec3 at;
	material mat;
	vec3 normal;

	vec3 dir;
	bool end;
};

//================================================================================

//==================================RANDOM========================================
bool rand_init = true;
uvec2 rand_stage;
#define TEA_ITERATION_COUNT 24
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

const float margin = 0;
bool intersectPrimitives(vec3 origin, vec3 dir, out hitinfo info) 
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
#ifdef CULL_BACKFACES 
			if (dot(-dir, normal) > 0)
#endif
			{
				info.at = hit;
				info.mat = triangles[i].mat;
				info.normal = normal;
				smallest = t;
				found = true;
			}
		}
	}

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
	float cos_theta = sqrt(1 - sin_theta * sin_theta);

	float psi = rv[1] * 2 * PI;
 
	return vec3(sin_theta * cos(psi), cos_theta, sin_theta * sin(psi));
}

vec3 halfvectorGGXSample(float roughness)
{
	vec2 rv = rand();
	
	rv[0] = min(rv[0], 0.9999);

	float theta = atan(roughness * sqrt(rv[0] / (1 - rv[0])));
	float phi = 2 * PI * rv[1];

	float x = sin(theta) * cos(phi);
	float z = sin(theta) * sin(phi);
	float y = cos(theta);

	return vec3(x, y, z);
}
//================================================================================

//==================================TRACE=========================================
vec3 getDiffuseColor(material mat)
{
	// A metallic surface has no diffuse color, dielectrics have the albedo diffuse color
	return mix(mat.color, vec3(0), mat.metalness);
}

vec3 getSpecularColor(material mat)
{
	// A metallic surface reflects the world in the color of its albedo while a dielectric has a weak white color
	return mix(vec3(0.1), mat.color, mat.metalness);
}

float shadowingSchlick(float roughness, float d)
{
	float k = roughness * sqrt(2 * OneOverPI);
	return d / (d * (1 - k) + k);
}

float shadowingSmith(float roughness, float n_v, float n_l)
{
	return shadowingSchlick(roughness, n_v) * shadowingSchlick(roughness, n_l);
}

float attenuation(float dist)
{
	return dist * dist;
}

float attenuation(vec3 position1, vec3 position2)
{
	float len = length(position1 - position2);
	return attenuation(len);
}

float computeD(hitinfo h, float cos_theta)
{
	float roughness_2 = h.mat.roughness * h.mat.roughness;
	float r = (roughness_2 - 1) * cos_theta * cos_theta + 1;
	float D = roughness_2 / (PI * r * r);
	if (isnan(D))
		D = 1;
	return D;
}

float fresnelDielectric(float cosThetaI, float cosThetaT, float etaT)
{
	float eta = 1 / etaT;
	float Rper = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
	float Rpar = (cosThetaI - eta * cosThetaT) / (cosThetaI + eta * cosThetaT);
	return 0.5f*(Rpar*Rpar + Rper * Rper);
}

vec3 fresnelConductor()
{
	return vec3(0.5);
}

vec4 CookTorranceBRDF(vec3 wo, vec3 wi, hitinfo h, bool metallic)
{
	vec3 halfNormal = normalize(wo + wi);
	float cosTheta = halfNormal.y;
	float D = computeD(h, cosTheta);

	float v_h = dot(halfNormal, wi);

	vec3 F;
	if (metallic)
		F = fresnelConductor();
	else
		F = vec3(fresnelDielectric(dot(wi, h.normal), dot(wo, h.normal), h.mat.IOR));

	float n_v = dot(h.normal, wo);
	float n_l = dot(h.normal, wi);
	float G = shadowingSmith(h.mat.roughness, n_v, n_l);

	return vec4(F * D * G / (n_v * n_l * 4), D);
}

float pdfGGX(float D, vec3 wm, vec3 wi)
{
	float cos_theta = abs(dot(wm, wi));
	return max(D / (4 * cos_theta), 0.000001);
}

float pdfCosine(vec3 normal, vec3 wi)
{
	// 'normal' and 'wi' both need to be normalized
	return dot(normal, wi) * OneOverPI;
}

vec3 lightSpecular(vec3 position, hitinfo h, vec3 wi, vec3 wo, int k)
{
	vec3 half_normal = normalize(wo + wi);
	float cos_theta = max(0, dot(half_normal, h.normal));

	// in the brdf both rays need to be pointing away from the surface
	vec4 res = CookTorranceBRDF(wo, wi, h, false);
	vec3 brdf_color = res.rgb;

	float distance = length(lights[k].position - position);

	return brdf_color * getSpecularColor(h.mat) * lights[k].color * dot(h.normal, wi) * lights[k].intensity / attenuation(distance);
}

//////////////////////////////
//	\\ cam     ^ light		//
//   \\		  | \			//
//	  \\  wo  // wi			//
//     \\    //				//
//     \ |  //				//
//       V //				//
//________X_________________//
//							//
//////////////////////////////
vec3 lightDiffuse(vec3 position, hitinfo h, vec3 wi, int k)
{
	float distance = length(lights[k].position - position);

	return getDiffuseColor(h.mat) * OneOverPI * lights[k].color * max(0, dot(h.normal, wi)) * lights[k].intensity / attenuation(distance);
}

vec3 getContributions(hitinfo h, vec3 wo)
{
	vec3 position = h.at;
	vec3 color = vec3(0);
	for (int i = 0; i < LIGHT_COUNT; ++i)
	{
		// 'wi' points towards the light
		vec3 wi = normalize(lights[i].position - position);
		// find any obstructing geometry between the surface point and the light
#ifdef USE_SHADOWS
		if (CheckVisibility(position, wi, length(lights[i].position - position)))
#endif
		{
			color += lightDiffuse(position, h, wi, i);
			//color += lightSpecular(position, h, wi, wo, i);
		}
	}

	return color;
}

hitinfo rayCast(vec3 origin, vec3 dir) 
{
	hitinfo h;
	h.end = true;
	if (intersectPrimitives(origin, dir, h)) 
	{
		/*
		
		*/
		h.end = false;
	}
	return h;
}

vec3 getBackColor(vec3 dir)
{
	// TODO: add environment map option
	return backColor;
}

vec3 getColorAt(hitinfo h, vec3 wo)
{
	if (h.end)
		return getBackColor(wo);
	else
		return getContributions(h, wo);
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

vec3 transformFromFrame(frame f, vec3 v)
{
	return f.x * v.x + f.y * v.y + f.z * v.z;
}

vec3 transformToFrame(frame f, vec3 v)
{
	return vec3(dot(v, f.x), dot(v, f.y), dot(v, f.z));
}

vec3 reflectGGX(hitinfo h, vec3 dir, out vec3 halfNormal)
{
	halfNormal = halfvectorGGXSample(h.mat.roughness);
	return reflect(dir, halfNormal);
}

vec3 reflectUniform(hitinfo h)
{
	return cosineSample();
}

float getReflectance(vec3 color)
{
	return max(max(color.r, color.g), color.b);
}

vec4 accumulateColor(vec3 origin, vec3 dir)
{	
	//const int maxDepth = 50;
	int hitCount = -1;
	vec3 accumulatedColor = vec3(0);
	vec3 throughput = vec3(1);
	while (true)
	{
		hitinfo hit = rayCast(origin, dir);
		frame f = createFrame(hit.normal);
		vec3 localDir = transformToFrame(f, dir);
		++hitCount;

		if (!hit.end && hitCount < 3/* && hitCount < maxDepth*/)
		{
			vec3 diffuseColor = getDiffuseColor(hit.mat);
			
			// Russian roulette
			vec2 r = rand();
			float roulette = (r.x + r.y) * 0.5;
			float refl = getReflectance(diffuseColor);
			if (roulette > refl)
				break;
			//vec3 specularColor = getSpecularColor(hit.mat);

			vec3 localReflectDir = reflectUniform(hit);
			float cos = localReflectDir.y;

			accumulatedColor += throughput * getContributions(hit, -dir);
			throughput *= diffuseColor;

			//bool metallic = hit.mat.metalness >= 0.5;
			//
			//float diffuseReflectance = getReflectance(diffuseColor);
			//float specularReflectance = getReflectance(specularColor);
			//float reflectanceSum = diffuseReflectance + specularReflectance;
			//float r = rand()[0] * reflectanceSum;
			//r = 1;
			//
			//vec3 localReflectDir;
			//if (r < diffuseReflectance)
			//{
			//	localReflectDir = reflectUniform(hit);
			//}
			//else
			//{
			//	vec3 halfNormal;
			//	localReflectDir = reflectGGX(hit, localDir, halfNormal);
			//	return vec4(CookTorranceBRDF(-localDir, localReflectDir, hit, false).rgb, 1);
			//	vec4 brdfd = CookTorranceBRDF(-localDir, localReflectDir, hit, metallic);
			//	vec3 brdf = brdfd.rgb;
			//	//return vec4(brdf, 1);
			//	//pdfGGX(brdfd.a, );
			//	//accumulatedColor;
			//}
			
			origin = hit.at;
			dir = transformFromFrame(f, localReflectDir);
		}
		else
		{
			break;
		}
		//accumulatedColor += hit.mat.color;
	}
	if (hitCount > 0)
		accumulatedColor /= hitCount;
	//accumulatedColor += throughput * backColor;
	return vec4(accumulatedColor, 1);
}

vec4 pixelSample(vec3 origin, vec3 dirUpLeft, vec3 dirUpRight, vec3 dirDownLeft, vec3 dirDownRight)
{
	vec2 r = rand();
	vec3 dir = normalize(mix(mix(dirUpLeft, dirDownLeft, r.y), mix(dirUpRight, dirDownRight, r.y), r.x));
	return accumulateColor(origin, dir);
}
//================================================================================

#define SAMPLE_COUNT 8
#define LOCAL_SIZES 8

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
