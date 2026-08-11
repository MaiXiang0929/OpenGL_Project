#version 400 core
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;
void main() {
    for (int i = 0; i < 3; ++i) {
        int j = (i + 1) % 3;
        gl_Position = gl_in[i].gl_Position; gl_Position.z -= 0.003 * gl_Position.w; EmitVertex();
        gl_Position = gl_in[j].gl_Position; gl_Position.z -= 0.003 * gl_Position.w; EmitVertex();
        EndPrimitive();
    }
}
