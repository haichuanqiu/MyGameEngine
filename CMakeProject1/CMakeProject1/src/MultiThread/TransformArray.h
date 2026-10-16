#pragma once
#include "Engine/GameObjectSystem/Transform.h"
#include "vector"

class MultiThreadTransformArray {
public:
     std::vector<Transform*> array;
     MultiThreadTransformArray() = default;
     explicit MultiThreadTransformArray(
          const std::vector<Transform*>& transforms
     ) : array(transforms) {
          setAllDirty();
     }
     void setAllDirty() {
          auto& dirtyList = Transform::MainThreadDirtyList;

          dirtyList.insert(
               dirtyList.end(),
               array.begin(),
               array.end()
          );
     }

     inline void setPosition(size_t index, const Vector3& target) {
          array[index]->SetPositionInternal(target);
     }

     inline void setRotation(size_t index, const Quaternion& target) {
          array[index]->SetRotationInternal(target);
     }

     inline void setScale(size_t index, const Vector3& target) {
          array[index]->SetScaleInternal(target);
     }

     inline size_t size() const {
          return array.size();
     }
};