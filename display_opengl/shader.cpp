#include "shader.hpp"
#include <string>
#include <iostream>
#include <fstream>

static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage);
static std::string LoadShader(const std::string& fileName);
static GLuint CreateShader(const std::string& text, GLenum shaderType);
void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    // Print or log the debug message
    std::cerr << "OpenGL Debug Message: " << message << std::endl;
}

Shader::Shader(const std::string& fileName){
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLDebugCallback, nullptr);

    m_program = glCreateProgram();
    m_shaders[0] = CreateShader(LoadShader(fileName + ".vs"), GL_VERTEX_SHADER);
    m_shaders[1] = CreateShader(LoadShader(fileName + ".fs"), GL_FRAGMENT_SHADER);
    CheckShaderError(m_shaders[0], GL_COMPILE_STATUS, false, "Error: Vertex shader compilation failed: ");
    CheckShaderError(m_shaders[1], GL_COMPILE_STATUS, false, "Error: Fragment shader compilation failed: ");
    
    for(unsigned int i = 0; i < NUM_SHADERS; i++){
        glAttachShader(m_program, m_shaders[i]);
    }

    //glBindAttribLocation(m_program, 0, "position");

    glLinkProgram(m_program);
    CheckShaderError(m_program, GL_LINK_STATUS, true, "Error: Program shader program linking failed: ");
    
    glValidateProgram(m_program);
    CheckShaderError(m_program, GL_VALIDATE_STATUS, true, "Error: Program shader program is invalid: ");
}

Shader::~Shader(){

    for(unsigned int i = 0; i < NUM_SHADERS; i++){
        glDetachShader(m_program, m_shaders[i]);
        glDeleteShader(m_shaders[i]);
    }

    glDeleteProgram(m_program);
}

void Shader::Bind(){
    glUseProgram(m_program);
}

static GLuint CreateShader(const std::string& text, GLenum shaderType){
    GLuint shader = glCreateShader(shaderType);
    if (shader == 0)
        std::cerr << "Error: Shader creation failed!" << std::endl;

    const GLchar* shaderSourceStrings[1];
    GLint shaderSourceStringsLenghts[1];
    shaderSourceStrings[0] = text.c_str();
    shaderSourceStringsLenghts[0] = text.length();

    //std::cout << "Shader Source:\n" << text << std::endl;

    glShaderSource(shader, 1, shaderSourceStrings, shaderSourceStringsLenghts);
    glCompileShader(shader);

    CheckShaderError(shader, GL_COMPILE_STATUS, false, "Error: Program shader program compilation failed: ");

    return shader;
}

static std::string LoadShader(const std::string &filename){
    std::ifstream file;
    file.open((filename).c_str());

    std::string output;
    std::string line;
    if(file.is_open()){
        while(file.good()){
            getline(file, line);
            output.append(line + "\n");
        }
    }
    else{
        std::cerr << "Unable to load/open shader: " << filename << std::endl;
    }
    return output;
}

static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage){
    GLint success = 0;
    GLchar error[1024] = { 0 };

    if (isProgram)
        glGetProgramiv(shader, flag, &success);
    else
        glGetProgramiv(shader, flag, &success);
    
    if (success == GL_FALSE){
        if (isProgram)
            glGetProgramInfoLog(shader, sizeof(error), NULL, error);
        else
            glGetShaderInfoLog(shader, sizeof(error), NULL, error);
        
        std::cerr << errorMessage << ": '" << error << "'" << std::endl;
    }
}