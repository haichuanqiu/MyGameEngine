#pragma once
#include "Engine/GameObjectSystem.h"
#include "Engine/DataStructure.h"
#include "Serialization/EngineObjectHandle.h"
#include "MultiThread/multiThread.h"
#include "MultiThread/TransformArray.h"

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "Serialization/EngineObjectHandle.h"
#include "MultiThread/multiThread.h"
#include "MultiThread/TransformArray.h"

// ============================================================
// Shared calculation
// ============================================================

inline Vector3 CalculateSwarmPosition(
     size_t originalIndex,
     const Vector3& center,
     float currentTime,
     float currentRadius,
     float currentWaveStrength,
     float currentSpacing,
     int currentIterations)
{
     const float index = static_cast<float>(originalIndex);
     const float angle = index * currentSpacing + currentTime;

     float x = 0.0f;
     float y = 0.0f;
     float z = 0.0f;

     const int iterations = std::max(1, currentIterations);

     for (int j = 0; j < iterations; ++j)
     {
          const float offset = static_cast<float>(j) * 0.001f;

          x += std::cos(angle + offset) * currentRadius;
          z += std::sin(angle + offset) * currentRadius;
          y += std::sin(angle * 2.0f + offset) * currentWaveStrength;
     }

     const float invIterations = 1.0f / static_cast<float>(iterations);

     return Vector3(
          center.x + x * invIterations,
          center.y + y * invIterations,
          center.z + z * invIterations
     );
}

// ============================================================
// 01 - Single Thread + vector<bool> + Normal Array
// ============================================================

class Swarm_Single_VectorBool : public Component
{
public:
     std::vector<EngineObjectHandle<GameObject>> objects;

     float radius = 10.0f;
     float moveSpeed = 1.0f;
     float waveStrength = 2.0f;
     float spacing = 0.1f;
     int calculationIterations = 1;
     float time = 0.0f;

     void Update()
     {
          ENGINE_PROFILE_SCOPE("01 Single VectorBool Update");

          Transform* controllerTransform = gameObject->transform;
          if (!controllerTransform)
               return;

          const Vector3 center = controllerTransform->GetPosition();

          constexpr float dt = 1.0f / 60.0f;
          time += dt * moveSpeed;

          const size_t count = objects.size();
          if (count == 0)
               return;

          {
               ENGINE_PROFILE_SCOPE("01 Single VectorBool Prepare");

               calculatedPositions.resize(count);
               validObjects.resize(count);

               for (size_t i = 0; i < count; ++i)
               {
                    GameObject* obj = objects[i].get();
                    validObjects[i] = (obj && obj->transform);
               }
          }

          {
               ENGINE_PROFILE_SCOPE("01 Single VectorBool Calculate");

               for (size_t i = 0; i < count; ++i)
               {
                    if (!validObjects[i])
                         continue;

                    calculatedPositions[i] = CalculateSwarmPosition(
                         i, center, time, radius, waveStrength,
                         spacing, calculationIterations
                    );
               }
          }

          {
               ENGINE_PROFILE_SCOPE("01 Single VectorBool Apply");

               for (size_t i = 0; i < count; ++i)
               {
                    if (!validObjects[i])
                         continue;

                    GameObject* obj = objects[i].get();

                    if (obj && obj->transform)
                         obj->transform->SetPosition(calculatedPositions[i]);
               }
          }
     }

private:
     std::vector<Vector3> calculatedPositions;
     std::vector<bool> validObjects;
};

REFLECT_BASE(
     Swarm_Single_VectorBool,
     Component,
     FIELD(Swarm_Single_VectorBool, objects),
     FIELD(Swarm_Single_VectorBool, radius),
     FIELD(Swarm_Single_VectorBool, moveSpeed),
     FIELD(Swarm_Single_VectorBool, waveStrength),
     FIELD(Swarm_Single_VectorBool, spacing),
     FIELD(Swarm_Single_VectorBool, calculationIterations)
)

REGISTER_COMPONENT(Swarm_Single_VectorBool)


// ============================================================
// 02 - Multi Thread + vector<bool> + Normal Array
// ============================================================

struct SwarmJob_MT_VectorBool_Array : JobParallelFor
{
     float currentTime = 0.0f;
     float currentRadius = 0.0f;
     float currentWaveStrength = 0.0f;
     float currentSpacing = 0.0f;
     int currentIterations = 1;

     Vector3 center;

     const std::vector<bool>* validObjects = nullptr;
     std::vector<Vector3>* calculatedPositions = nullptr;

     void ToExecute(int i) override
     {
          if (!(*validObjects)[i])
               return;

          (*calculatedPositions)[i] = CalculateSwarmPosition(
               static_cast<size_t>(i),
               center,
               currentTime,
               currentRadius,
               currentWaveStrength,
               currentSpacing,
               currentIterations
          );
     }
};

class Swarm_MT_VectorBool_Array : public Component
{
public:
     std::vector<EngineObjectHandle<GameObject>> objects;

     float radius = 10.0f;
     float moveSpeed = 1.0f;
     float waveStrength = 2.0f;
     float spacing = 0.1f;
     int calculationIterations = 1;
     float time = 0.0f;

     void Update()
     {
          ENGINE_PROFILE_SCOPE("02 MT VectorBool Array Update");

          Transform* controllerTransform = gameObject->transform;
          if (!controllerTransform)
               return;

          const Vector3 center = controllerTransform->GetPosition();

          constexpr float dt = 1.0f / 60.0f;
          time += dt * moveSpeed;

          const size_t count = objects.size();
          if (count == 0)
               return;

          {
               ENGINE_PROFILE_SCOPE("02 MT VectorBool Array Prepare");

               calculatedPositions.resize(count);
               validObjects.resize(count);

               for (size_t i = 0; i < count; ++i)
               {
                    GameObject* obj = objects[i].get();
                    validObjects[i] = (obj && obj->transform);
               }
          }

          JobGroup group;

          {
               ENGINE_PROFILE_SCOPE("02 MT VectorBool Array Submit");

               auto job = std::make_unique<SwarmJob_MT_VectorBool_Array>();

               job->center = center;
               job->currentTime = time;
               job->currentRadius = radius;
               job->currentWaveStrength = waveStrength;
               job->currentSpacing = spacing;
               job->currentIterations = std::max(1, calculationIterations);
               job->validObjects = &validObjects;
               job->calculatedPositions = &calculatedPositions;
               job->group = &group;
               job->batchSize = 256;

               MultiThread::Get().GetThreadPool().Submit(
                    std::move(job), count, 0
               );
          }

          {
               ENGINE_PROFILE_SCOPE("02 MT VectorBool Array Wait");
               group.Wait();
          }

          {
               ENGINE_PROFILE_SCOPE("02 MT VectorBool Array Apply");

               for (size_t i = 0; i < count; ++i)
               {
                    if (!validObjects[i])
                         continue;

                    GameObject* obj = objects[i].get();

                    if (obj && obj->transform)
                         obj->transform->SetPosition(calculatedPositions[i]);
               }
          }
     }

private:
     std::vector<Vector3> calculatedPositions;
     std::vector<bool> validObjects;
};

REFLECT_BASE(
     Swarm_MT_VectorBool_Array,
     Component,
     FIELD(Swarm_MT_VectorBool_Array, objects),
     FIELD(Swarm_MT_VectorBool_Array, radius),
     FIELD(Swarm_MT_VectorBool_Array, moveSpeed),
     FIELD(Swarm_MT_VectorBool_Array, waveStrength),
     FIELD(Swarm_MT_VectorBool_Array, spacing),
     FIELD(Swarm_MT_VectorBool_Array, calculationIterations)
)

REGISTER_COMPONENT(Swarm_MT_VectorBool_Array)


// ============================================================
// 03 - Multi Thread + vector<uint8_t> + Normal Array
// ============================================================

struct SwarmJob_MT_Uint8_Array : JobParallelFor
{
     float currentTime = 0.0f;
     float currentRadius = 0.0f;
     float currentWaveStrength = 0.0f;
     float currentSpacing = 0.0f;
     int currentIterations = 1;

     Vector3 center;

     const std::vector<std::uint8_t>* validObjects = nullptr;
     std::vector<Vector3>* calculatedPositions = nullptr;

     void ToExecute(int i) override
     {
          if (!(*validObjects)[i])
               return;

          (*calculatedPositions)[i] = CalculateSwarmPosition(
               static_cast<size_t>(i),
               center,
               currentTime,
               currentRadius,
               currentWaveStrength,
               currentSpacing,
               currentIterations
          );
     }
};

class Swarm_MT_Uint8_Array : public Component
{
public:
     std::vector<EngineObjectHandle<GameObject>> objects;

     float radius = 10.0f;
     float moveSpeed = 1.0f;
     float waveStrength = 2.0f;
     float spacing = 0.1f;
     int calculationIterations = 1;
     float time = 0.0f;

     void Update()
     {
          ENGINE_PROFILE_SCOPE("03 MT Uint8 Array Update");

          Transform* controllerTransform = gameObject->transform;
          if (!controllerTransform)
               return;

          const Vector3 center = controllerTransform->GetPosition();

          constexpr float dt = 1.0f / 60.0f;
          time += dt * moveSpeed;

          const size_t count = objects.size();
          if (count == 0)
               return;

          {
               ENGINE_PROFILE_SCOPE("03 MT Uint8 Array Prepare");

               calculatedPositions.resize(count);
               validObjects.resize(count);

               for (size_t i = 0; i < count; ++i)
               {
                    GameObject* obj = objects[i].get();
                    validObjects[i] = (obj && obj->transform) ? 1 : 0;
               }
          }

          JobGroup group;

          {
               ENGINE_PROFILE_SCOPE("03 MT Uint8 Array Submit");

               auto job = std::make_unique<SwarmJob_MT_Uint8_Array>();

               job->center = center;
               job->currentTime = time;
               job->currentRadius = radius;
               job->currentWaveStrength = waveStrength;
               job->currentSpacing = spacing;
               job->currentIterations = std::max(1, calculationIterations);
               job->validObjects = &validObjects;
               job->calculatedPositions = &calculatedPositions;
               job->group = &group;
               job->batchSize = 256;

               MultiThread::Get().GetThreadPool().Submit(
                    std::move(job), count, 0
               );
          }

          {
               ENGINE_PROFILE_SCOPE("03 MT Uint8 Array Wait");
               group.Wait();
          }

          {
               ENGINE_PROFILE_SCOPE("03 MT Uint8 Array Apply");

               for (size_t i = 0; i < count; ++i)
               {
                    if (!validObjects[i])
                         continue;

                    GameObject* obj = objects[i].get();

                    if (obj && obj->transform)
                         obj->transform->SetPosition(calculatedPositions[i]);
               }
          }
     }

private:
     std::vector<Vector3> calculatedPositions;
     std::vector<std::uint8_t> validObjects;
};

REFLECT_BASE(
     Swarm_MT_Uint8_Array,
     Component,
     FIELD(Swarm_MT_Uint8_Array, objects),
     FIELD(Swarm_MT_Uint8_Array, radius),
     FIELD(Swarm_MT_Uint8_Array, moveSpeed),
     FIELD(Swarm_MT_Uint8_Array, waveStrength),
     FIELD(Swarm_MT_Uint8_Array, spacing),
     FIELD(Swarm_MT_Uint8_Array, calculationIterations)
)

REGISTER_COMPONENT(Swarm_MT_Uint8_Array)


// ============================================================
// 04 - Multi Thread + vector<bool> + TransformArray
// ============================================================

struct SwarmJob_MT_VectorBool_TransformArray : JobParallelFor
{
     float currentTime = 0.0f;
     float currentRadius = 0.0f;
     float currentWaveStrength = 0.0f;
     float currentSpacing = 0.0f;
     int currentIterations = 1;

     Vector3 center;

     MultiThreadTransformArray myArray;

     const std::vector<bool>* validObjects = nullptr;
     const std::vector<size_t>* originalIndices = nullptr;

     void ToExecute(int i) override
     {
          const size_t compactIndex = static_cast<size_t>(i);
          const size_t originalIndex = (*originalIndices)[compactIndex];

          if (!(*validObjects)[originalIndex])
               return;

          const Vector3 position = CalculateSwarmPosition(
               originalIndex,
               center,
               currentTime,
               currentRadius,
               currentWaveStrength,
               currentSpacing,
               currentIterations
          );

          myArray.setPosition(i, position);
     }
};

class Swarm_MT_VectorBool_TransformArray : public Component
{
public:
     std::vector<EngineObjectHandle<GameObject>> objects;

     float radius = 10.0f;
     float moveSpeed = 1.0f;
     float waveStrength = 2.0f;
     float spacing = 0.1f;
     int calculationIterations = 1;
     float time = 0.0f;

     void Update()
     {
          ENGINE_PROFILE_SCOPE("04 MT VectorBool TransformArray Update");

          Transform* controllerTransform = gameObject->transform;
          if (!controllerTransform)
               return;

          const Vector3 center = controllerTransform->GetPosition();

          constexpr float dt = 1.0f / 60.0f;
          time += dt * moveSpeed;

          const size_t count = objects.size();
          if (count == 0)
               return;

          {
               ENGINE_PROFILE_SCOPE("04 MT VectorBool TransformArray Prepare");

               validObjects.resize(count);
               targets.clear();
               originalIndices.clear();

               targets.reserve(count);
               originalIndices.reserve(count);

               for (size_t i = 0; i < count; ++i)
               {
                    GameObject* obj = objects[i].get();
                    const bool valid = (obj && obj->transform);

                    validObjects[i] = valid;

                    if (!valid)
                         continue;

                    targets.push_back(obj->transform);
                    originalIndices.push_back(i);
               }
          }

          if (targets.empty())
               return;

          MultiThreadTransformArray transformArray(targets);
          JobGroup group;

          {
               ENGINE_PROFILE_SCOPE("04 MT VectorBool TransformArray Submit");

               auto job =
                    std::make_unique<SwarmJob_MT_VectorBool_TransformArray>();

               job->center = center;
               job->currentTime = time;
               job->currentRadius = radius;
               job->currentWaveStrength = waveStrength;
               job->currentSpacing = spacing;
               job->currentIterations = std::max(1, calculationIterations);

               job->myArray = transformArray;
               job->validObjects = &validObjects;
               job->originalIndices = &originalIndices;
               job->group = &group;
               job->batchSize = 256;

               MultiThread::Get().GetThreadPool().Submit(
                    std::move(job), transformArray.size(), 0
               );
          }

          {
               ENGINE_PROFILE_SCOPE("04 MT VectorBool TransformArray Wait");
               group.Wait();
          }
     }

private:
     std::vector<bool> validObjects;
     std::vector<Transform*> targets;
     std::vector<size_t> originalIndices;
};

REFLECT_BASE(
     Swarm_MT_VectorBool_TransformArray,
     Component,
     FIELD(Swarm_MT_VectorBool_TransformArray, objects),
     FIELD(Swarm_MT_VectorBool_TransformArray, radius),
     FIELD(Swarm_MT_VectorBool_TransformArray, moveSpeed),
     FIELD(Swarm_MT_VectorBool_TransformArray, waveStrength),
     FIELD(Swarm_MT_VectorBool_TransformArray, spacing),
     FIELD(Swarm_MT_VectorBool_TransformArray, calculationIterations)
)

REGISTER_COMPONENT(Swarm_MT_VectorBool_TransformArray)


// ============================================================
// 05 - Multi Thread + vector<uint8_t> + TransformArray
// ============================================================

struct SwarmJob_MT_Uint8_TransformArray : JobParallelFor
{
     float currentTime = 0.0f;
     float currentRadius = 0.0f;
     float currentWaveStrength = 0.0f;
     float currentSpacing = 0.0f;
     int currentIterations = 1;

     Vector3 center;

     MultiThreadTransformArray myArray;

     const std::vector<std::uint8_t>* validObjects = nullptr;
     const std::vector<size_t>* originalIndices = nullptr;

     void ToExecute(int i) override
     {
          const size_t compactIndex = static_cast<size_t>(i);
          const size_t originalIndex = (*originalIndices)[compactIndex];

          if (!(*validObjects)[originalIndex])
               return;

          const Vector3 position = CalculateSwarmPosition(
               originalIndex,
               center,
               currentTime,
               currentRadius,
               currentWaveStrength,
               currentSpacing,
               currentIterations
          );

          myArray.setPosition(i, position);
     }
};

class Swarm_MT_Uint8_TransformArray : public Component
{
public:
     std::vector<EngineObjectHandle<GameObject>> objects;

     float radius = 10.0f;
     float moveSpeed = 1.0f;
     float waveStrength = 2.0f;
     float spacing = 0.1f;
     int calculationIterations = 1;
     float time = 0.0f;

     void Update()
     {
          ENGINE_PROFILE_SCOPE("05 MT Uint8 TransformArray Update");

          Transform* controllerTransform = gameObject->transform;
          if (!controllerTransform)
               return;

          const Vector3 center = controllerTransform->GetPosition();

          constexpr float dt = 1.0f / 60.0f;
          time += dt * moveSpeed;

          const size_t count = objects.size();
          if (count == 0)
               return;

          {
               ENGINE_PROFILE_SCOPE("05 MT Uint8 TransformArray Prepare");

               validObjects.resize(count);
               targets.clear();
               originalIndices.clear();

               targets.reserve(count);
               originalIndices.reserve(count);

               for (size_t i = 0; i < count; ++i)
               {
                    GameObject* obj = objects[i].get();
                    const bool valid = (obj && obj->transform);

                    validObjects[i] = valid ? 1 : 0;

                    if (!valid)
                         continue;

                    targets.push_back(obj->transform);
                    originalIndices.push_back(i);
               }
          }

          if (targets.empty())
               return;

          MultiThreadTransformArray transformArray(targets);
          JobGroup group;

          {
               ENGINE_PROFILE_SCOPE("05 MT Uint8 TransformArray Submit");

               auto job =
                    std::make_unique<SwarmJob_MT_Uint8_TransformArray>();

               job->center = center;
               job->currentTime = time;
               job->currentRadius = radius;
               job->currentWaveStrength = waveStrength;
               job->currentSpacing = spacing;
               job->currentIterations = std::max(1, calculationIterations);

               job->myArray = transformArray;
               job->validObjects = &validObjects;
               job->originalIndices = &originalIndices;
               job->group = &group;
               job->batchSize = 256;

               MultiThread::Get().GetThreadPool().Submit(
                    std::move(job), transformArray.size(), 0
               );
          }

          {
               ENGINE_PROFILE_SCOPE("05 MT Uint8 TransformArray Wait");
               group.Wait();
          }
     }

private:
     std::vector<std::uint8_t> validObjects;
     std::vector<Transform*> targets;
     std::vector<size_t> originalIndices;
};

REFLECT_BASE(
     Swarm_MT_Uint8_TransformArray,
     Component,
     FIELD(Swarm_MT_Uint8_TransformArray, objects),
     FIELD(Swarm_MT_Uint8_TransformArray, radius),
     FIELD(Swarm_MT_Uint8_TransformArray, moveSpeed),
     FIELD(Swarm_MT_Uint8_TransformArray, waveStrength),
     FIELD(Swarm_MT_Uint8_TransformArray, spacing),
     FIELD(Swarm_MT_Uint8_TransformArray, calculationIterations)
)

REGISTER_COMPONENT(Swarm_MT_Uint8_TransformArray)
