#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>


// ============================================================
// Forward Declaration
// ============================================================

struct TypeInfo;

class EngineObject;


// ============================================================
// Remove CV / Reference
// ============================================================

template<typename T>
using RemoveCVRef =
std::remove_cv_t<
     std::remove_reference_t<T>
>;


// ============================================================
// FieldType
// ============================================================

enum class FieldType
{
     Int,
     Float,
     Bool,
     String,

     Struct,
     Vector,

     Reference,

     Unknown
};


// ============================================================
// SerializedReference
// ============================================================

struct SerializedReference
{
     bool isNull = true;

     int ScopeLevel = -1;

     int ScopeID = -1;

     int ObjectID = -1;
};


// ============================================================
// Type Traits
// ============================================================


// ============================================================
// IsVector
// ============================================================

template<typename T>
struct IsVector
     : std::false_type
{};


template<typename T, typename Allocator>
struct IsVector<
     std::vector<T, Allocator>
>
     : std::true_type
{
     using ElementType = T;
};


// ============================================================
// IsUniquePtr
// ============================================================

template<typename T>
struct IsUniquePtr
     : std::false_type
{};


template<typename T, typename Deleter>
struct IsUniquePtr<
     std::unique_ptr<T, Deleter>
>
     : std::true_type
{
     using ElementType = T;
};


// ============================================================
// IsSharedPtr
// ============================================================

template<typename T>
struct IsSharedPtr
     : std::false_type
{};


template<typename T>
struct IsSharedPtr<
     std::shared_ptr<T>
>
     : std::true_type
{
     using ElementType = T;
};


// ============================================================
// GetFieldType
// ============================================================

template<typename T>
constexpr FieldType GetFieldType()
{
     using U =
          RemoveCVRef<T>;


     if constexpr (
          std::is_same_v<U, int>
          )
     {
          return FieldType::Int;
     }

     else if constexpr (
          std::is_same_v<U, float>
          )
     {
          return FieldType::Float;
     }

     else if constexpr (
          std::is_same_v<U, bool>
          )
     {
          return FieldType::Bool;
     }

     else if constexpr (
          std::is_same_v<U, std::string>
          )
     {
          return FieldType::String;
     }

     else if constexpr (
          IsVector<U>::value
          )
     {
          return FieldType::Vector;
     }

     else
     {
          return FieldType::Struct;
     }
}


// ============================================================
// GetFieldTypeName
// ============================================================

inline const char* GetFieldTypeName(
     FieldType type)
{
     switch (type)
     {
     case FieldType::Int:
          return "int";

     case FieldType::Float:
          return "float";

     case FieldType::Bool:
          return "bool";

     case FieldType::String:
          return "string";

     case FieldType::Struct:
          return "struct";

     case FieldType::Vector:
          return "vector";

     case FieldType::Reference:
          return "reference";

     default:
          return "unknown";
     }
}


// ============================================================
// PropertyNode
// ============================================================

struct PropertyNode
{
     // ========================================================
     // Name
     // ========================================================

     std::string name;


     // ========================================================
     // Field Type
     // ========================================================

     FieldType type =
          FieldType::Unknown;


     // ========================================================
     // Type Name
     // ========================================================

     std::string typeName;


     // ========================================================
     // Value
     // ========================================================

     std::any value;


     // ========================================================
     // Set
     // ========================================================

     std::function<
          void(const std::any&)
     > set;


     // ========================================================
     // Children
     // ========================================================

     std::vector<
          PropertyNode
     > children;


     // ========================================================
     // Reference Data
     // ========================================================

     SerializedReference reference;


     // ========================================================
     // Reference Runtime Type
     //
     // Material*:
     //
     // referenceType = typeid(Material)
     // ========================================================

     const std::type_info*
          referenceType =
          nullptr;


     // ========================================================
     // Can Set Reference
     //
     // Inspector 可以拿任意 EngineObject* 来测试：
     //
     // if (node.canSetReference(object))
     //
     // Material* field:
     //
     // Material      -> true
     // Asset         -> false
     // Renderer      -> false
     //
     // Asset* field:
     //
     // Material      -> true
     // Texture       -> true
     // Renderer      -> false
     // ========================================================

     std::function<
          bool(EngineObject*)
     > canSetReference;


     // ========================================================
     // Set Reference
     //
     // 真正赋值。
     //
     // 这里假设调用前已经经过：
     //
     // canSetReference()
     //
     // ========================================================

     std::function<
          void(void*)
     > setReference;
};


// ============================================================
// FieldInfo
// ============================================================

struct FieldInfo
{
     const char* name =
          nullptr;


     FieldType type =
          FieldType::Unknown;


     // ========================================================
     // Get
     // ========================================================

     std::function<
          std::any(const void*)
     > get;


     // ========================================================
     // Set
     // ========================================================

     std::function<
          void(
               void*,
               const std::any&
               )
     > set;


     // ========================================================
     // Build
     // ========================================================

     std::function<
          PropertyNode(void*)
     > build;
};


// ============================================================
// TypeInfo
// ============================================================

using TypeId =
uint64_t;


struct TypeInfo
{
     // ========================================================
     // Type Name
     // ========================================================

     const char* name =
          nullptr;


     // ========================================================
     // Fields
     // ========================================================

     std::vector<
          FieldInfo
     > fields;


     // ========================================================
     // Type ID
     // ========================================================

     TypeId id =
          0;


     // ========================================================
     // Base Type
     // ========================================================

     const std::type_info*
          baseType =
          nullptr;


     // ========================================================
     // Base Cast
     // ========================================================

     std::function<
          void* (void*)
     > baseCast;


     // ========================================================
     // FindField
     // ========================================================

     FieldInfo* FindField(
          const std::string& fieldName)
     {
          for (
               auto& field :
               fields
               )
          {
               if (
                    field.name ==
                    fieldName
                    )
               {
                    return &field;
               }
          }


          return nullptr;
     }


     // ========================================================
     // Const FindField
     // ========================================================

     const FieldInfo* FindField(
          const std::string& fieldName) const
     {
          for (
               const auto& field :
               fields
               )
          {
               if (
                    field.name ==
                    fieldName
                    )
               {
                    return &field;
               }
          }


          return nullptr;
     }


     // ========================================================
     // Get
     // ========================================================

     template<typename T>
     T Get(
          void* object,
          const std::string& fieldName)
     {
          FieldInfo* field =
               FindField(
                    fieldName
               );


          if (!field)
               return T{};


          if (!field->get)
               return T{};


          try
          {
               return std::any_cast<T>(
                    field->get(
                         object
                    )
               );
          }
          catch (...)
          {
               return T{};
          }
     }


     // ========================================================
     // Set
     // ========================================================

     template<typename T>
     void Set(
          void* object,
          const std::string& fieldName,
          const T& value)
     {
          FieldInfo* field =
               FindField(
                    fieldName
               );


          if (!field)
               return;


          if (!field->set)
               return;


          field->set(
               object,
               std::any(value)
          );
     }
};


// ============================================================
// Hash
// ============================================================

constexpr TypeId HashString(
     const char* str)
{
     TypeId hash =
          14695981039346656037ull;


     while (*str)
     {
          hash ^=
               static_cast<unsigned char>(
                    *str
                    );


          hash *=
               1099511628211ull;


          ++str;
     }


     return hash;
}


// ============================================================
// GetTypeId
// ============================================================

constexpr TypeId GetTypeId(
     const char* name)
{
     return HashString(
          name
     );
}


// ============================================================
// Reflection Registry
// ============================================================

class ReflectionRegistry
{
public:

     static ReflectionRegistry& Instance()
     {
          static ReflectionRegistry instance;

          return instance;
     }


     // ========================================================
     // Register
     // ========================================================

     template<typename T>
     void Register(
          TypeInfo info)
     {
          m_Types[
               std::type_index(
                    typeid(T)
               )
          ] =
               std::move(info);
     }


     // ========================================================
     // Find By type_info
     // ========================================================

     const TypeInfo* Find(
          const std::type_info& type) const
     {
          auto it =
               m_Types.find(
                    std::type_index(
                         type
                    )
               );


          if (
               it ==
               m_Types.end()
               )
          {
               return nullptr;
          }


          return &it->second;
     }


     // ========================================================
     // Find By C++ Type
     // ========================================================

     template<typename T>
     const TypeInfo* Find() const
     {
          return Find(
               typeid(T)
          );
     }


     // ========================================================
     // Find By TypeId
     // ========================================================

     const TypeInfo* FindById(
          TypeId id) const
     {
          for (
               const auto& [type, info] :
               m_Types
               )
          {
               if (
                    info.id ==
                    id
                    )
               {
                    return &info;
               }
          }


          return nullptr;
     }


private:

     std::unordered_map<
          std::type_index,
          TypeInfo
     > m_Types;
};


// ============================================================
// GetTypeInfo
// ============================================================

template<typename T>
TypeInfo GetTypeInfo();


// ============================================================
// Runtime Type Detection
// ============================================================

template<typename T>
const TypeInfo* FindRuntimeType(
     T& value)
{
     using U =
          RemoveCVRef<T>;


     if constexpr (
          std::is_polymorphic_v<U>
          )
     {
          return
               ReflectionRegistry::Instance()
               .Find(
                    typeid(value)
               );
     }
     else
     {
          return
               ReflectionRegistry::Instance()
               .Find<U>();
     }
}


// ============================================================
// BuildPrimitiveNode
// ============================================================

template<typename T>
PropertyNode BuildPrimitiveNode(
     const char* name,
     T& value)
{
     PropertyNode node;


     node.name =
          name;


     node.type =
          GetFieldType<T>();


     if constexpr (
          std::is_same_v<T, int>
          )
     {
          node.typeName =
               "int";
     }

     else if constexpr (
          std::is_same_v<T, float>
          )
     {
          node.typeName =
               "float";
     }

     else if constexpr (
          std::is_same_v<T, bool>
          )
     {
          node.typeName =
               "bool";
     }

     else if constexpr (
          std::is_same_v<T, std::string>
          )
     {
          node.typeName =
               "string";
     }


     node.value =
          value;


     node.set =
          [&value](
               const std::any& newValue)
          {
               value =
                    std::any_cast<T>(
                         newValue
                    );
          };


     return node;
}


// ============================================================
// BuildPropertyNode Forward Declarations
// ============================================================

template<typename T>
PropertyNode BuildPropertyNode(
     const char* name,
     T& value);


template<typename T>
PropertyNode BuildPropertyNode(
     const char* name,
     std::unique_ptr<T>& value);


template<typename T>
PropertyNode BuildPropertyNode(
     const char* name,
     std::shared_ptr<T>& value);


template<
     typename T,
     typename Allocator
>
PropertyNode BuildPropertyNode(
     const char* name,
     std::vector<T, Allocator>& value);


// ============================================================
// unique_ptr
// ============================================================

template<typename T>
PropertyNode BuildPropertyNode(
     const char* name,
     std::unique_ptr<T>& value)
{
     if (!value)
     {
          PropertyNode node;

          node.name =
               name;

          node.type =
               FieldType::Struct;

          node.typeName =
               "null";

          return node;
     }


     return BuildPropertyNode(
          name,
          *value
     );
}


// ============================================================
// shared_ptr
// ============================================================

template<typename T>
PropertyNode BuildPropertyNode(
     const char* name,
     std::shared_ptr<T>& value)
{
     if (!value)
     {
          PropertyNode node;

          node.name =
               name;

          node.type =
               FieldType::Struct;

          node.typeName =
               "null";

          return node;
     }


     return BuildPropertyNode(
          name,
          *value
     );
}


// ============================================================
// vector
// ============================================================

template<
     typename T,
     typename Allocator
>
PropertyNode BuildPropertyNode(
     const char* name,
     std::vector<T, Allocator>& value)
{
     PropertyNode node;


     node.name =
          name;


     node.type =
          FieldType::Vector;


     node.typeName =
          "vector";


     for (
          size_t i = 0;
          i < value.size();
          ++i
          )
     {
          std::string elementName =
               "[" +
               std::to_string(i) +
               "]";


          node.children.push_back(
               BuildPropertyNode(
                    elementName.c_str(),
                    value[i]
               )
          );
     }


     return node;
}


// ============================================================
// BuildTypeFields
// ============================================================

inline void BuildTypeFields(
     PropertyNode& node,
     const TypeInfo* type,
     void* object)
{
     if (!type)
          return;


     // ========================================================
     // Base
     // ========================================================

     if (
          type->baseType &&
          type->baseCast
          )
     {
          const TypeInfo* baseInfo =
               ReflectionRegistry::Instance()
               .Find(
                    *type->baseType
               );


          if (baseInfo)
          {
               void* baseObject =
                    type->baseCast(
                         object
                    );


               BuildTypeFields(
                    node,
                    baseInfo,
                    baseObject
               );
          }
     }


     // ========================================================
     // Current
     // ========================================================

     for (
          const auto& field :
          type->fields
          )
     {
          if (!field.build)
               continue;


          node.children.push_back(
               field.build(
                    object
               )
          );
     }
}


// ============================================================
// BuildPropertyNode
// ============================================================

template<typename T>
PropertyNode BuildPropertyNode(
     const char* name,
     T& value)
{
     using U =
          RemoveCVRef<T>;


     // ========================================================
     // Primitive
     // ========================================================

     if constexpr (
          std::is_same_v<U, int> ||
          std::is_same_v<U, float> ||
          std::is_same_v<U, bool> ||
          std::is_same_v<U, std::string>
          )
     {
          return BuildPrimitiveNode(
               name,
               value
          );
     }


     // ========================================================
     // Struct
     // ========================================================

     else
     {
          PropertyNode node;


          node.name =
               name;


          node.type =
               FieldType::Struct;


          const TypeInfo* type =
               FindRuntimeType(
                    value
               );


          if (!type)
          {
               node.typeName =
                    typeid(value).name();

               return node;
          }


          node.typeName =
               type->name
               ? type->name
               : "unknown";


          BuildTypeFields(
               node,
               type,
               static_cast<void*>(
                    &value
                    )
          );


          return node;
     }
}


// ============================================================
// MakeField
// ============================================================

template<typename Class, typename T>
FieldInfo MakeField(
     const char* name,
     T Class::* member)
{
     FieldInfo field;


     field.name =
          name;


     field.type =
          GetFieldType<T>();


     // ========================================================
     // Get
     // ========================================================

     if constexpr (
          std::is_copy_constructible_v<T>
          )
     {
          field.get =
               [member](
                    const void* object)
               -> std::any
               {
                    const Class* obj =
                         static_cast<
                         const Class*
                         >(
                              object
                              );


                    return
                         obj->*member;
               };
     }


     // ========================================================
     // Set
     // ========================================================

     if constexpr (
          std::is_copy_assignable_v<T>
          )
     {
          field.set =
               [member](
                    void* object,
                    const std::any& value)
               {
                    Class* obj =
                         static_cast<Class*>(
                              object
                              );


                    obj->*member =
                         std::any_cast<T>(
                              value
                         );
               };
     }


     // ========================================================
     // Build
     // ========================================================

     field.build =
          [member, name](
               void* object)
          {
               Class* obj =
                    static_cast<Class*>(
                         object
                         );


               T& value =
                    obj->*member;


               return
                    BuildPropertyNode(
                         name,
                         value
                    );
          };


     return field;
}


// ============================================================
// MakeReferenceField
//
// 核心：
//
// T* field
//
// 自动生成：
//
// reference
// referenceType
// canSetReference
// setReference
// ============================================================

template<typename Class, typename T>
FieldInfo MakeReferenceField(
     const char* name,
     T* Class::* member)
{
     FieldInfo field;


     field.name =
          name;


     field.type =
          FieldType::Reference;


     field.build =
          [member, name](
               void* object)
          {
               Class* obj =
                    static_cast<Class*>(
                         object
                         );


               T* target =
                    obj->*member;


               PropertyNode node;


               node.name =
                    name;


               node.type =
                    FieldType::Reference;


               node.typeName =
                    typeid(T).name();


               node.referenceType =
                    &typeid(T);


               // =================================================
               // Current Reference
               // =================================================

               if (!target)
               {
                    node.reference.isNull =
                         true;
               }
               else
               {
                    node.reference.isNull =
                         false;


                    node.reference.ScopeLevel =
                         target
                         ->ReferenceInfo
                         .ScopeLevel;


                    node.reference.ScopeID =
                         target
                         ->ReferenceInfo
                         .ScopeID;


                    node.reference.ObjectID =
                         target
                         ->ReferenceInfo
                         .ObjectID;
               }


               // =================================================
               // Can Set Reference
               //
               // 不修改任何东西。
               //
               // 只测试：
               //
               // EngineObject*
               //      ↓
               // dynamic_cast<T*>
               //
               // 成功：
               // true
               //
               // 失败：
               // false
               // =================================================

               node.canSetReference =
                    [](
                         EngineObject* candidate)
                    -> bool
                    {
                         // null 永远允许
                         if (!candidate)
                              return true;


                         T* casted =
                              dynamic_cast<T*>(
                                   candidate
                                   );


                         return
                              casted != nullptr;
                    };


               // =================================================
               // Set Reference
               //
               // 理论上应该先通过：
               //
               // canSetReference()
               //
               // 再调用这个。
               // =================================================

               node.setReference =
                    [obj, member](
                         void* resolved)
                    {
                         obj->*member =
                              static_cast<T*>(
                                   resolved
                                   );
                    };


               return node;
          };


     return field;
}


// ============================================================
// Automatic Registration
// ============================================================

template<typename T>
struct ReflectionAutoRegister
{
     ReflectionAutoRegister()
     {
          ReflectionRegistry::Instance()
               .Register<T>(
                    GetTypeInfo<T>()
               );
     }
};


// ============================================================
// REFLECT
// ============================================================

#define REFLECT(ClassName, ...)                             \
                                                            \
inline TypeInfo Get##ClassName##TypeInfo()                  \
{                                                           \
     TypeInfo info;                                         \
                                                            \
     info.id =                                              \
          HashString(#ClassName);                           \
                                                            \
     info.name =                                            \
          #ClassName;                                       \
                                                            \
     info.fields =                                          \
     {                                                      \
          __VA_ARGS__                                       \
     };                                                     \
                                                            \
     return info;                                           \
}                                                           \
                                                            \
template<>                                                  \
inline TypeInfo GetTypeInfo<ClassName>()                    \
{                                                           \
     return Get##ClassName##TypeInfo();                     \
}                                                           \
                                                            \
inline ReflectionAutoRegister<ClassName>                    \
     g_##ClassName##_ReflectionRegistration;


// ============================================================
// REFLECT_BASE
// ============================================================

#define REFLECT_BASE(ClassName, BaseClass, ...)             \
                                                            \
inline TypeInfo Get##ClassName##TypeInfo()                  \
{                                                           \
     TypeInfo info;                                         \
                                                            \
     info.id =                                              \
          HashString(#ClassName);                           \
                                                            \
     info.name =                                            \
          #ClassName;                                       \
                                                            \
     info.baseType =                                        \
          &typeid(BaseClass);                               \
                                                            \
     info.baseCast =                                        \
          [](void* object) -> void*                         \
          {                                                 \
               return                                       \
                    static_cast<BaseClass*>(                \
                         static_cast<ClassName*>(            \
                              object                        \
                         )                                  \
                    );                                      \
          };                                                \
                                                            \
     info.fields =                                          \
     {                                                      \
          __VA_ARGS__                                       \
     };                                                     \
                                                            \
     return info;                                           \
}                                                           \
                                                            \
template<>                                                  \
inline TypeInfo GetTypeInfo<ClassName>()                    \
{                                                           \
     return Get##ClassName##TypeInfo();                     \
}                                                           \
                                                            \
inline ReflectionAutoRegister<ClassName>                    \
     g_##ClassName##_ReflectionRegistration;


// ============================================================
// FIELD
// ============================================================

#define FIELD(ClassName, field)                             \
     MakeField<ClassName>(                                  \
          #field,                                           \
          &ClassName::field                                 \
     )


// ============================================================
// REF_FIELD
// ============================================================

#define REF_FIELD(ClassName, field)                         \
     MakeReferenceField<ClassName>(                         \
          #field,                                           \
          &ClassName::field                                 \
     )


// ============================================================
// REFLECT_FRIEND
// ============================================================

#define REFLECT_FRIEND(ClassName)                           \
     friend TypeInfo Get##ClassName##TypeInfo();