// Use OpenGL 3.3.0
#version 330 core
precision lowp float;
in vec2 vertex_position;
out vec4 color;

// Camera position.
uniform vec3 camera;
// Coordinate in quad space (0 to 1). gl_FragCoord is 0 to w and 0 to h.
uniform float aspect_ratio;

#define RADIUS 1
#define ITERATIONS 200
#define GAMMA 2.2

float SDF (vec3 pos) {
    return length(pos) - RADIUS;
}

float trace (vec3 start_position, vec3 direction) {
    int i;
    float travelled = 0;
    vec3 current_position = start_position;
    for (i = 0; i < ITERATIONS; i++) {
        float distance_i_can_travel = SDF(current_position);
        if (distance_i_can_travel < 0.0001) break;
        vec3 step_vector = distance_i_can_travel * direction;
        travelled += distance_i_can_travel;
        current_position += step_vector;
    }
    return float(i) / ITERATIONS;
}

void main () {
    vec2 uv = vec2(vertex_position.x * aspect_ratio, vertex_position.y);
    vec3 direction = normalize(vec3(uv, 1.0));
    float n = trace(camera, direction);

    // Raise n to inverse gamma power (gamma correction). Gamma is bbout `2.2`.
    n = pow(n, 1.0 / GAMMA);
    color = vec4(n, n, n, 1.0);
}