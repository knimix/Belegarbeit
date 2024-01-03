#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 fragment_uv;
layout (location = 1) out vec3 fragment_Color;

layout (set = 0, binding = 0) uniform transform{
    mat4 view;
    mat4 projection;
} u_transforms;

layout (push_constant) uniform pushConstants{
    mat4 matrix;
} camera;


struct ObjectData{
    mat4 model;
    vec3 color;
};

//all object matrices
layout(std430,set = 0, binding = 1) readonly buffer ObjectBuffer{

    ObjectData objects[];
} objectBuffer;


void main(){
    fragment_uv = uv;
   gl_Position =  camera.matrix * objectBuffer.objects[gl_InstanceIndex].model *  vec4(position, 1.0);
   fragment_Color =  objectBuffer.objects[gl_InstanceIndex].color;
    //objectBuffer.objects[gl_InstanceIndex].model.x;
    //gl_Position = objectBuffer.objects[gl_InstanceIndex].model * vec4(position,1.0);
}
