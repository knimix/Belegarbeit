#vertex
#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec3 fragment_Color;


uniform mat4 camera;

struct ObjectData{
    mat4 model;
    vec3 color;
};

//all object matrices
layout(std430, binding = 0) buffer ObjectBuffer{

    ObjectData objects[];
} objectBuffer;


void main(){
    gl_Position =  camera * objectBuffer.objects[gl_InstanceID].model *  vec4(position, 1.0);
    fragment_Color =  objectBuffer.objects[gl_InstanceID].color;
}

#fragment
#version 430 core


out vec4 color;
in vec3 fragment_Color;



void main(){
    color = vec4(fragment_Color,1.0);
}