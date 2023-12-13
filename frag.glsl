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

#define EPSILON 0.0001
#define ITERATIONS 120
#define GAMMA 2.2
#define BACKGROUND_COLOR vec3(0.0, 0.0, 0.0);
#define AMBIENT_OCCLUSION_STRENGTH 0.5

// Sphere.
float sdf_sphere (vec3 pos, float radius) {
    return length(pos) - radius;
}

float sdf_plane () {
    return 0.0;
}

// Mandelbulb.
float sdf_mandelbulb(vec3 pos, int iterations, float bailout, float power) {
	vec3 z = pos;
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
    // pos.xyz = mod(pos.xyz, 5.0) - 5.0 / 2.0;
    return sdf_mandelbulb(pos, 25, 6.0, 9.0);
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

vec3 calc_ray_direction () {
    // Direction of the vector to pass through this pixel. I know using the aspect ratio as the third component is weird.
    vec3 direction = normalize(vec3(
        vertex_position.x * aspect_ratio,
        vertex_position.y,
        max(aspect_ratio, 1 / aspect_ratio)
    ));
    apply_rotations_to(direction);
    return direction;
}

float ambient_occlusion (int raymarcher_iterations) {
    return 1.0 - smoothstep(0.0, 1.0, float(raymarcher_iterations) / ITERATIONS);
}

vec3 gamma_correction (vec3 color) {
    return pow(color, vec3(1.0 / GAMMA));
}

vec3 calc_color (int iterations, vec3 final_position) {
    vec3 color = vec3(0.2549, 0.8392, 1.0);
    color = color * vec3(ambient_occlusion(iterations));
    color = gamma_correction(color);
    return color;
}

void main () {
    // Ray marching.
    int iterations;
    vec3 final_position;
    trace(camera, calc_ray_direction(), iterations, final_position);
    
    // Coloring.
    vec3 color = calc_color(iterations, final_position);
    final_color = vec4(color, 1.0);
}