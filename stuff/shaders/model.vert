#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(push_constant) uniform PushConstant
{
    mat4 vpMatrix;
} pc;

layout(set = 1, binding = 0) uniform UniformBufferObject
{
    mat4 model;
    int textured;
} ubo;

layout(location = 0) out vec4 color;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out int textured;
layout(location = 3) out vec3 outNormal;

void main() {
    gl_Position = pc.vpMatrix * ubo.model * vec4(inVertex, 1.0);
    color = inColor;
    texCoord = inTexCoord;
    textured = ubo.textured;
    outNormal = inNormal;
}
