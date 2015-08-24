#pragma once

#include <iro/include.hpp>

#include <nyutil/vec.hpp>
#include <nyutil/mat.hpp>

//shader
class shader
{
protected:
    bool compile(const std::string& vertexShader, const std::string& fragmentShader);

    unsigned int program_;

public:
    enum type
    {
        fragment,
        vertex
    };

    shader();
    ~shader();

    bool loadFromFile(const std::string& vertexFile, const std::string& fragmentFile);
    bool loadFromFile(const std::string& file, type type);

    bool loadFromString(const std::string& vertexShader, const std::string& fragmentShader);
    bool loadFromString(const std::string& shader, type type);

    void setUniformParameter(const std::string& name, float value);
    void setUniformParameter(const std::string& name, float x, float y);
    void setUniformParameter(const std::string& name, float x, float y, float z);
    void setUniformParameter(const std::string& name, float x, float y, float z, float w);

    void setUniformParameter(const std::string& name, const vec2f& value);
    void setUniformParameter(const std::string& name, const vec3f& value);
    void setUniformParameter(const std::string& name, const vec4f& value);

    void setUniformParameter(const std::string& name, const mat2f& value);
    void setUniformParameter(const std::string& name, const mat3f& value);
    void setUniformParameter(const std::string& name, const mat4f& value);

    void setUniformParameter(const std::string& name, const mat23f& value);
    void setUniformParameter(const std::string& name, const mat24f& value);
    void setUniformParameter(const std::string& name, const mat32f& value);
    void setUniformParameter(const std::string& name, const mat34f& value);
    void setUniformParameter(const std::string& name, const mat42f& value);
    void setUniformParameter(const std::string& name, const mat43f& value);

    unsigned int getProgram() const { return program_; }
    void use() const;
};