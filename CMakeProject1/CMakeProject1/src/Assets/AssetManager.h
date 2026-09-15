#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "EngineObject.h"
#include "Serialization/Reflection.h"
#include "Serialization/ClassRegistry.h"
#include "Serialization/JsonSerializer.h"

class Asset : public EngineObject
{
public:
     virtual ~Asset() = default;


     std::string filePath;

     REFLECT_FRIEND(Asset);
};


REFLECT_BASE(
     Asset,
     EngineObject,
     FIELD(Asset, filePath),
     )


     class AssetManager
{
public:
     template<typename T>
     std::vector<T*> FindAllOfType()
     {
          std::vector<T*> result;

          for (auto& asset : m_Assets)
          {
               if (!asset)
                    continue;

               T* typedAsset =
                    dynamic_cast<T*>(asset.get());

               if (typedAsset)
               {
                    result.push_back(
                         typedAsset
                    );
               }
          }

          return result;
     }

     static int LoadContent(
          const std::string& content)
     {
          // =====================================================
          // TypeId
          // =====================================================

          const std::string typePrefix =
               "TypeId:";

          size_t typePos =
               content.find(
                    typePrefix
               );

          if (typePos == std::string::npos)
          {
               std::cerr
                    << "[AssetManager] TypeId not found."
                    << std::endl;

               return -1;
          }


          size_t typeStart =
               typePos +
               typePrefix.size();


          size_t typeEnd =
               content.find(
                    '\n',
                    typeStart
               );


          if (typeEnd == std::string::npos)
          {
               std::cerr
                    << "[AssetManager] Invalid TypeId."
                    << std::endl;

               return -1;
          }


          std::string typeString =
               content.substr(
                    typeStart,
                    typeEnd - typeStart
               );


          TypeId typeId = 0;


          try
          {
               typeId =
                    static_cast<TypeId>(
                         std::stoull(
                              typeString
                         )
                         );
          }
          catch (...)
          {
               std::cerr
                    << "[AssetManager] Invalid TypeId value."
                    << std::endl;

               return -1;
          }


          // =====================================================
          // Check Reflection
          // =====================================================

          const TypeInfo* typeInfo =
               ReflectionRegistry::Instance()
               .FindById(
                    typeId
               );


          if (!typeInfo)
          {
               std::cerr
                    << "[AssetManager] Type not reflected. "
                    << "TypeId: "
                    << typeId
                    << std::endl;

               return -1;
          }


          // =====================================================
          // Create
          // =====================================================

          void* object =
               ClassRegistry::Instance()
               .Create(
                    typeId
               );


          if (!object)
          {
               std::cerr
                    << "[AssetManager] Type not registered. "
                    << "TypeId: "
                    << typeId
                    << std::endl;

               return -1;
          }


          Asset* asset =
               static_cast<Asset*>(
                    object
                    );


          // =====================================================
          // Data
          // =====================================================

          const std::string dataPrefix =
               "Data: ";


          size_t dataPos =
               content.find(
                    dataPrefix,
                    typeEnd
               );


          if (dataPos == std::string::npos)
          {
               delete asset;

               std::cerr
                    << "[AssetManager] Data not found."
                    << std::endl;

               return -1;
          }


          size_t dataStart =
               dataPos +
               dataPrefix.size();


          std::string data =
               content.substr(
                    dataStart
               );


          // =====================================================
          // Remove trailing newline
          // =====================================================

          while (
               !data.empty() &&
               (
                    data.back() == '\n' ||
                    data.back() == '\r'
                    )
               )
          {
               data.pop_back();
          }


          // =====================================================
          // Deserialize
          // =====================================================

          JsonSerializer::Deserialize(
               *asset,
               data
          );


          // =====================================================
          // Object ID
          // =====================================================

          AssetManager& manager =
               Instance();


          asset->ReferenceInfo.ObjectID =
               manager.m_NextObjectID++;


          int objectID =
               asset->ReferenceInfo.ObjectID;


          // =====================================================
          // Asset Manager Ownership
          // =====================================================

          manager.m_Assets.push_back(
               std::shared_ptr<Asset>(
                    asset
               )
          );


          return objectID;
     }

     static void Scan(
          const std::string& path)
     {
          namespace fs =
               std::filesystem;


          // =====================================================
          // Check Path
          // =====================================================

          if (!fs::exists(path))
          {
               std::cerr
                    << "[AssetManager] Scan path does not exist: "
                    << path
                    << std::endl;

               return;
          }


          // =====================================================
          // Scan Recursively
          // =====================================================

          for (
               const auto& entry :
               fs::recursive_directory_iterator(path)
               )
          {
               // =================================================
               // Only Files
               // =================================================

               if (!entry.is_regular_file())
                    continue;


               const fs::path& filePath =
                    entry.path();


               // =================================================
               // Extension
               // =================================================

               if (
                    filePath.extension()
                    != ".AssetObject"
                    )
               {
                    continue;
               }


               // =================================================
               // Open File
               // =================================================

               std::ifstream file(
                    filePath,
                    std::ios::in
               );


               if (!file)
               {
                    std::cerr
                         << "[AssetManager] Failed to open asset: "
                         << filePath.string()
                         << std::endl;

                    continue;
               }


               // =================================================
               // Read Entire File
               // =================================================

               std::stringstream buffer;

               buffer <<
                    file.rdbuf();


               std::string content =
                    buffer.str();


               // =================================================
               // Load
               // =================================================

               int objectID =
                    LoadContent(
                         content
                    );


               if (objectID == -1)
               {
                    std::cerr
                         << "[AssetManager] Failed to load asset: "
                         << filePath.string()
                         << std::endl;

                    continue;
               }


               std::cout
                    << "[AssetManager] Loaded asset: "
                    << filePath.string()
                    << " ObjectID: "
                    << objectID
                    << std::endl;
          }
     }

     static AssetManager& Instance()
     {
          static AssetManager instance;
          return instance;
     }

     template<typename T>
     uint64_t Register(T asset)
     {
          auto ptr =
               std::make_shared<T>(
                    std::move(asset)
               );

          ptr->ReferenceInfo.ObjectID =
               m_NextObjectID++;
          ptr->ReferenceInfo.ScopeID =-1;
          ptr->ReferenceInfo.ScopeLevel = 1;

          uint64_t objectID =
               ptr->ReferenceInfo.ObjectID;

          m_Assets.push_back(
               ptr
          );

          return objectID;
     }

     template<typename T>
     bool Save(
          int objectID,
          const std::string& path)
     {
          auto asset = Find<T>(objectID);

          if (!asset)
               return false;

          asset->filePath = path;

          return JsonSerializer::SaveWithType(
               *asset,
               path
          );
     }

     template<typename T>
     bool Save(int objectID)
     {
          auto asset = Find<T>(objectID);

          if (!asset)
               return false;

          if (asset->filePath.empty())
          {
               std::cerr
                    << "[AssetManager] Asset has no file path. "
                    << "ObjectID: " << objectID
                    << std::endl;

               return false;
          }

          return Save<T>(
               objectID,
               asset->filePath
          );
     }

     template<typename T>
     T* Find(int objectID)
     {
          for (auto& asset : m_Assets)
          {
               if (!asset)
                    continue;

               if (asset->ReferenceInfo.ObjectID != objectID)
                    continue;


               T* result =
                    dynamic_cast<T*>(
                         asset.get()
                         );


               if (!result)
               {
                    std::cerr
                         << "[AssetManager] Type mismatch. "
                         << "ObjectID: "
                         << objectID
                         << std::endl;

                    return nullptr;
               }


               return result;
          }


          std::cerr
               << "[AssetManager] Asset not found. "
               << "ObjectID: "
               << objectID
               << std::endl;


          return nullptr;
     }

private:

     AssetManager() = default;
     ~AssetManager() = default;

     AssetManager(const AssetManager&) = delete;
     AssetManager& operator=(const AssetManager&) = delete;

     int m_NextObjectID = 0;

     std::vector<std::shared_ptr<Asset>> m_Assets;
};