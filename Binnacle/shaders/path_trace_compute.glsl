#version 430 core

layout(binding = 0, rgba32f) uniform image2D framebuffer;

//=============================CAMERA==PARAMETERS=================================
uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray01;
uniform vec3 ray10;
uniform vec3 ray11;
//================================================================================

uniform int time;

const int bounces = 3;

ivec2 pix;
#define PI 3.14159265358979323846

//===========================PRIMITIVE==DEFINITIONS===============================
struct material
{
	float roughness;
	float metalness;

	vec3 color;
};

struct sphere
{
	vec3 center;
	float radius;

	material mat;
};

struct box 
{
	vec3 min;
	vec3 max;

	material mat;
};

struct light
{
	vec3 position;
	float intensity;
	vec3 color;
};

#define MAX_SCENE_BOUNDS 100.0
#define BOX_COUNT 1
#define SPHERE_COUNT 1
#define LIGHT_COUNT 2

const box boxes[] = 
{
	/* The ground */
	{vec3(-5.0, -0.5, -5.0), vec3(5.0, 0.0, 5.0), {0.7, 0, vec3(1, 0, 0)}},
	/* Box in the middle */
	{vec3(-0.5, 0.0, -0.5), vec3(0.5, 1.0, 0.5), {0.2, 1, vec3(0.2, 0, 0.5)}}
};

const sphere spheres[] =
{
	{vec3(0, 0.5, 0), 0.5, {0.1, 1, vec3(0, 0.3, 1)}}
};

const light lights[] =
{
	{vec3(cos(time / 10000.0 + PI * 0.75) * 4, 0.3, sin(time / 10000.0 + PI * 0.75) * 4), 32, vec3(1)},
	{vec3(cos(time / 10000.0) * 3, 0.7, sin(time / 10000.0) * 3), 48, vec3(1)}
};

struct hitinfo 
{
	vec2 delta;
	int bi;
	vec3 normal;
	vec3 tangent;
	material mat;
};

struct traceinfo
{
	vec3 position;
	vec3 direction;
	vec4 color;
	hitinfo h;
	bool hit;
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
vec2 intersectBox(vec3 origin, vec3 dir, const box b) 
{
	vec3 tMin = (b.min - origin) / dir;
	vec3 tMax = (b.max - origin) / dir;
	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
	return vec2(tNear, tFar);
}

vec2 intersectSphere(vec3 origin, vec3 dir, const sphere s)
{
	vec3 toCenter = s.center - origin;
	float cosa = dot(normalize(toCenter), dir);

	float a = length(toCenter);
	float a2 = a * a;
	float c2 = s.radius * s.radius;

	float fir = a * cosa;
	float sec = sqrt(a * a * cosa * cosa - a2 + c2);

	return vec2(fir - sec, fir + sec);
}

vec3 compute_tangent(vec3 normal)
{
	if (normal.x == 0 && normal.z == 0)
		return vec3(1, 0, 0);
	float a = normal.z / sqrt(normal.z * normal.z + normal.x * normal.x);
	float b = -normal.x / sqrt(normal.z * normal.z + normal.x * normal.x);
	return vec3(a, 0, b);
}

const float margin = 0.01;
bool intersectPrimitives(vec3 origin, vec3 dir, out hitinfo info) 
{
	float smallest = MAX_SCENE_BOUNDS;
	bool found = false;
	for (int i = 0; i < BOX_COUNT; ++i) 
	{
		vec2 delta = intersectBox(origin, dir, boxes[i]);
		if (delta.x > 0.0 && delta.x < delta.y && delta.x < smallest) 
		{	
			info.delta = delta;
			info.bi = i;

			vec3 inter = origin + delta.x * dir;
			vec3 center = mix(boxes[i].min, boxes[i].max, 0.5);
			info.normal = inter - center;

			float rx = abs(info.normal.x) / (boxes[i].max.x - boxes[i].min.x) * 2;
			float ry = abs(info.normal.y) / (boxes[i].max.y - boxes[i].min.y) * 2;
			float rz = abs(info.normal.z) / (boxes[i].max.z - boxes[i].min.z) * 2;

			float sx = abs(info.normal.x) / info.normal.x;
			float sy = abs(info.normal.y) / info.normal.y;
			float sz = abs(info.normal.z) / info.normal.z;
			
			if (abs(info.normal.x) * 2 + margin < boxes[i].max.x - boxes[i].min.x)
				rx = 0;
			if (abs(info.normal.y) * 2 + margin < boxes[i].max.y - boxes[i].min.y)
				ry = 0;
			if (abs(info.normal.z) * 2 + margin < boxes[i].max.z - boxes[i].min.z)
				rz = 0;
			
			info.normal = vec3(sx * rx, sy * ry, sz * rz);

			info.tangent = compute_tangent(info.normal);

			info.mat = boxes[i].mat;

			smallest = delta.x;
			found = true;
		}
	}
	for (int i = 0; i < SPHERE_COUNT; ++i)
	{
		vec2 delta = intersectSphere(origin, dir, spheres[i]);
		if (delta.x > 0.0 && delta.x < delta.y && delta.x < smallest) 
		{	
			info.delta = delta;
			info.bi = i + BOX_COUNT;
			info.normal = normalize(origin + (delta.x * dir) - spheres[i].center);
			info.tangent = compute_tangent(info.normal);
			info.mat = spheres[i].mat;
			smallest = delta.x;
			found = true;
		}
	}
	return found;
}

bool CheckVisibility(vec3 origin, vec3 dir, float length, int bi)
{
	for (int i = 0; i < BOX_COUNT; ++i)
	{
		vec2 delta = intersectBox(origin, dir, boxes[i]);
		if (delta.x > margin && delta.x < length + margin) 
			return false;
	}

	for (int i = 0; i < SPHERE_COUNT; ++i)
	{
		vec2 delta = intersectSphere(origin, dir, spheres[i]);
		if (delta.x > margin && delta.x < length + margin) 
			return false;
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
	float k = roughness * sqrt(2 / PI);
	return d / (d * (1 - k) + k);
}

float shadowing_smith(float roughness, float n_v, float n_l)
{
	return shadowing_schlick(roughness, n_v) * shadowing_schlick(roughness, n_l);
}

float computeD(hitinfo h, float cos_theta)
{
	float roughness_2 = pow(h.mat.roughness, 2);
	float r = (roughness_2 - 1) * pow(cos_theta, 2) + 1;
	float D = roughness_2 / (PI * pow(r, 2));
	if (isnan(D))
		D = 1;
	return D;
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
	return vec3(0.05);
}

float pdf(float D, float cos_theta, vec3 wo)
{
	return max(D * cos_theta, 0.00000001);
}

vec3 light_specular(vec3 position, hitinfo h, vec3 wi, vec3 wo, int k)
{
	vec3 half_normal = normalize(wi + wo);
	float cos_theta = max(0, dot(half_normal, h.normal));

	vec4 res = brdf(wo, wi, cos_theta, h, half_normal);
	vec3 brdf_color = res.rgb;

	float distance = length(lights[k].position - position);
	float pdf = pdf(res.a, cos_theta, wi);

	return (brdf_color * lights[k].color * dot(h.normal, wi) * lights[k].intensity) / (distance * distance);
}

vec3 light_diffuse(vec3 position, hitinfo h, vec3 wi, int k)
{
	float distance = length(lights[k].position - position);

	return (get_diffuse_color(h.mat) / PI * lights[k].color * dot(h.normal, wi) * lights[k].intensity) / (distance * distance);
}

vec3 reflect_specular(vec3 position1, vec3 position2, hitinfo h, vec3 wi, vec3 wo, vec3 color)
{
	vec3 half_normal = normalize(wi + wo);
	float cos_theta = max(0, dot(half_normal, h.normal));

	vec4 res = brdf(wo, wi, cos_theta, h, half_normal);
	vec3 brdf_color = res.rgb;

	float distance = length(position1 - position2);
	float pdf = pdf(res.a, cos_theta, wi);

	return (brdf_color * color) / (distance * distance);
}

vec3 reflect_diffuse(vec3 position1, vec3 position2, hitinfo h, vec3 color)
{
	float distance = length(position1 - position2);

	return (get_diffuse_color(h.mat) / PI * color) / (distance * distance);
}

vec4 trace(vec3 origin, vec3 dir, int bounces);

vec4 getContributions(vec3 position, hitinfo h, vec3 wo)
{
	vec3 color = vec3(0);
	for (int i = 0; i < LIGHT_COUNT; ++i)
	{
		vec3 wi = normalize(lights[i].position - position);
		//return vec4(position, 1);
		//return vec4(vec3(h.delta.y), 1);
		if (CheckVisibility(position, wi, length(position - lights[i].position), h.bi))
		{
			color += light_specular(position, h, wi, wo, i);
			color += light_diffuse(position, h, wi, i);
		}
	}

	color += h.mat.color * ambient_contribution();

	return vec4(color, 1);
}

traceinfo trace(vec3 origin, vec3 dir) 
{
	hitinfo h;
	if (intersectPrimitives(origin, dir, h)) 
	{
		vec3 position = origin + dir * h.delta.x;
		vec4 color = getContributions(position, h, -dir);

		vec3 bitangent = cross(h.normal, h.tangent);
		mat3 TBN = mat3(h.tangent, h.normal, bitangent);

		vec3 half_vector = halfvector_GGX_sample(h.mat.roughness);
		vec3 half_vector_transformed = normalize(TBN * half_vector);

		vec3 wi = reflect(dir, half_vector_transformed);

		float cos_theta = dot(h.normal, half_vector_transformed);

		vec3 resultColor = color.rgb;

		return traceinfo(position, wi, vec4(resultColor, 1), h, true);
	}
	return traceinfo(vec3(0), vec3(0), vec4(0.05, 0.05, 0.05, 1.0), h, false);
}

float attenuation(vec3 position1, vec3 position2)
{
	float len = length(position1 - position2);
	return len * len;
}

vec4 accumulateColor(vec3 origin, vec3 dir)
{
	//traceinfo tis[bounces];
	vec3 color = vec3(0);
	for (int i = 0; i < bounces; ++i)
	{
		traceinfo ti = trace(origin, dir);
		origin = ti.position;
		dir = ti.direction;

		if (ti.hit || i == 0)
			color += ti.color.rgb / (i + 1);

		//tis[i] = ti;
		if (!ti.hit)
			break;
	}

	//for (int i = 0; i < bounces - 1; ++i)
	//{
	//	if (!tis[i].hit)
	//		break;
	//	//color += reflect_diffuse(tis[i].position, tis[i + 1].position, tis[i].h, tis[i + 1].color.rgb) / (i + 1);
	//
	//	vec3 wo = normalize(origin - tis[0].position);
	//	if (i > 0)
	//		wo = normalize(tis[i - 1].position - tis[i].position);
	//	vec3 wi = normalize(tis[i].position - tis[i + 1].position);
	//	color += reflect_specular(tis[i].position, tis[i + 1].position, tis[i].h, wi, wo, tis[i + 1].color.rgb) / (i + 1);
	//}
	return vec4(color, 1);
}
//================================================================================

//===================================MAIN=========================================
layout (local_size_x = 8, local_size_y = 8) in;
void main(void) 
{
	//lights[0].position = vec3(cos(time / 1000), 0.3, sin(time / 1000));
	pix = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(framebuffer);
	
	if (pix.x >= size.x || pix.y >= size.y) 
		return;
	
	vec2 pos = vec2(pix) / vec2(size.x - 1, size.y - 1);
	vec3 dir = normalize(mix(mix(ray00, ray01, pos.y), mix(ray10, ray11, pos.y), pos.x));
	vec4 color = accumulateColor(eye, dir);
	imageStore(framebuffer, pix, color);
}
//================================================================================
