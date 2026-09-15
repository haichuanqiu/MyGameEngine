#pragma once
#include "ReferenceDescription.h"
class EngineObject;

class ReferenceResolver
{
public:

     EngineObject* GetItem(
          ReferenceDescription refDes
     );

     template<typename T>
     std::vector<T*> FindAllOfType(
          ReferenceDescription targetScope
     );
     template<typename T>
     T* GetItemOfType(
          ReferenceDescription refDes
     )
     {
          EngineObject* object =
               GetItem(refDes);

          if (!object)
               return nullptr;

          return dynamic_cast<T*>(object);
     }


     static ReferenceResolver& Instance()
     {
          static ReferenceResolver instance;
          return instance;
     }
};