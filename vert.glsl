// Use OpenGL 3.3
#version 330 core
uniform float aspect_ratio;
// Include the vector attribute at location 0. Save it as `pos`.
layout (location = 0) in vec3 pos;
out vec2 uv;
void main () {
    uv = vec2(pos.x * aspect_ratio, pos.y);
    // uv = pos.xy;
    gl_Position = vec4(pos.xyz, 1.0);
}