#include <shader/shader.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "sources.hpp"

#include <fstream>
#include <vector>


bool shader::initialized_ = 0;
shader shader::rgb = shader();
shader shader::argb = shader();

bool shader::init()
{
    initialized_ = 1;


    return (argb.loadFromString(sourceVS, argbFS) &
            rgb.loadFromString(sourceVS, rgbFS));
}

//shader
shader::shader()
{
}

shader::~shader()
{
}

bool shader::loadFromFile(const std::string& vertexFile, const std::string& fragmentFile)
{
    std::string vertString;
    std::string fragString;

    std::ifstream vertStream;
    vertStream.open(vertexFile);
    if(vertStream.is_open())
    {
        std::string tmp = "";

        while(std::getline(vertStream, tmp))
            vertString += tmp + "\n";

        vertStream.close();
    }
    else
    {
        std::cout << "could not load vertexFile for shader" << std::endl;
        return 0;
    }

    std::ifstream fragStream;
    fragStream.open(fragmentFile);
    if(fragStream.is_open())
    {
        std::string tmp = "";

        while(std::getline(fragStream, tmp))
            fragString += tmp + "\n";

        fragStream.close();
    }
    else
    {
        std::cout << "could not load fragmentFile for shader" << std::endl;
        return 0;
    }

    return compile(vertString, fragString);
}
bool shader::loadFromFile(const std::string& file, type type)
{
    std::string source;
    std::string tmp = "";
    std::ifstream stream;
    stream.open(file);

    if(!stream.is_open())
    {
        return 0;
    }

    while(std::getline(stream, tmp))
    source += tmp + "\n";
    stream.close();

    if(type == vertex) return compile(source, std::string());
    if(type == fragment) return compile(std::string(), source);

    return 0;
}

bool shader::loadFromString(const std::string& vertexShader, const std::string& fragmentShader)
{
    return compile(vertexShader, fragmentShader);
}
bool shader::loadFromString(const std::string& shader, type type)
{
    if(type == vertex) return compile(shader, std::string());
    if(type == fragment) return compile(std::string(), shader);

    return 0;
}

void shader::setUniformParameter(const std::string& name, float value)
{
    int location = glGetUniformLocation(program_, name.c_str());
    glUniform1f(location, value);
}

void shader::setUniformParameter(const std::string& name, float x, float y)
{
    int location = glGetUniformLocation(program_, name.c_str());
    glUniform2f(location, x, y);
}
void shader::setUniformParameter(const std::string& name, float x, float y, float z)
{
    int location = glGetUniformLocation(program_, name.c_str());
    glUniform3f(location, x, y, z);
}
void shader::setUniformParameter(const std::string& name, float x, float y, float z, float w)
{
    int location = glGetUniformLocation(program_, name.c_str());
    glUniform4f(location, x, y, z, w);
}
void shader::setUniformParameter(const std::string& name, const vec2f& value)
{
    setUniformParameter(name, value.x, value.y);
}
void shader::setUniformParameter(const std::string& name, const vec3f& value)
{
    setUniformParameter(name, value.x, value.y, value.z);
}
void shader::setUniformParameter(const std::string& name, const vec4f& value)
{
    setUniformParameter(name, value.x, value.y, value.w);
}
void shader::setUniformParameter(const std::string& name, const mat2f& value)
{

}
void shader::setUniformParameter(const std::string& name, const mat3f& value)
{

}
void shader::setUniformParameter(const std::string& name, const mat4f& value)
{

}
void shader::setUniformParameter(const std::string& name, const mat23f& value)
{

}
void shader::setUniformParameter(const std::string& name, const mat24f& value)
{

}
void shader::setUniformParameter(const std::string& name, const mat32f& value)
{

}
void shader::setUniformParameter(const std::string& name, const mat34f& value)
{

}
void shader::setUniformParameter(const std::string& name, const mat42f& value)
{

}
void shader::setUniformParameter(const std::string& name, const mat43f& value)
{

}

bool shader::compile(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int vsID = 0;
    unsigned int fsID = 0;

    if(!vertexShader.empty())
    {
        vsID = glCreateShader(GL_VERTEX_SHADER);
        GLint result = 0;

        const char* const str = vertexShader.c_str();
        glShaderSource(vsID, 1, &str, nullptr);
        glCompileShader(vsID);

        glGetShaderiv(vsID, GL_COMPILE_STATUS, &result);
        if(result != 1)
        {
            int infoLength;
            glGetShaderiv(vsID, GL_INFO_LOG_LENGTH, &infoLength);
            std::vector<char> info(infoLength);
            glGetShaderInfoLog(vsID, infoLength, nullptr, info.data());
            std::cout << "failed to compile vertex shader:\n" << std::string(info.data()) << std::endl;
            vsID = 0;
        }
    }

    if(!fragmentShader.empty())
    {
        fsID = glCreateShader(GL_FRAGMENT_SHADER);
        GLint result = 0;

        const char* const str = fragmentShader.c_str();
        glShaderSource(fsID, 1, &str, nullptr);
        glCompileShader(fsID);

        glGetShaderiv(fsID, GL_COMPILE_STATUS, &result);
        if(result != 1)
        {
            int infoLength;
            glGetShaderiv(fsID, GL_INFO_LOG_LENGTH, &infoLength);
            std::vector<char> info(infoLength);
            glGetShaderInfoLog(fsID, infoLength, nullptr, info.data());
            std::cout << "failed to compile fragment shader:\n" << std::string(info.data()) << std::endl;
            fsID = 0;
        }
    }

    if(!vsID && !fsID) return 0;

    unsigned int progID = glCreateProgram();
    if(vsID)glAttachShader(progID, vsID);
    if(fsID)glAttachShader(progID, fsID);
    glLinkProgram(progID);

    if(vsID)glDeleteShader(vsID);
    if(fsID)glDeleteShader(fsID);

    program_ = progID;
    return 1;
}

void shader::use() const
{
    if(program_)
        glUseProgram(program_);
}


