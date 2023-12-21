// Use OpenGL 3.3.0
#version 330 core
precision lowp float;
in vec2 vertex_position;
out vec4 final_color;

// Camera position.
uniform vec3 camera;
// Pitch/yaw angles.
uniform vec2 rotation;
// Aspect ratio of GLFW window.
uniform float aspect_ratio;
// Height of the plane to walk on.
uniform float ground_height;
// Time since the render started.
uniform float time;

#define EPSILON 0.01
#define ITERATIONS 150 // 175
#define GAMMA 2.2
#define BACKGROUND_COLOR vec3(0.0, 0.0, 0.0);
#define AMBIENT_OCCLUSION_STRENGTH 0.7
#define PI 3.1415926535897932384626433832795
#define SPHERE_LUMINANCE 100000
#define SPHERE_COLOR vec3(1.0, 1.0, 1.0)

// https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// Sphere.
float sdf_sphere (vec3 pos, float radius) {
    // Repeat.
    return length(pos) - radius;
}

float sdf_torus(vec3 pos, vec2 t) {
  vec2 q = vec2(length(pos.xz)-t.x,pos.y);
  return length(q)-t.y;
}

float sdf_floor (vec3 pos) {
    return pos.y - ground_height + 11;
}

// Mandelbulb.
float sdf_mandelbulb(vec3 pos, int iterations, float bailout, float power) {
	vec3 z = pos / vec3(7.0);
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < iterations ; i++) {
		r = length(z);
		if (r>bailout) break;
		
		// convert to polar coordinates
		float theta = acos(z.z/r);
		float phi = atan(z.y,z.x);
		dr =  pow( r, power-1.0)*power*dr + 1.0;
		
		// scale and rotate the point
		float zr = pow( r,power);
		theta = theta*power;
		phi = phi*power;
		
		// convert back to cartesian coordinates
		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z+=pos;
	}
	return 0.5*log(r)*r/dr;
}

float sdf_cone( vec3 pos, vec2 c ) {
    // c is the sin/cos of the angle
    vec2 q = vec2( length(pos.xz), - pos.y );
    float d = length(q-c*max(dot(q,c), 0.0));
    return d * ((q.x*c.y-q.y*c.x<0.0)?-1.0:1.0);
}
float sdf_union(float sdf1, float sdf2) {
    return min(sdf1,sdf2);
}
float sdf_subtraction(float sdf1, float sdf2) {
    return max(-sdf1,sdf2);
}
float sdf_intersection(float sdf1, float sdf2) {
    return max(sdf1,sdf2);
}
float sdf_xor(float sdf1, float sdf2) {
    return max(min(sdf1,sdf2),-max(sdf1,sdf2));
}

float SDF (vec3 pos) {
    return sdf_union(
        sdf_union(
            sdf_floor(pos),
            sdf_sphere(mod(pos.xyz, 170.0) - 170.0 / 2, 10.0)
        ),
        sdf_mandelbulb(pos, 25, abs(mod(time, 4.0) - 2.0), 8.0)
    );
}

void trace (vec3 start_position, vec3 direction, out int i, out vec3 final_position) {
    // Iteration count.
    i = 0;
    // Initial position.
    vec3 current_position = start_position;
    for (i = 0; i < ITERATIONS; i++) {
        float distance_i_can_travel = SDF(current_position);
        if (distance_i_can_travel < EPSILON) break;
        vec3 step_vector = distance_i_can_travel * direction;
        current_position += step_vector;
    }
    final_position = current_position;
}

void apply_rotations_to(out vec3 direction) {
    float pitch = rotation.x;
    float yaw = rotation.y;
    direction = mat3(
        1.0, 0.0, 0.0,
        0.0, cos(pitch), sin(pitch),
        0.0, -sin(pitch), cos(pitch)
    ) * direction;
    direction = mat3(
        cos(yaw), 0.0, -sin(yaw),
        0.0, 1.0, 0.0,
        sin(yaw), 0.0, cos(yaw)
    ) * direction;
}

// Direction of the vector to pass through this pixel. I know using the aspect ratio as the third component is weird.
vec3 calc_ray_direction () {
    vec3 direction = normalize(vec3(
        vertex_position.x * aspect_ratio,
        vertex_position.y,
        max(aspect_ratio, 1 / aspect_ratio)
    ));
    apply_rotations_to(direction);
    return direction;
}

// Apply ambient occlusion to a color, based on the ray steps it took to reach the object.
float ambient_occlusion (int raymarcher_iterations) {
    return (1.0 - AMBIENT_OCCLUSION_STRENGTH * smoothstep(0.0, 1.0, float(raymarcher_iterations) / ITERATIONS));
}

// Apply gamma correction to a color.
vec3 gamma_correction (vec3 color) { return pow(color, vec3(1.0 / GAMMA)); }

// Calculate the main color of the scene.
vec3 calc_color (int iterations, vec3 final_position) {
    if (distance(camera, final_position) > 5000.0) return BACKGROUND_COLOR;
    // Main color.
    vec3 color = vec3(0.9, 0.9, 0.9 + abs(mod(time, 10.0) - 5.0) * 0.02);
    // Length of ray from final position to closest sphere.
    float light_dist = sdf_sphere(mod(final_position, 170.0) - 170.0 / 2, 10.0);
    // Apply sphere luminance.
    color = color * SPHERE_COLOR * vec3(SPHERE_LUMINANCE / (4.0 * PI * light_dist * light_dist));
    // AO
    color = color * vec3(ambient_occlusion(iterations));
    // Gamma correction
    color = gamma_correction(color);
    color = clamp(color, 0.0, 1.0);
    return color;
}

// March and color.
void main () {
    // Ray marching.
    int iterations;
    vec3 final_position;
    trace(camera, calc_ray_direction(), iterations, final_position);
    
    // Coloring.
    vec3 color = calc_color(iterations, final_position);
    final_color = vec4(color, 1.0);
}