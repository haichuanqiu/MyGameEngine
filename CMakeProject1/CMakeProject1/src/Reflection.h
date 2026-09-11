#pragma once

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>


// ============================================================
// Forward Declaration
// ============================================================

struct TypeInfo;


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

     Struct,
     Vector,

     Unknown
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
     using U = RemoveCVRef<T>;

     if constexpr (std::is_same_v<U, int>)
     {
          return FieldType::Int;
     }

     else if constexpr (std::is_same_v<U, float>)
     {
          return FieldType::Float;
     }

     else if constexpr (std::is_same_v<U, bool>)
     {
          return FieldType::Bool;
     }

     else if constexpr (IsVector<U>::value)
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

     case FieldType::Struct:
          return "struct";

     case FieldType::Vector:
          return "vector";

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
     //
     // int
     // float
     // Vector3
     // Transform
     // Renderer
     // ========================================================

     std::string typeName;


     // ========================================================
     // Value
     //
     // 只用于 int / float / bool 等可复制值。
     //
     // vector<unique_ptr<T>>
     // 不会放到这里。
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

     std::vector<PropertyNode> children;
};


// ============================================================
// FieldInfo
// ============================================================

struct FieldInfo
{
     const char* name = nullptr;

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
          void(void*, const std::any&)
     > set;


     // ========================================================
     // Build Inspector
     // ========================================================

     std::function<
          PropertyNode(void*)
     > build;
};


// ============================================================
// TypeInfo
//
// 注意：
//
// TypeInfo 必须在 ReflectionRegistry 之前完整定义。
// ============================================================
using TypeId = uint64_t;
struct TypeInfo
{
     const char* name = nullptr;

     std::vector<FieldInfo> fields;

     TypeId id = 0;
     // ========================================================
     // FindField
     // ========================================================

     FieldInfo* FindField(
          const std::string& fieldName)
     {
          for (auto& field : fields)
          {
               if (field.name == fieldName)
                    return &field;
          }

          return nullptr;
     }


     // ========================================================
     // Const FindField
     // ========================================================

     const FieldInfo* FindField(
          const std::string& fieldName) const
     {
          for (const auto& field : fields)
          {
               if (field.name == fieldName)
                    return &field;
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
               FindField(fieldName);

          if (!field)
               return T{};


          if (!field->get)
               return T{};


          try
          {
               return std::any_cast<T>(
                    field->get(object)
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
               FindField(fieldName);

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
constexpr TypeId HashString(
     const char* str)
{
     TypeId hash =
          14695981039346656037ull;

     while (*str)
     {
          hash ^=
               static_cast<unsigned char>(*str);

          hash *=
               1099511628211ull;

          ++str;
     }

     return hash;
}
constexpr TypeId GetTypeId(
     const char* name)
{
     return HashString(name);
}
// ============================================================
// Reflection Registry
//
// TypeInfo 已经完整定义，因此这里合法。
//
// std::type_index
//          ↓
// TypeInfo
//
// 不需要 Component。
// 不需要 Base。
// 不需要知道所有 class。
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
     void Register(TypeInfo info)
     {
          m_Types[
               std::type_index(typeid(T))
          ] = std::move(info);
     }


     // ========================================================
     // Find by type_info
     // ========================================================

     const TypeInfo* Find(
          const std::type_info& type) const
     {
          auto it =
               m_Types.find(
                    std::type_index(type)
               );

          if (it == m_Types.end())
               return nullptr;

          return &it->second;
     }


     // ========================================================
     // Find by C++ type
     // ========================================================

     template<typename T>
     const TypeInfo* Find() const
     {
          return Find(typeid(T));
     }
     const TypeInfo* FindById(TypeId id) const
     {
          for (const auto& [type, info] : m_Types)
          {
               if (info.id == id)
                    return &info;
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
//
// 这里只声明。
// REFLECT 后面会生成 specialization。
// ============================================================

template<typename T>
TypeInfo GetTypeInfo();


// ============================================================
// Runtime Type Detection
//
// 如果：
//
// Component* component
//
// 实际：
//
// Transform
//
// 那么：
//
// typeid(*component)
//
// ==
//
// typeid(Transform)
//
// ============================================================

template<typename T>
const TypeInfo* FindRuntimeType(
     T& value)
{
     using U =
          RemoveCVRef<T>;


     // ========================================================
     // Polymorphic
     // ========================================================

     if constexpr (std::is_polymorphic_v<U>)
     {
          return ReflectionRegistry::Instance()
               .Find(typeid(value));
     }


     // ========================================================
     // Non-polymorphic
     // ========================================================

     else
     {
          return ReflectionRegistry::Instance()
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


     // ========================================================
     // Type Name
     // ========================================================

     if constexpr (std::is_same_v<T, int>)
     {
          node.typeName = "int";
     }

     else if constexpr (std::is_same_v<T, float>)
     {
          node.typeName = "float";
     }

     else if constexpr (std::is_same_v<T, bool>)
     {
          node.typeName = "bool";
     }


     // ========================================================
     // Value
     // ========================================================

     node.value =
          value;


     // ========================================================
     // Set
     // ========================================================

     node.set =
          [&value](const std::any& newValue)
          {
               value =
                    std::any_cast<T>(
                         newValue
                    );
          };


     return node;
}


// ============================================================
// BuildPropertyNode
//
// Forward declarations of special overloads.
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


template<typename T, typename Allocator>
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

template<typename T, typename Allocator>
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


     // ========================================================
     // Build every element
     // ========================================================

     for (size_t i = 0;
          i < value.size();
          ++i)
     {
          std::string elementName =
               "[" + std::to_string(i) + "]";


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
// BuildPropertyNode
//
// 普通 Struct / Polymorphic Object
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
          std::is_same_v<U, bool>
          )
     {
          return BuildPrimitiveNode(
               name,
               value
          );
     }


     // ========================================================
     // Struct / Polymorphic
     // ========================================================

     else
     {
          PropertyNode node;


          node.name =
               name;


          node.type =
               FieldType::Struct;


          // ====================================================
          // Runtime Type
          // ====================================================

          const TypeInfo* type =
               FindRuntimeType(value);


          // ====================================================
          // No Reflection
          // ====================================================

          if (!type)
          {
               node.typeName =
                    typeid(value).name();

               return node;
          }


          // ====================================================
          // Type Name
          // ====================================================

          node.typeName =
               type->name
               ? type->name
               : "unknown";


          // ====================================================
          // Fields
          // ====================================================

          for (const auto& field :
               type->fields)
          {
               if (!field.build)
                    continue;


               node.children.push_back(
                    field.build(
                         static_cast<void*>(&value)
                    )
               );
          }


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
     //
     // unique_ptr 等不可复制类型不生成 get。
     // ========================================================

     if constexpr (
          std::is_copy_constructible_v<T>
          )
     {
          field.get =
               [member](const void* object)
               -> std::any
               {
                    const Class* obj =
                         static_cast<const Class*>(
                              object
                              );


                    return obj->*member;
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
          [member, name](void* object)
          {
               Class* obj =
                    static_cast<Class*>(
                         object
                         );


               T& value =
                    obj->*member;


               return BuildPropertyNode(
                    name,
                    value
               );
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
//
// 使用：
//
// REFLECT(
//     Transform,
//
//     FIELD(Transform, position),
//     FIELD(Transform, scale),
//     FIELD(Transform, rotation)
// )
//
// ============================================================

#define REFLECT(ClassName, ...)                             \
                                                            \
inline TypeInfo Get##ClassName##TypeInfo()                  \
{                                                           \
    TypeInfo info;                                          \
                                                            \
    info.id = HashString(#ClassName);                       \
    info.name = #ClassName;                                 \
                                                            \
    info.fields = { __VA_ARGS__ };                          \
                                                            \
    return info;                                            \
}                                                           \
                                                            \
template<>                                                  \
inline TypeInfo GetTypeInfo<ClassName>()                   \
{                                                           \
    return Get##ClassName##TypeInfo();                      \
}                                                           \
                                                            \
inline ReflectionAutoRegister<ClassName>                    \
    g_##ClassName##_ReflectionRegistration;


// ============================================================
// FIELD
// ============================================================

#define FIELD(ClassName, field)                             \
    MakeField<ClassName>(                                   \
        #field,                                             \
        &ClassName::field                                   \
    )

#define REFLECT_FRIEND(ClassName) \
    friend TypeInfo Get##ClassName##TypeInfo();