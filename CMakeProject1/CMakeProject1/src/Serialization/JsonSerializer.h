#pragma once

#include "Reflection.h"
#include "Assets/ReferenceResolver.h"
#include <any>
#include <fstream>
#include <sstream>
#include <string>

#pragma once
#include <any>
#include <cctype>
#include <cstring>
#include <utility>
#include <vector>


class JsonSerializer
{
private:

     // ============================================================
     // Simple JSON Value
     // ============================================================

     struct JsonValue
     {
          enum class Type
          {
               Null,
               Number,
               Bool,
               String,
               Object,
               Array
          };


          Type type =
               Type::Null;


          double number =
               0.0;


          bool boolean =
               false;


          std::string string;


          std::vector<
               std::pair<
               std::string,
               JsonValue
               >
          > object;


          std::vector<
               JsonValue
          > array;
     };


     // ============================================================
     // Parser
     // ============================================================

     class Parser
     {
     public:

          explicit Parser(
               const std::string& text)
               : m_Text(text)
          {}


          // ========================================================
          // Parse
          // ========================================================

          bool Parse(
               JsonValue& out)
          {
               SkipWhitespace();


               if (!ParseValue(out))
                    return false;


               SkipWhitespace();


               return
                    m_Position ==
                    m_Text.size();
          }


     private:

          // ========================================================
          // Parse Value
          // ========================================================

          bool ParseValue(
               JsonValue& out)
          {
               SkipWhitespace();


               if (
                    m_Position >=
                    m_Text.size()
                    )
               {
                    return false;
               }


               char c =
                    m_Text[
                         m_Position
                    ];


               // =================================================
               // Object
               // =================================================

               if (c == '{')
               {
                    return
                         ParseObject(
                              out
                         );
               }


               // =================================================
               // Array
               // =================================================

               if (c == '[')
               {
                    return
                         ParseArray(
                              out
                         );
               }


               // =================================================
               // String
               // =================================================

               if (c == '"')
               {
                    return
                         ParseStringValue(
                              out
                         );
               }


               // =================================================
               // Bool
               // =================================================

               if (
                    c == 't' ||
                    c == 'f'
                    )
               {
                    return
                         ParseBool(
                              out
                         );
               }


               // =================================================
               // Null
               // =================================================

               if (c == 'n')
               {
                    return
                         ParseNull(
                              out
                         );
               }


               // =================================================
               // Number
               // =================================================

               if (
                    c == '-' ||
                    c == '+' ||
                    std::isdigit(
                         static_cast<
                         unsigned char
                         >(c)
                    )
                    )
               {
                    return
                         ParseNumber(
                              out
                         );
               }


               return false;
          }


          // ========================================================
          // Parse Object
          // ========================================================

          bool ParseObject(
               JsonValue& out)
          {
               if (!Consume('{'))
                    return false;


               out =
                    JsonValue{};


               out.type =
                    JsonValue::Type::Object;


               SkipWhitespace();


               // Empty Object
               if (Consume('}'))
                    return true;


               while (true)
               {
                    SkipWhitespace();


                    std::string key;


                    if (!ParseString(key))
                         return false;


                    SkipWhitespace();


                    if (!Consume(':'))
                         return false;


                    SkipWhitespace();


                    JsonValue value;


                    if (!ParseValue(value))
                         return false;


                    out.object.emplace_back(
                         std::move(key),
                         std::move(value)
                    );


                    SkipWhitespace();


                    if (Consume('}'))
                         return true;


                    if (!Consume(','))
                         return false;
               }
          }


          // ========================================================
          // Parse Array
          // ========================================================

          bool ParseArray(
               JsonValue& out)
          {
               if (!Consume('['))
                    return false;


               out =
                    JsonValue{};


               out.type =
                    JsonValue::Type::Array;


               SkipWhitespace();


               // Empty Array
               if (Consume(']'))
                    return true;


               while (true)
               {
                    SkipWhitespace();


                    JsonValue value;


                    if (!ParseValue(value))
                         return false;


                    out.array.push_back(
                         std::move(value)
                    );


                    SkipWhitespace();


                    if (Consume(']'))
                         return true;


                    if (!Consume(','))
                         return false;
               }
          }


          // ========================================================
          // Parse Bool
          // ========================================================

          bool ParseBool(
               JsonValue& out)
          {
               if (Match("true"))
               {
                    out =
                         JsonValue{};


                    out.type =
                         JsonValue::Type::Bool;


                    out.boolean =
                         true;


                    return true;
               }


               if (Match("false"))
               {
                    out =
                         JsonValue{};


                    out.type =
                         JsonValue::Type::Bool;


                    out.boolean =
                         false;


                    return true;
               }


               return false;
          }


          // ========================================================
          // Parse Null
          // ========================================================

          bool ParseNull(
               JsonValue& out)
          {
               if (!Match("null"))
                    return false;


               out =
                    JsonValue{};


               out.type =
                    JsonValue::Type::Null;


               return true;
          }


          // ========================================================
          // Parse Number
          // ========================================================

          bool ParseNumber(
               JsonValue& out)
          {
               size_t begin =
                    m_Position;


               if (
                    m_Position <
                    m_Text.size() &&
                    (
                         m_Text[m_Position] == '-' ||
                         m_Text[m_Position] == '+'
                         )
                    )
               {
                    ++m_Position;
               }


               bool hasDigits =
                    false;


               while (
                    m_Position <
                    m_Text.size() &&
                    std::isdigit(
                         static_cast<
                         unsigned char
                         >(
                              m_Text[
                                   m_Position
                              ]
                              )
                    )
                    )
               {
                    hasDigits =
                         true;


                    ++m_Position;
               }


               // =================================================
               // Decimal
               // =================================================

               if (
                    m_Position <
                    m_Text.size() &&
                    m_Text[m_Position] == '.'
                    )
               {
                    ++m_Position;


                    while (
                         m_Position <
                         m_Text.size() &&
                         std::isdigit(
                              static_cast<
                              unsigned char
                              >(
                                   m_Text[
                                        m_Position
                                   ]
                                   )
                         )
                         )
                    {
                         hasDigits =
                              true;


                         ++m_Position;
                    }
               }


               // =================================================
               // Scientific
               // =================================================

               if (
                    m_Position <
                    m_Text.size() &&
                    (
                         m_Text[m_Position] == 'e' ||
                         m_Text[m_Position] == 'E'
                         )
                    )
               {
                    ++m_Position;


                    if (
                         m_Position <
                         m_Text.size() &&
                         (
                              m_Text[m_Position] == '+' ||
                              m_Text[m_Position] == '-'
                              )
                         )
                    {
                         ++m_Position;
                    }


                    while (
                         m_Position <
                         m_Text.size() &&
                         std::isdigit(
                              static_cast<
                              unsigned char
                              >(
                                   m_Text[
                                        m_Position
                                   ]
                                   )
                         )
                         )
                    {
                         hasDigits =
                              true;


                         ++m_Position;
                    }
               }


               if (!hasDigits)
                    return false;


               std::string numberText =
                    m_Text.substr(
                         begin,
                         m_Position - begin
                    );


               try
               {
                    out =
                         JsonValue{};


                    out.type =
                         JsonValue::Type::Number;


                    out.number =
                         std::stod(
                              numberText
                         );
               }
               catch (...)
               {
                    return false;
               }


               return true;
          }


          // ========================================================
          // Parse String
          // ========================================================

          bool ParseString(
               std::string& out)
          {
               if (!Consume('"'))
                    return false;


               out.clear();


               while (
                    m_Position <
                    m_Text.size()
                    )
               {
                    char c =
                         m_Text[
                              m_Position++
                         ];


                    if (c == '"')
                         return true;


                    if (c != '\\')
                    {
                         out += c;

                         continue;
                    }


                    if (
                         m_Position >=
                         m_Text.size()
                         )
                    {
                         return false;
                    }


                    char escaped =
                         m_Text[
                              m_Position++
                         ];


                    switch (escaped)
                    {
                    case '"':
                         out += '"';
                         break;

                    case '\\':
                         out += '\\';
                         break;

                    case '/':
                         out += '/';
                         break;

                    case 'n':
                         out += '\n';
                         break;

                    case 'r':
                         out += '\r';
                         break;

                    case 't':
                         out += '\t';
                         break;

                    case 'b':
                         out += '\b';
                         break;

                    case 'f':
                         out += '\f';
                         break;

                    default:
                         return false;
                    }
               }


               return false;
          }


          // ========================================================
          // Parse String Value
          // ========================================================

          bool ParseStringValue(
               JsonValue& out)
          {
               std::string stringValue;


               if (!ParseString(stringValue))
                    return false;


               out =
                    JsonValue{};


               out.type =
                    JsonValue::Type::String;


               out.string =
                    std::move(
                         stringValue
                    );


               return true;
          }


          // ========================================================
          // Skip Whitespace
          // ========================================================

          void SkipWhitespace()
          {
               while (
                    m_Position <
                    m_Text.size() &&
                    std::isspace(
                         static_cast<
                         unsigned char
                         >(
                              m_Text[
                                   m_Position
                              ]
                              )
                    )
                    )
               {
                    ++m_Position;
               }
          }


          // ========================================================
          // Consume
          // ========================================================

          bool Consume(
               char expected)
          {
               SkipWhitespace();


               if (
                    m_Position >=
                    m_Text.size()
                    )
               {
                    return false;
               }


               if (
                    m_Text[m_Position] !=
                    expected
                    )
               {
                    return false;
               }


               ++m_Position;


               return true;
          }


          // ========================================================
          // Match
          // ========================================================

          bool Match(
               const char* text)
          {
               size_t length =
                    std::strlen(
                         text
                    );


               if (
                    m_Position + length >
                    m_Text.size()
                    )
               {
                    return false;
               }


               if (
                    m_Text.compare(
                         m_Position,
                         length,
                         text
                    ) != 0
                    )
               {
                    return false;
               }


               m_Position +=
                    length;


               return true;
          }


     private:

          const std::string&
               m_Text;


          size_t m_Position =
               0;
     };


public:

     // ============================================================
     // Serialize
     // ============================================================

     template<typename T>
     static std::string Serialize(
          T& value)
     {
          const TypeInfo* info =
               FindRuntimeType(
                    value
               );


          if (!info)
          {
               return
                    "{No applicable serialization found.}";
          }


          PropertyNode node =
               BuildPropertyNode(
                    "target",
                    value
               );


          return
               SerializePropertyNode(
                    node
               );
     }

     template<typename T>
     static bool LoadReference(
          T& value,
          const std::string& data)
     {
          JsonValue root;


          if (!ParseRootObject(data, root))
               return false;


          PropertyNode node =
               BuildPropertyNode(
                    "target",
                    value
               );


          return ApplyReferenceNode(
               node,
               root
          );
     }

     // ============================================================
     // Deserialize
     //
     // 普通 Field：
     //
     // Int
     // Float
     // Bool
     // String
     // Struct
     // Vector
     //
     // 正常恢复。
     //
     // Reference：
     //
     // 忽略。
     //
     // 由外部 Reference Loader / Resolver 处理。
     // ============================================================

     template<typename T>
     static bool Deserialize(
          T& value,
          const std::string& data)
     {
          const TypeInfo* info =
               FindRuntimeType(
                    value
               );


          if (!info)
               return false;


          JsonValue root;


          if (!ParseRootObject(data, root))
               return false;


          PropertyNode node =
               BuildPropertyNode(
                    "target",
                    value
               );


          return ApplyObject(
               node,
               root
          );
     }


     // ============================================================
     // Serialize PropertyNode
     // ============================================================

     static std::string SerializePropertyNode(
          const PropertyNode& node)
     {
          std::ostringstream out;


          WriteNode(
               out,
               node,
               0
          );


          return out.str();
     }


     // ============================================================
     // Save
     // ============================================================

     template<typename T>
     static bool Save(
          T& value,
          const std::string& filePath)
     {
          std::ofstream file(
               filePath
          );


          if (!file)
               return false;


          file <<
               Serialize(
                    value
               );


          return file.good();
     }


     // ============================================================
     // Save With Type
     // ============================================================

     template<typename T>
     static bool SaveWithType(
          T& value,
          const std::string& filePath)
     {
          std::ofstream file(
               filePath
          );


          if (!file)
               return false;


          // =====================================================
          // Reflection
          // =====================================================

          const TypeInfo* info =
               ReflectionRegistry::Instance()
               .Find(
                    typeid(value)
               );


          if (!info)
               return false;


          // =====================================================
          // TypeId
          // =====================================================

          TypeId typeId =
               info->id;


          file <<
               "TypeId:";


          file <<
               typeId;


          file <<
               "\n";


          // =====================================================
          // Data
          // =====================================================

          file <<
               "Data: ";


          file <<
               Serialize(
                    value
               );


          file <<
               "\n";


          return file.good();
     }


private:

     // ============================================================
     // Parse Root Object
     // ============================================================

     static bool ParseRootObject(
          const std::string& data,
          JsonValue& root)
     {
          Parser parser(data);


          if (!parser.Parse(root))
               return false;


          return
               root.type ==
               JsonValue::Type::Object;
     }


     // ============================================================
     // Read Int Member
     // ============================================================

     static bool ReadIntMember(
          const JsonValue& object,
          const std::string& name,
          int& out)
     {
          const JsonValue* value =
               FindObjectMember(
                    object,
                    name
               );


          if (!value)
               return false;


          if (
               value->type !=
               JsonValue::Type::Number
               )
          {
               return false;
          }


          out =
               static_cast<int>(
                    value->number
               );


          return true;
     }


     // ============================================================
     // Apply Reference Node
     // ============================================================

     static bool ApplyReferenceNode(
          PropertyNode& property,
          const JsonValue& json)
     {
          if (
               property.type ==
               FieldType::Reference
               )
          {
               if (
                    json.type ==
                    JsonValue::Type::Null
                    )
               {
                    if (property.setReference)
                    {
                         property.setReference(
                              nullptr
                         );
                    }


                    return true;
               }


               if (
                    json.type !=
                    JsonValue::Type::Object
                    )
               {
                    return false;
               }


               ReferenceDescription reference;


               if (!ReadIntMember(
                    json,
                    "ScopeLevel",
                    reference.ScopeLevel
                    ))
               {
                    return false;
               }


               if (!ReadIntMember(
                    json,
                    "ScopeID",
                    reference.ScopeID
                    ))
               {
                    return false;
               }


               if (!ReadIntMember(
                    json,
                    "ObjectID",
                    reference.ObjectID
                    ))
               {
                    return false;
               }


               EngineObject* resolved =
                    ReferenceResolver::Instance()
                    .GetItem(
                         reference
                    );


               if (property.setReference)
               {
                    property.setReference(
                         resolved
                    );
               }


               return true;
          }


          if (
               property.type ==
               FieldType::Struct
               )
          {
               if (
                    json.type !=
                    JsonValue::Type::Object
                    )
               {
                    return true;
               }


               bool success =
                    true;


               for (
                    auto& child :
                    property.children
                    )
               {
                    const JsonValue* jsonChild =
                         FindObjectMember(
                              json,
                              child.name
                         );


                    if (!jsonChild)
                         continue;


                    if (!ApplyReferenceNode(
                         child,
                         *jsonChild
                         ))
                    {
                         success =
                              false;
                    }
               }


               return success;
          }


          if (
               property.type ==
               FieldType::Vector
               )
          {
               if (
                    json.type !=
                    JsonValue::Type::Array
                    )
               {
                    return true;
               }


               bool success =
                    true;


               const size_t count =
                    std::min(
                         property.children.size(),
                         json.array.size()
                    );


               for (
                    size_t i = 0;
                    i < count;
                    ++i
                    )
               {
                    if (!ApplyReferenceNode(
                         property.children[i],
                         json.array[i]
                         ))
                    {
                         success =
                              false;
                    }
               }


               return success;
          }


          return true;
     }


     // ============================================================
     // Set Node Value
     // ============================================================

     template<typename T>
     static bool SetNodeValue(
          PropertyNode& node,
          T&& value)
     {
          if (!node.set)
               return false;


          try
          {
               node.set(
                    std::any(
                         std::forward<T>(value)
                    )
               );


               return true;
          }
          catch (...)
          {
               return false;
          }
     }


     // ============================================================
     // Apply Object
     // ============================================================

     static bool ApplyObject(
          PropertyNode& node,
          const JsonValue& value)
     {
          if (
               value.type !=
               JsonValue::Type::Object
               )
          {
               return false;
          }


          bool success =
               true;


          for (
               auto& child :
               node.children
               )
          {
               const JsonValue* jsonChild =
                    FindObjectMember(
                         value,
                         child.name
                    );


               if (!jsonChild)
                    continue;


               if (
                    !ApplyNode(
                         child,
                         *jsonChild
                    )
                    )
               {
                    success =
                         false;
               }
          }


          return success;
     }


     // ============================================================
     // Apply Node
     // ============================================================

     static bool ApplyNode(
          PropertyNode& node,
          const JsonValue& value)
     {
          switch (
               node.type
               )
          {
               // =====================================================
               // Int
               // =====================================================

          case FieldType::Int:
          {
               if (
                    value.type !=
                    JsonValue::Type::Number
                    )
               {
                    return false;
               }


               return SetNodeValue(
                    node,
                    static_cast<int>(
                         value.number
                    )
               );
          }


          // =====================================================
          // Float
          // =====================================================

          case FieldType::Float:
          {
               if (
                    value.type !=
                    JsonValue::Type::Number
                    )
               {
                    return false;
               }


               return SetNodeValue(
                    node,
                    static_cast<float>(
                         value.number
                    )
               );
          }


          // =====================================================
          // Bool
          // =====================================================

          case FieldType::Bool:
          {
               if (
                    value.type !=
                    JsonValue::Type::Bool
                    )
               {
                    return false;
               }


               return SetNodeValue(
                    node,
                    value.boolean
               );
          }


          // =====================================================
          // String
          // =====================================================

          case FieldType::String:
          {
               if (
                    value.type !=
                    JsonValue::Type::String
                    )
               {
                    return false;
               }


               return SetNodeValue(
                    node,
                    value.string
               );
          }


          // =====================================================
          // Struct
          // =====================================================

          case FieldType::Struct:
          {
               return
                    ApplyObject(
                         node,
                         value
                    );
          }


          // =====================================================
          // Vector
          // =====================================================

          case FieldType::Vector:
          {
               return
                    ApplyArray(
                         node,
                         value
                    );
          }


          // =====================================================
          // Reference
          //
          // JsonSerializer 不负责 Reference Load。
          //
          // 这里直接认为成功，然后跳过。
          //
          // Runtime pointer 保持原值。
          // =====================================================

          case FieldType::Reference:
          {
               return true;
          }


          default:
               return false;
          }
     }


     // ============================================================
     // Apply Array
     // ============================================================

     static bool ApplyArray(
          PropertyNode& node,
          const JsonValue& value)
     {
          if (
               value.type !=
               JsonValue::Type::Array
               )
          {
               return false;
          }


          bool success =
               true;


          const size_t count =
               std::min(
                    node.children.size(),
                    value.array.size()
               );


          for (
               size_t i = 0;
               i < count;
               ++i
               )
          {
               if (
                    !ApplyNode(
                         node.children[i],
                         value.array[i]
                    )
                    )
               {
                    success =
                         false;
               }
          }


          return success;
     }


     // ============================================================
     // Find Object Member
     // ============================================================

     static const JsonValue*
          FindObjectMember(
               const JsonValue& object,
               const std::string& name)
     {
          if (
               object.type !=
               JsonValue::Type::Object
               )
          {
               return nullptr;
          }


          for (
               const auto& [key, value] :
               object.object
               )
          {
               if (
                    key ==
                    name
                    )
               {
                    return &value;
               }
          }


          return nullptr;
     }


     // ============================================================
     // Write Node
     // ============================================================

     static void WriteNode(
          std::ostringstream& out,
          const PropertyNode& node,
          int indent)
     {
          switch (
               node.type
               )
          {
               // =====================================================
               // Int
               // =====================================================

          case FieldType::Int:
          {
               out <<
                    std::any_cast<int>(
                         node.value
                    );


               break;
          }


          // =====================================================
          // Float
          // =====================================================

          case FieldType::Float:
          {
               out <<
                    std::any_cast<float>(
                         node.value
                    );


               break;
          }


          // =====================================================
          // Bool
          // =====================================================

          case FieldType::Bool:
          {
               out <<
                    (
                         std::any_cast<bool>(
                              node.value
                         )
                         ? "true"
                         : "false"
                         );


               break;
          }


          // =====================================================
          // String
          // =====================================================

          case FieldType::String:
          {
               out <<
                    "\"";


               out <<
                    Escape(
                         std::any_cast<
                         std::string
                         >(
                              node.value
                         )
                    );


               out <<
                    "\"";


               break;
          }


          // =====================================================
          // Struct
          // =====================================================

          case FieldType::Struct:
          {
               WriteObject(
                    out,
                    node,
                    indent
               );


               break;
          }


          // =====================================================
          // Vector
          // =====================================================

          case FieldType::Vector:
          {
               WriteArray(
                    out,
                    node,
                    indent
               );


               break;
          }


          // =====================================================
          // Reference
          // =====================================================

          case FieldType::Reference:
          {
               WriteReference(
                    out,
                    node,
                    indent
               );


               break;
          }


          default:
          {
               out <<
                    "null";


               break;
          }
          }
     }


     // ============================================================
     // Write Object
     // ============================================================

     static void WriteObject(
          std::ostringstream& out,
          const PropertyNode& node,
          int indent)
     {
          out <<
               "{";


          if (
               !node.children.empty()
               )
          {
               out <<
                    "\n";


               for (
                    size_t i = 0;
                    i < node.children.size();
                    ++i
                    )
               {
                    const auto& child =
                         node.children[i];


                    WriteIndent(
                         out,
                         indent + 1
                    );


                    out <<
                         "\"";


                    out <<
                         Escape(
                              child.name
                         );


                    out <<
                         "\": ";


                    WriteNode(
                         out,
                         child,
                         indent + 1
                    );


                    if (
                         i + 1 <
                         node.children.size()
                         )
                    {
                         out <<
                              ",";
                    }


                    out <<
                         "\n";
               }


               WriteIndent(
                    out,
                    indent
               );
          }


          out <<
               "}";
     }


     // ============================================================
     // Write Array
     // ============================================================

     static void WriteArray(
          std::ostringstream& out,
          const PropertyNode& node,
          int indent)
     {
          out <<
               "[";


          if (
               !node.children.empty()
               )
          {
               out <<
                    "\n";


               for (
                    size_t i = 0;
                    i < node.children.size();
                    ++i
                    )
               {
                    const auto& child =
                         node.children[i];


                    WriteIndent(
                         out,
                         indent + 1
                    );


                    WriteNode(
                         out,
                         child,
                         indent + 1
                    );


                    if (
                         i + 1 <
                         node.children.size()
                         )
                    {
                         out <<
                              ",";
                    }


                    out <<
                         "\n";
               }


               WriteIndent(
                    out,
                    indent
               );
          }


          out <<
               "]";
     }


     // ============================================================
     // Write Reference
     // ============================================================

     static void WriteReference(
          std::ostringstream& out,
          const PropertyNode& node,
          int indent)
     {
          // =====================================================
          // Null
          // =====================================================

          if (
               node.reference.isNull
               )
          {
               out <<
                    "null";


               return;
          }


          // =====================================================
          // Begin
          // =====================================================

          out <<
               "{\n";


          // =====================================================
          // ScopeLevel
          // =====================================================

          WriteIndent(
               out,
               indent + 1
          );


          out <<
               "\"ScopeLevel\": ";


          out <<
               node.reference.ScopeLevel;


          out <<
               ",\n";


          // =====================================================
          // ScopeID
          // =====================================================

          WriteIndent(
               out,
               indent + 1
          );


          out <<
               "\"ScopeID\": ";


          out <<
               node.reference.ScopeID;


          out <<
               ",\n";


          // =====================================================
          // ObjectID
          // =====================================================

          WriteIndent(
               out,
               indent + 1
          );


          out <<
               "\"ObjectID\": ";


          out <<
               node.reference.ObjectID;


          out <<
               "\n";


          // =====================================================
          // End
          // =====================================================

          WriteIndent(
               out,
               indent
          );


          out <<
               "}";
     }


     // ============================================================
     // Write Indent
     // ============================================================

     static void WriteIndent(
          std::ostringstream& out,
          int indent)
     {
          out <<
               std::string(
                    indent * 4,
                    ' '
               );
     }


     // ============================================================
     // Escape
     // ============================================================

     static std::string Escape(
          const std::string& str)
     {
          std::string result;


          for (
               char c :
          str
               )
          {
               switch (c)
               {
               case '"':
                    result +=
                         "\\\"";
                    break;


               case '\\':
                    result +=
                         "\\\\";
                    break;


               case '\n':
                    result +=
                         "\\n";
                    break;


               case '\r':
                    result +=
                         "\\r";
                    break;


               case '\t':
                    result +=
                         "\\t";
                    break;


               case '\b':
                    result +=
                         "\\b";
                    break;


               case '\f':
                    result +=
                         "\\f";
                    break;


               default:
                    result +=
                         c;
                    break;
               }
          }


          return result;
     }
};