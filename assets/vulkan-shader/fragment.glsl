#version 450 core


layout(location = 0) out vec4 color;
layout(location = 0) in vec2 fragment_uv;
layout(location = 1) in vec3 fragment_Color;


layout(set = 0, binding = 0) uniform sampler2D in_texture;

void main(){
    //color = texture(in_texture, fragment_uv);
    color = vec4(fragment_Color,1.0);
}