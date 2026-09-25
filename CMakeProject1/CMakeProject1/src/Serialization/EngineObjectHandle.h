#pragma once
#include "Assets/ReferenceResolver.h"
template<class T>
class EngineObjectHandle
{
public:
     const ReferenceDescription& GetReferenceInfo() const
     {
          return ReferenceInfo;
     }
     void SetReferenceInfo(const ReferenceDescription& ref)
     {
          ReferenceInfo = ref;
     }

     void Reset()
     {
          ReferenceInfo = ReferenceDescription{};
     }

     T* get() const
     {
     
          return ReferenceResolver::Instance()
               .GetItemOfType<T>(ReferenceInfo);
     }

private:
     ReferenceDescription ReferenceInfo;
};