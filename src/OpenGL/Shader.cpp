#include "Shader.h"

#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader() {
    m_Program = glCreateProgram();
}

Shader::Shader(const std::string& path, bool debug) : m_Path(path), m_Debug(debug){
    m_Program = glCreateProgram();
    std::ifstream shaderFile(path);
    if(!shaderFile.is_open()){
        return;
    }
    std::string line;
    int type = -1;

    std::string vertexSource;
    std::string fragmentSource;
    std::string geometrySource;
    std::string tessControlSource;
    std::string tessEvalSource;
    std::string computeSource;

    while (std::getline(shaderFile, line)) {
        if(line.find("#vertex") != std::string::npos){
            type = 0;
        }else if(line.find("#fragment") != std::string::npos){
            type = 1;
        }else if(line.find("#geometry") != std::string::npos ){
            type = 2;
        }else if(line.find("#tessControl")!= std::string::npos ){
            type = 3;
        }else if(line.find("#tessEval")!= std::string::npos ){
            type = 4;
        }else if(line.find("#tessEval")!= std::string::npos ){
            type = 5;
        }else{
            switch (type) {
                case 0:
                    vertexSource.append(line + "\n");
                    break;
                case 1:
                    fragmentSource.append(line + "\n");
                    break;
                case 2:
                    geometrySource.append(line + "\n");
                    break;
                case 3:
                    tessControlSource.append(line + "\n");
                    break;
                case 4:
                    tessEvalSource.append(line + "\n");
                    break;
                case 5:
                    computeSource.append(line + "\n");
                    break;
            }
        }

    }
    if (!vertexSource.empty()) {
        m_Vertex = compileShader(GL_VERTEX_SHADER, vertexSource.data());
        glAttachShader(m_Program, m_Vertex);
    }
    if (!fragmentSource.empty()) {
        m_Fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource.data());
        glAttachShader(m_Program, m_Fragment);
    }
    if (!geometrySource.empty()) {
        m_Geometry = compileShader(GL_GEOMETRY_SHADER, geometrySource.data());
        glAttachShader(m_Program, m_Geometry);
    }
    if (!tessControlSource.empty()) {
        m_TessellationControl = compileShader(GL_TESS_CONTROL_SHADER, tessControlSource.data());
        glAttachShader(m_Program, m_TessellationControl);
    }
    if (!tessEvalSource.empty()) {
        m_TessellationEvaluation = compileShader(GL_TESS_EVALUATION_SHADER, tessEvalSource.data());
        glAttachShader(m_Program, m_TessellationEvaluation);
    }
    if (!computeSource.empty()) {
        m_Compute = compileShader(GL_COMPUTE_SHADER, computeSource.data());
        glAttachShader(m_Program, m_Compute);
    }
    glLinkProgram(m_Program);
    glValidateProgram(m_Program);
    glUseProgram(m_Program);
}

Shader::~Shader() {
    glUseProgram(0);
    if (m_Vertex != -1)
        glDetachShader(m_Program, m_Vertex);
    if (m_Fragment != -1)
        glDetachShader(m_Program, m_Fragment);
    if (m_Geometry != -1)
        glDetachShader(m_Program, m_Geometry);
    if (m_TessellationControl != -1)
        glDetachShader(m_Program, m_TessellationControl);
    if (m_TessellationEvaluation != -1)
        glDetachShader(m_Program, m_TessellationEvaluation);
    if (m_Compute != -1)
        glDetachShader(m_Program, m_Compute);
    glDeleteProgram(m_Program);
    m_Locations.clear();
}

unsigned int Shader::compileShader(unsigned int type, const char *source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *) malloc(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        if(m_Debug){
            if (type == GL_VERTEX_SHADER) {
                std::cout << "Vertex shaders Error: " << message << std::endl;
            } else if (type == GL_FRAGMENT_SHADER) {
                std::cout << "Fragment shaders Error: " << message << std::endl;
            } else if (type == GL_GEOMETRY_SHADER) {
                std::cout << "Geometry shaders Error: " << message << std::endl;
            } else if (type == GL_TESS_CONTROL_SHADER) {
                std::cout << "Tessellation Control shaders Error: " << message << std::endl;
            } else if (type == GL_TESS_EVALUATION_SHADER) {
                std::cout << "Tessellation Evaluation shaders Error: " << message << std::endl;
            } else if (type == GL_COMPUTE_SHADER) {
                std::cout << "Compute shaders Error: " << message << std::endl;
            }
        }
        glDeleteShader(id);
        return 0;
    }
    return id;
}

void Shader::Bind() {
    glUseProgram(m_Program);
}

void Shader::SetUniformInt(const char *name, int value) {
    glUniform1i(glGetUniformLocation(m_Program, name), value);
}

void Shader::SetUniformIntArray( const char *name, int size, const int *data) {
    glUniform1iv(glGetUniformLocation(m_Program, name), size, data);
}

void Shader::SetUniformFloat( const char *name, float value) {
    glUniform1f(glGetUniformLocation(m_Program, name), value);
}

void Shader::SetUniformFloatArray(const char *name, int size, const float *data) {
    glUniform1fv(glGetUniformLocation(m_Program, name), size, data);
}

void Shader::SetUniformVec2( const char *name, const  Gem::fVec2 &vec2) {
    glUniform2f(glGetUniformLocation(m_Program, name), vec2.x, vec2.y);
}

void Shader::SetUniformVec2Array(const char *name, int size, const float *data) {
    glUniform2fv(glGetUniformLocation(m_Program, name), size, data);
}

void Shader::SetUniformVec3(const char *name, const  Gem::fVec3 &vec3) {
    glUniform3f(glGetUniformLocation(m_Program, name), vec3.x, vec3.y, vec3.z);
}

void Shader::SetUniformVec3Array( const char *name, int size, const float *data) {
    glUniform3fv(glGetUniformLocation(m_Program, name), size, data);
}

void Shader::SetUniformVec4(const char *name, const  Gem::fVec4 &vec4) {
    glUniform4f(glGetUniformLocation(m_Program, name), vec4.x, vec4.y, vec4.z, vec4.w);
}

void Shader::SetUniformVec4Array(const char *name, int size, const float *data) {
    glUniform4fv(glGetUniformLocation(m_Program, name), size, data);
}

void Shader::SetUniformBool(const char *name, bool value) {
    glUniform1i(glGetUniformLocation(m_Program, name), (GLint) value);
}

void Shader::SetUniformBoolArray(const char *name, int size, const bool *data) {
    glUniform1iv(glGetUniformLocation(m_Program, name), size, (GLint*) data);
}

void Shader::SetUniformMatrix4f(const char *name, const  Gem::fMat4 &mat) {
    glUniformMatrix4fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE, mat.data);
}
