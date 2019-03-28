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

const int bounces = 6;

ivec2 pix;
#define PI 3.14159265358979323846
#define OneOverPI 0.31830988618379067153

//===========================PRIMITIVE==DEFINITIONS===============================
struct material
{
	float roughness;
	float metalness;

	vec3 color;
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

#define LIGHT_COUNT 2

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
	{{vec3(-5, 0, 5), vec3(0, 1, 0)}, {vec3(5, 0, 5), vec3(0, 1, 0)}, {vec3(-5, 0, -5), vec3(0, 1, 0)}, {0.01, 0, vec3(1, 1, 1)}},
	{{vec3(-5, 0, -5), vec3(0, 1, 0)}, {vec3(5, 0, 5), vec3(0, 1, 0)}, {vec3(5, 0, -5), vec3(0, 1, 0)}, {0.01, 0, vec3(1, 1, 1)}},
	BOX(-0.5, 0, -0.5, 0.5, 1, 0.5, material(0.2, 0, vec3(1, 0.2, 0.2))),
};

const int time_mod = 20000;
const float time_div = time_mod / (2 * PI);

const float first_light_location_value = (time % time_mod) / time_div + PI * 0.6;
const float second_light_location_value = (time % time_mod) / time_div;

const light lights[] =
{
	{vec3(cos(first_light_location_value) * 4, 0.3, sin(first_light_location_value) * 4), 16, vec3(0, 1, 0)},
	{vec3(cos(second_light_location_value) * 3, 0.7, sin(second_light_location_value) * 3), 20, vec3(1)}
};

struct hitinfo
{
	vec3 hit;
	material mat;
	vec3 normal;

	vec3 dir;
	bool end;
};

struct traceinfo
{
	vec3 color;
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
		rand_stage = tea(uvec2(abs(pix.x), abs(pix.y)));
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

vec3 compute_tangent(vec3 normal)
{
	if (normal.x == 0 && normal.z == 0)
		return vec3(1, 0, 0);
	float a = normal.z / sqrt(normal.z * normal.z + normal.x * normal.x);
	float b = -normal.x / sqrt(normal.z * normal.z + normal.x * normal.x);
	return vec3(a, 0, b);
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

const float margin = 0.01;
bool intersectPrimitives(vec3 origin, vec3 dir, out hitinfo info) 
{
	float smallest = MAX_SCENE_BOUNDS;
	bool found = false;
	
	for (int i = 0; i < TRIANGLE_COUNT; ++i)
	{
		float t = intersectTriangle(origin, dir, i);
		vec3 hit = origin + t * dir;
		vec3 normal = interpolateNormal(i, hit);
		
		if (t > 0.01 && t < smallest 
#ifdef CULL_BACKFACES
		&& dot(-dir, normal) > 0
#endif
		)
		{
			info.hit = hit;
			info.mat = triangles[i].mat;
			info.normal = normal;
			smallest = t;
			found = true;
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
vec3 uniform_hemisphere_sample()
{
	vec2 rv = rand();

	float r = sqrt(1.0f - rv[0] * rv[0]);
    float phi = 2 * PI * rv[1];
 
    return vec3(cos(phi) * r, rv[0], sin(phi) * r);
}

vec3 cosineSample()
{
	vec2 rv = rand();
	float sin_theta = sqrt(rv[0]);   
	float cos_theta = sqrt(1 - sin_theta * sin_theta);

	float psi = rv[1] * 2 * PI;
 
	return vec3(sin_theta * cos(psi), cos_theta, sin_theta * sin(psi));
}

vec3 halfvector_GGX_sample(float roughness)
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

vec3 get_diffuse_color(material mat)
{
	return mix(mat.color, vec3(0), mat.metalness);
}

vec3 get_specular_color(material mat)
{
	return mix(vec3(1), mat.color, mat.metalness);
}

float shadowing_schlick(float roughness, float d)
{
	float k = roughness * sqrt(2 * OneOverPI);
	return d / (d * (1 - k) + k);
}

float shadowing_smith(float roughness, float n_v, float n_l)
{
	return shadowing_schlick(roughness, n_v) * shadowing_schlick(roughness, n_l);
}

float attenuation(float dist)
{
	return dist * dist + dist + 1;
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

vec3 LambertBRDF(material m, vec3 wo, vec3 wi)
{
	return m.color * OneOverPI;
}

vec4 brdf(vec3 wo, vec3 wi, float cos_theta, hitinfo h, vec3 h_t)
{
	float D = computeD(h, cos_theta);

	float v_h = dot(h_t, wi);
	vec3 f = vec3(0.04);
	f = mix(f, h.mat.color, h.mat.metalness);
	vec3 F = f + (1 - f) * pow(1 - v_h, 5);

	float n_v = abs(dot(h.normal, wo));
	float n_l = abs(dot(h.normal, wi));
	float G = shadowing_smith(h.mat.roughness, n_v, n_l);

	vec3 reflectance = F * D * G / abs(n_v * n_l * 4);

	return vec4(reflectance * get_specular_color(h.mat), D); // this might need to be f
}

vec3 ambient_contribution()
{
	return backColor / 10;
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

vec3 light_specular(vec3 position, hitinfo h, vec3 wi, vec3 wo, int k)
{
	vec3 half_normal = normalize(wo - wi);
	float cos_theta = max(0, dot(half_normal, h.normal));

	// in the brdf both rays need to be pointing away from the surface
	vec4 res = brdf(wo, -wi, cos_theta, h, half_normal);
	vec3 brdf_color = res.rgb;

	float distance = length(lights[k].position - position);
	float pdf = pdfGGX(res.a, half_normal, wi);

	return (get_specular_color(h.mat) * brdf_color * lights[k].color * dot(h.normal, -wi) * lights[k].intensity) / attenuation(distance) / pdf;
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
vec3 light_diffuse(vec3 position, hitinfo h, vec3 wi, int k)
{
	float distance = length(lights[k].position - position);


	return (get_diffuse_color(h.mat) * OneOverPI * lights[k].color * dot(h.normal, wi) * lights[k].intensity) / attenuation(distance);
}

vec3 getContributions(hitinfo h, vec3 wo)
{
	vec3 position = h.hit;
	vec3 color = vec3(0);
	for (int i = 0; i < LIGHT_COUNT; ++i)
	{
		// 'wi' points towards the light
		vec3 wi = normalize(lights[i].position - position);
		// find any obstructing geometry between the surface point and the light
		if (CheckVisibility(position, wi, length(lights[i].position - position)))
		{
			//color += light_specular(position, h, wi, wo, i);
			color += light_diffuse(position, h, wi, i);
		}
	}

	//color += h.mat.color * ambient_contribution();

	return color;
}

hitinfo rayCast(vec3 origin, vec3 dir) 
{
	hitinfo h;
	h.end = true;
	if (intersectPrimitives(origin, dir, h)) 
	{
		vec3 tangent = compute_tangent(h.normal);
		vec3 bitangent = cross(h.normal, tangent);
		mat3 TBN = mat3(tangent, h.normal, bitangent);

		vec3 half_vector = cosineSample();
		vec3 half_vector_transformed = normalize(TBN * half_vector);

		vec3 wi = reflect(dir, half_vector_transformed);

		h.dir = wi;
		h.end = false;
	}
	return h;
}

vec4 accumulateColor(vec3 origin, vec3 dir)
{	
	hitinfo hits[bounces + 1];
	int end = 0;

	for (int i = 0; i < bounces + 1; ++i)
	{
		hits[i] = rayCast(origin, dir);
		origin = hits[i].hit;
		dir = hits[i].dir;
		if (!hits[i].end)
			++end;
		else
			break;
	}

	//return vec4(vec3(end - 1 >= 0), 1);

	// For each bounce accumulate its color attenuated by bouncing and by distance travelled
	vec3 color = vec3(0);
	for (int i = end; i >= 0; --i)
	{ 
		if (hits[i].end)
		{
			// Bounce into the environment
			color = backColor;
			continue;
		}

		// Modify by distance, last bounce and brdf
		color *= 0.65; // bounce energy loss (in the future it should depend on the material)
		
		vec3 wi = hits[i].dir;
		vec3 wo;

		if (i == 0)
			wo = normalize(eye - hits[i].hit);
		else
			wo = -hits[i - 1].dir;

		// TODO: do this only if you have a color to transform
		if (color.x > 0 && color.y > 0 && color.z > 0)
			color = (get_diffuse_color(hits[i].mat) * LambertBRDF(hits[i].mat, wo, wi) * color * dot(hits[i].normal, wi)) / pdfCosine(hits[i].normal, wi);
		color += getContributions(hits[i], wo);
	}

	return vec4(color, 1);
}

vec4 pixelSample(vec3 origin, vec3 dir, int samples)
{
	if (samples <= 0)
		return vec4(0, 0, 0, 1);
	// TODO: jitter
	vec4 color = accumulateColor(origin, dir);
	for (int i = 1; i < samples; ++i)
	{
		color += accumulateColor(origin, dir);
	}
	return color / float(samples);
}
//================================================================================

//===================================MAIN=========================================
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main(void) 
{
	pix = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(framebuffer);
	
	if (pix.x >= size.x || pix.y >= size.y) 
		return;
	
	vec2 pos = vec2(pix) / vec2(size.x - 1, size.y - 1);
	vec3 dir = normalize(mix(mix(ray00, ray01, pos.y), mix(ray10, ray11, pos.y), pos.x));
	vec4 color = pixelSample(eye, dir, 16);
	imageStore(framebuffer, pix, color);
}
//================================================================================
