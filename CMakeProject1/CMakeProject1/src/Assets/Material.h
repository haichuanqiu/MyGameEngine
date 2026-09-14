#pragma once
#include <string>
#include <variant>
#include "rendering/ShaderManager.h"
#include "AssetManager.h"
#include "Serialization/ClassRegistry.h"
#include "ReferenceResolver.h"
using MaterialValue = std::variant<
	bool,
	int,
	float,
	glm::vec4,
	glm::mat4
>;
class MaterialParameter
{
public:
     MaterialParameter() = default;
     ~MaterialParameter() = default;

private:
	std::string AttributeName;
	MaterialValue value;
};

class Material :public Asset
{
public:

     Material() = default;


public:

     // ============================================================
     // Shader
     // ============================================================

     bool setVertShaderPath(const char* vertexPath)
     {
          if (vertexPath == nullptr)
          {
               return false;
          }

          vertShaderPath = vertexPath;

          return setShader();
     }


     bool setFragShaderPath(const char* fragmentPath)
     {
          if (fragmentPath == nullptr)
          {
               return false;
          }

          fragShaderPath = fragmentPath;

          return setShader();
     }
     Shader* shader = nullptr;


     bool setShader()
     {
          // Both paths must exist before
          // trying to build a shader.

          if (vertShaderPath.empty() ||
               fragShaderPath.empty())
          {
               return false;
          }


          shader = ShaderManager::Instance().tryBuildShader(
               vertShaderPath.c_str(),
               fragShaderPath.c_str()
          );


          return shader != nullptr;
     }

    

private:

     // ============================================================
     // Shader
     // ============================================================


     // ============================================================
     // Shader Paths
     // ============================================================

     std::string vertShaderPath;

     std::string fragShaderPath;

     int number=1;
     // ============================================================
     // Material Parameters
     // ============================================================

     std::vector<MaterialParameter> parameters;
     REFLECT_FRIEND(Material);
};
REFLECT_BASE(
     Material,
     Asset,
     FIELD(Material, number),
     FIELD(Material, parameters),
     FIELD(Material, vertShaderPath),
     FIELD(Material, fragShaderPath)
)
REGISTER_CLASS(Material)