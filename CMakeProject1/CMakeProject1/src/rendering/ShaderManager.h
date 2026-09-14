#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <unordered_map>

#include "rendering/shader.h"

class ShaderManager
{
public:

     static ShaderManager& Instance()
     {
          static ShaderManager instance;

          return instance;
     }


     ShaderManager(const ShaderManager&) = delete;
     ShaderManager& operator=(const ShaderManager&) = delete;


public:

     // ============================================================
     // Build / Get Shader
     // ============================================================

     Shader* tryBuildShader(
          const char* vertexPath,
          const char* fragmentPath
     )
     {
          if (vertexPath == nullptr ||
               fragmentPath == nullptr)
          {
               return nullptr;
          }


          std::string key =
               std::string(vertexPath) +
               "|" +
               std::string(fragmentPath);


          // ========================================================
          // Already Exists
          // ========================================================

          auto it = shaders.find(key);

          if (it != shaders.end())
          {
               return it->second.get();
          }


          // ========================================================
          // Build New Shader
          // ========================================================

          auto shader = std::make_unique<Shader>(
               vertexPath,
               fragmentPath
          );

          Shader* result = shader.get();

          shaders.emplace(
               std::move(key),
               std::move(shader)
          );

          return result;
     }


     Shader* getShader(
          const char* vertexPath,
          const char* fragmentPath
     )
     {
          if (vertexPath == nullptr ||
               fragmentPath == nullptr)
          {
               return nullptr;
          }


          std::string key =
               std::string(vertexPath) +
               "|" +
               std::string(fragmentPath);


          auto it = shaders.find(key);

          if (it == shaders.end())
          {
               return nullptr;
          }

          return it->second.get();
     }


     void clear()
     {
          shaders.clear();
     }


private:

     ShaderManager() = default;

     ~ShaderManager() = default;


private:

     std::unordered_map<
          std::string,
          std::unique_ptr<Shader>
     > shaders;
};

