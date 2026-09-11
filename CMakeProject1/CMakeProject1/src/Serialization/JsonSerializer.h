#pragma once

#include "Reflection.h"

#include <any>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class JsonSerializer
{
private:

     // ============================================================
     // Simple JSON Value
     //
     // 这里只支持当前 Reflection 实际需要的：
     //
     // int
     // float
     // bool
     // object
     // array
     // null
     //
     // 暂时不支持 string，因为你的 FieldType 目前也没有 String。
     // ============================================================

     struct JsonValue
     {
          enum class Type
          {
               Null,
               Number,
               Bool,
               Object,
               Array
          };

          Type type = Type::Null;

          // Number 同时保存 int / float
          double number = 0.0;

          bool boolean = false;

          std::vector<
               std::pair<std::string, JsonValue>
          > object;

          std::vector<JsonValue> array;
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


          bool Parse(JsonValue& out)
          {
               SkipWhitespace();

               if (!ParseValue(out))
                    return false;

               SkipWhitespace();

               return m_Position == m_Text.size();
          }


     private:

          // ========================================================
          // Parse Value
          // ========================================================

          bool ParseValue(JsonValue& out)
          {
               SkipWhitespace();

               if (m_Position >= m_Text.size())
                    return false;

               char c = m_Text[m_Position];

               // Object
               if (c == '{')
               {
                    return ParseObject(out);
               }

               // Array
               if (c == '[')
               {
                    return ParseArray(out);
               }

               // Bool
               if (c == 't' || c == 'f')
               {
                    return ParseBool(out);
               }

               // Null
               if (c == 'n')
               {
                    return ParseNull(out);
               }

               // Number
               if (
                    c == '-' ||
                    c == '+' ||
                    std::isdigit(
                         static_cast<unsigned char>(c)
                    )
                    )
               {
                    return ParseNumber(out);
               }

               return false;
          }


          // ========================================================
          // Parse Object
          // ========================================================

          bool ParseObject(JsonValue& out)
          {
               if (!Consume('{'))
                    return false;

               out = JsonValue{};
               out.type = JsonValue::Type::Object;

               SkipWhitespace();

               // Empty object
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

          bool ParseArray(JsonValue& out)
          {
               if (!Consume('['))
                    return false;

               out = JsonValue{};
               out.type = JsonValue::Type::Array;

               SkipWhitespace();

               // Empty array
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

          bool ParseBool(JsonValue& out)
          {
               if (Match("true"))
               {
                    out = JsonValue{};
                    out.type = JsonValue::Type::Bool;
                    out.boolean = true;
                    return true;
               }

               if (Match("false"))
               {
                    out = JsonValue{};
                    out.type = JsonValue::Type::Bool;
                    out.boolean = false;
                    return true;
               }

               return false;
          }


          // ========================================================
          // Parse Null
          // ========================================================

          bool ParseNull(JsonValue& out)
          {
               if (!Match("null"))
                    return false;

               out = JsonValue{};
               out.type = JsonValue::Type::Null;

               return true;
          }


          // ========================================================
          // Parse Number
          // ========================================================

          bool ParseNumber(JsonValue& out)
          {
               size_t begin =
                    m_Position;

               if (
                    m_Position < m_Text.size() &&
                    (
                         m_Text[m_Position] == '-' ||
                         m_Text[m_Position] == '+'
                         )
                    )
               {
                    ++m_Position;
               }

               bool hasDigits = false;

               while (
                    m_Position < m_Text.size() &&
                    std::isdigit(
                         static_cast<unsigned char>(
                              m_Text[m_Position]
                              )
                    )
                    )
               {
                    hasDigits = true;
                    ++m_Position;
               }

               // Decimal
               if (
                    m_Position < m_Text.size() &&
                    m_Text[m_Position] == '.'
                    )
               {
                    ++m_Position;

                    while (
                         m_Position < m_Text.size() &&
                         std::isdigit(
                              static_cast<unsigned char>(
                                   m_Text[m_Position]
                                   )
                         )
                         )
                    {
                         hasDigits = true;
                         ++m_Position;
                    }
               }

               // Scientific notation
               if (
                    m_Position < m_Text.size() &&
                    (
                         m_Text[m_Position] == 'e' ||
                         m_Text[m_Position] == 'E'
                         )
                    )
               {
                    ++m_Position;

                    if (
                         m_Position < m_Text.size() &&
                         (
                              m_Text[m_Position] == '+' ||
                              m_Text[m_Position] == '-'
                              )
                         )
                    {
                         ++m_Position;
                    }

                    while (
                         m_Position < m_Text.size() &&
                         std::isdigit(
                              static_cast<unsigned char>(
                                   m_Text[m_Position]
                                   )
                         )
                         )
                    {
                         hasDigits = true;
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
                    out = JsonValue{};

                    out.type =
                         JsonValue::Type::Number;

                    out.number =
                         std::stod(numberText);
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
                    m_Position < m_Text.size()
                    )
               {
                    char c =
                         m_Text[m_Position++];

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
                         m_Text[m_Position++];

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
                         // 暂时不处理 unicode escape
                         return false;
                    }
               }

               return false;
          }


          // ========================================================
          // Whitespace
          // ========================================================

          void SkipWhitespace()
          {
               while (
                    m_Position < m_Text.size() &&
                    std::isspace(
                         static_cast<unsigned char>(
                              m_Text[m_Position]
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

          bool Consume(char expected)
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
                    std::strlen(text);

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

               m_Position += length;

               return true;
          }


     private:

          const std::string& m_Text;

          size_t m_Position = 0;
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
               FindRuntimeType(value);

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

          return SerializePropertyNode(node);
     }


     // ============================================================
     // Deserialize
     //
     // data:
     //
     // {
     //     "FieldOfView": 60,
     //     "NearClip": 0.1,
     //     "Perspective": true
     // }
     // ============================================================

     template<typename T>
     static bool Deserialize(
          T& value,
          const std::string& data)
     {
          const TypeInfo* info =
               FindRuntimeType(value);

          if (!info)
               return false;

          JsonValue root;

          Parser parser(data);

          if (!parser.Parse(root))
               return false;

          if (
               root.type !=
               JsonValue::Type::Object
               )
          {
               return false;
          }

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

          file << Serialize(value);

          return file.good();
     }


private:

     // ============================================================
     // Apply Object
     //
     // PropertyNode:
     //
     // target
     //   ├── FieldOfView
     //   ├── NearClip
     //   └── FarClip
     //
     // JsonValue:
     //
     // {
     //   "FieldOfView": 60,
     //   "NearClip": 0.1,
     //   "FarClip": 1000
     // }
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

          bool success = true;

          for (auto& child :
               node.children)
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
                    success = false;
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
          switch (node.type)
          {
          case FieldType::Int:
          {
               if (
                    value.type !=
                    JsonValue::Type::Number
                    )
               {
                    return false;
               }

               if (!node.set)
                    return false;

               try
               {
                    int intValue =
                         static_cast<int>(
                              value.number
                              );

                    node.set(
                         std::any(intValue)
                    );

                    return true;
               }
               catch (...)
               {
                    return false;
               }
          }

          case FieldType::Float:
          {
               if (
                    value.type !=
                    JsonValue::Type::Number
                    )
               {
                    return false;
               }

               if (!node.set)
                    return false;

               try
               {
                    float floatValue =
                         static_cast<float>(
                              value.number
                              );

                    node.set(
                         std::any(floatValue)
                    );

                    return true;
               }
               catch (...)
               {
                    return false;
               }
          }

          case FieldType::Bool:
          {
               if (
                    value.type !=
                    JsonValue::Type::Bool
                    )
               {
                    return false;
               }

               if (!node.set)
                    return false;

               try
               {
                    node.set(
                         std::any(value.boolean)
                    );

                    return true;
               }
               catch (...)
               {
                    return false;
               }
          }

          case FieldType::Struct:
          {
               return ApplyObject(
                    node,
                    value
               );
          }

          case FieldType::Vector:
          {
               return ApplyArray(
                    node,
                    value
               );
          }

          default:
               return false;
          }
     }


     // ============================================================
     // Apply Array
     //
     // 例如 Vector3 如果未来使用 vector<float>：
     //
     // [
     //     1,
     //     2,
     //     3
     // ]
     //
     // 或 vector<Vector3>
     //
     // [
     //     {...},
     //     {...}
     // ]
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

          bool success = true;

          const size_t count =
               std::min(
                    node.children.size(),
                    value.array.size()
               );

          for (size_t i = 0;
               i < count;
               ++i)
          {
               if (
                    !ApplyNode(
                         node.children[i],
                         value.array[i]
                    )
                    )
               {
                    success = false;
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

          for (const auto& [key, value] :
               object.object)
          {
               if (key == name)
                    return &value;
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
          switch (node.type)
          {
          case FieldType::Int:
          {
               out <<
                    std::any_cast<int>(
                         node.value
                    );

               break;
          }

          case FieldType::Float:
          {
               out <<
                    std::any_cast<float>(
                         node.value
                    );

               break;
          }

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

          case FieldType::Struct:
          {
               WriteObject(
                    out,
                    node,
                    indent
               );

               break;
          }

          case FieldType::Vector:
          {
               WriteArray(
                    out,
                    node,
                    indent
               );

               break;
          }

          default:
          {
               out << "null";

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
          out << "{";

          if (!node.children.empty())
          {
               out << "\n";

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
                         "\"" <<
                         Escape(child.name) <<
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
                         out << ",";
                    }

                    out << "\n";
               }

               WriteIndent(
                    out,
                    indent
               );
          }

          out << "}";
     }


     // ============================================================
     // Write Array
     // ============================================================

     static void WriteArray(
          std::ostringstream& out,
          const PropertyNode& node,
          int indent)
     {
          out << "[";

          if (!node.children.empty())
          {
               out << "\n";

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
                         out << ",";
                    }

                    out << "\n";
               }

               WriteIndent(
                    out,
                    indent
               );
          }

          out << "]";
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

          for (char c : str)
          {
               switch (c)
               {
               case '"':
                    result += "\\\"";
                    break;

               case '\\':
                    result += "\\\\";
                    break;

               case '\n':
                    result += "\\n";
                    break;

               case '\r':
                    result += "\\r";
                    break;

               case '\t':
                    result += "\\t";
                    break;

               default:
                    result += c;
                    break;
               }
          }

          return result;
     }
};