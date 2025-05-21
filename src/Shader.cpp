#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <spdlog/spdlog.h>
#include <algorithm>

static void processUniformNode(std::vector<UniformInfo>& uniforms, const std::string& name, GLenum type, GLint size);
static void checkCompileErrors(GLuint shader, GLenum type);

//TODO asercje zamiast exceptions, printf zamiast cout, nie korzystać z stringstreama

static GLuint loadShader(const std::string path, GLenum type)
{
    spdlog::info("Loading shader from file: {}", path);
    std::string code;
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        file.open(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        code = stream.str();
    }
    catch (std::ifstream::failure e)
    {
        spdlog::error("Shader file not successfully read");
    }
    const char* codeCStr = code.c_str();
    GLuint shader = glCreateShader(type);


    glShaderSource(shader, 1, &codeCStr, NULL);
    glCompileShader(shader);
    checkCompileErrors(shader, type);

    return shader;
}


Shader::Shader(const std::string& name, const std::unordered_map<GLenum, std::string>& shaders) : name{ name }
{
    ID = glCreateProgram();
    std::vector<GLuint> shaderIDs;
    for (auto& shader : shaders)
    {
        GLuint shaderID = loadShader(shader.second, shader.first);
        glAttachShader(ID, shaderID);
        shaderIDs.push_back(shaderID);
    }
    glLinkProgram(ID);

    checkCompileErrors(ID, GL_PROGRAM);

    // Cleanup
    for (auto& shaderID : shaderIDs)
    {
        glDeleteShader(shaderID);
    }

    // Get uniforms
    GLint numUniforms;
    glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &numUniforms);
    for (GLint i = 0; i < numUniforms; ++i)
    {
        GLchar name[256];
        GLenum type;
        GLint size;

        glGetActiveUniform(ID, i, sizeof(name), NULL, &size, &type, name);

        processUniformNode(uniforms, name, type, size);
    }
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

static void processUniformNode(std::vector<UniformInfo>& uniforms, const std::string& name, GLenum type, GLint size)
{
    bool leaf = true;
    std::string nodeName = name;
    GLint nodeSize = size;

    int dotPos = nodeName.find_first_of('.');
    if (dotPos != std::string::npos)
    {
        leaf = false;
        nodeName = nodeName.substr(0, dotPos);
    }

    int leftBracketPos = nodeName.find_first_of('[');
    int rightBracketPos = nodeName.find_first_of(']');
    if (leftBracketPos != std::string::npos && rightBracketPos != std::string::npos && leftBracketPos < rightBracketPos)
    {
        if (!leaf)
        {
            nodeSize = std::stoi(nodeName.substr(leftBracketPos + 1, rightBracketPos - leftBracketPos - 1)) + 1;
        }
        nodeName = nodeName.substr(0, leftBracketPos);
    }

    if (!leaf)
    {
        auto it = std::find_if(uniforms.begin(), uniforms.end(), [&nodeName](const UniformInfo& uniform) {
            return uniform.name == nodeName;
            });
        if (it != uniforms.end())
        {
            if (it->size < nodeSize)
            {
                it->size = nodeSize;
            }
            processUniformNode(it->members, name.substr(dotPos + 1), type, size);
        }
        else
        {
            UniformInfo newUniform;
            newUniform.name = nodeName;
            newUniform.size = nodeSize;
            processUniformNode(newUniform.members, name.substr(dotPos + 1), type, size);
            uniforms.push_back(newUniform);
        }
    }
    else
    {
        auto it = std::find_if(uniforms.begin(), uniforms.end(), [&nodeName](const UniformInfo& uniform) {
            return uniform.name == nodeName;
            });
        if (it == uniforms.end())
        {
            UniformInfo newUniform;
            newUniform.name = nodeName;
            newUniform.type = type;
            newUniform.size = nodeSize;
            uniforms.push_back(newUniform);
        }
    }

}

// Use shader
void Shader::use() const
{
    glUseProgram(ID);
}

// Uniform setting functions
void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

static const std::unordered_map<GLenum, std::string> typeNames = {
        {GL_VERTEX_SHADER, "VERTEX"},
        {GL_FRAGMENT_SHADER, "FRAGMENT"},
        {GL_GEOMETRY_SHADER, "GEOMETRY"},
        {GL_TESS_CONTROL_SHADER, "TESS_CONTROL"},
        {GL_TESS_EVALUATION_SHADER, "TESS_EVALUATION"},
        {GL_COMPUTE_SHADER, "COMPUTE"},
        {GL_PROGRAM, "PROGRAM"}
};

// Error checking
void checkCompileErrors(GLuint shader, GLenum type)
{
    GLint success;
    GLchar infoLog[1024];

    if (type != GL_PROGRAM)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << typeNames.at(type) << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << typeNames.at(type) << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
