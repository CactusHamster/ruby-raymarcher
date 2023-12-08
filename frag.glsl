// Use OpenGL 3.3.0
#version 330 core
precision lowp float;
in vec2 uv;
out vec4 color;

#define RADIUS 1
#define ITERATIONS 200

float SDF (vec3 pos) {
    return sqrt(pos.x*pos.x + pos.y*pos.y + pos.z * pos.z) - RADIUS;
}
float trace (vec3 start_position, vec3 direction) {
    float distance_travelled = 0.0;
    int steps;
    for (steps = 0; steps < ITERATIONS; steps++) {
        vec3 p = start_position + distance_travelled * direction;
        float dist = SDF(p);
        distance_travelled += dist;
        if (dist < 0.01) break;
    }
    return 1.0 - (float(steps)) / (float(ITERATIONS));
}

uniform vec3 camera;
void main () {
    // gl_FragCoord
    // vec3 from = vec3(0.0, 0.0, -2.0);
    // vec3 to = vec3(uv.x, uv.y, -1.0);
    if ((uv.x * uv.x) + (uv.y * uv.y) < 0.2 * 0.2) {
        color = vec4(camera.x, 1.0, 1.0, 1.0);
    } else {
        color = vec4(1.0, 1.0, 1.0, 1.0);
    }
}