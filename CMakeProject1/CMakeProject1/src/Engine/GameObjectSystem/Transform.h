#pragma once

#include "Engine/GameObjectSystem/GameObject.h"
#include "Engine/DataStructure.h"
#include "Serialization/ComponentRegistry.h"
#include "Event.h"
#include <unordered_set>
class Transform : public Component
{
public:
     inline static std::vector<Transform*> MainThreadDirtyList{};
     inline static std::unordered_set<Transform*> DirtySet{};

     Event<const Transform&> OnChange;

public:
     // =========================
     // Position
     // =========================

     const Vector3& GetPosition() const
     {
          return position;
     }

     static void InvokeOnChange() {
          for(auto var : MainThreadDirtyList)
          {
               var->OnChange.Invoke(*var);
          }
          MainThreadDirtyList.clear();
          DirtySet.clear();
     }

     // =========================
     // Scale
     // =========================

     const Vector3& GetScale() const
     {
          return scale;
     }
     void SetScaleInternal(const Vector3& value)
     {
          scale = value;
     }

     // =========================
     // Rotation
     // =========================
     void SetPositionInternal(const Vector3& value)
     {
          position = value;
     }

     void SetRotationInternal(const Quaternion& value)
     {
          rotation = value;
     }
     void SetScale(const Vector3& value)
     {
          SetScaleInternal(value);
          setDirty();
     }

     // =========================
     // Rotation
     // =========================
     void SetPosition(const Vector3& value)
     {
          SetPositionInternal( value);
          setDirty();
     }

     void SetRotation(const Quaternion& value)
     {
          SetRotationInternal(value);
          setDirty();
     }
     const Quaternion& GetRotation() const
     {
          return rotation;
     }


     void setDirty()
     {
          if (DirtySet.insert(this).second) {
               MainThreadDirtyList.push_back(this);
          }
     }
private:
     
private:
     Vector3 position;
     Vector3 scale{ 1.0f, 1.0f, 1.0f };
     Quaternion rotation;

     REFLECT_FRIEND(Transform);
};

REFLECT_BASE(
     Transform,
     Component,
     FIELD(Transform, position),
     FIELD(Transform, scale),
     FIELD(Transform, rotation),
)
REGISTER_COMPONENT(Transform)