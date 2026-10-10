#pragma once
#include "Engine/GameObjectSystem.h"
#include "Engine/DataStructure.h"
#include "Serialization/EngineObjectHandle.h"
#include "MultiThread/multiThread.h"
class ObjectSwarmController : public Component
{
public:

     std::vector<EngineObjectHandle<GameObject>> objects;

     // 控制参数
     float radius = 10.0f;
     float moveSpeed = 1.0f;
     float waveStrength = 2.0f;
     float spacing = 0.1f;

     int calculationIterations = 1;

     float time = 0.0f;


     void Update()
     {
         
          ENGINE_PROFILE_SCOPE("NorSwarmUpdate");

          Transform* controllerTransform =
               gameObject->transform;

          if (!controllerTransform)
               return;

          Vector3 center =
               controllerTransform->GetPosition();

          constexpr float dt = 1.0f / 60.0f;

          time += dt * moveSpeed;

          const size_t objectCount = objects.size();

          if (objectCount == 0)
               return;


          // ========================================================
          // Prepare
          // ========================================================

          {
               ENGINE_PROFILE_SCOPE("NorSwarm::Prepare");

               calculatedPositions.resize(objectCount);
               validObjects.resize(objectCount);

               for (size_t i = 0; i < objectCount; ++i)
               {
                    GameObject* obj = objects[i].get();

                    if (!obj || !obj->transform)
                    {
                         validObjects[i] = false;
                         continue;
                    }

                    validObjects[i] = true;
               }
          }


          // ========================================================
          // Calculate
          //
          // 单线程先计算全部位置
          // 不调用 SetPosition
          // ========================================================

          {
               ENGINE_PROFILE_SCOPE("NorSwarm::Calculate");
               
               for (size_t i = 0; i < objectCount; ++i)
               {
                    if (!validObjects[i])
                         continue;

                    float index = static_cast<float>(i);

                    float angle =
                         index * spacing +
                         time;

                    float x = 0.0f;
                    float y = 0.0f;
                    float z = 0.0f;

                    for (int j = 0; j < calculationIterations; ++j)
                    {
                         float offset =
                              static_cast<float>(j) * 0.001f;

                         x += std::cos(angle + offset) * radius;

                         z += std::sin(angle + offset) * radius;

                         y += std::sin(
                              angle * 2.0f + offset
                         ) * waveStrength;
                    }

                    float invIterations =
                         1.0f / static_cast<float>(
                              std::max(1, calculationIterations)
                              );

                    x *= invIterations;
                    y *= invIterations;
                    z *= invIterations;

                    calculatedPositions[i] = Vector3(
                         center.x + x,
                         center.y + y,
                         center.z + z
                    );
               }
          }


          // ========================================================
          // Apply Transforms
          //
          // 全部计算完成之后
          // 再统一 SetPosition
          // ========================================================

          {
               ENGINE_PROFILE_SCOPE("NorSwarm::ApplyTransforms");

               for (size_t i = 0; i < objectCount; ++i)
               {
                    if (!validObjects[i])
                         continue;

                    GameObject* obj = objects[i].get();

                    if (!obj)
                         continue;

                    Transform* transform = obj->transform;

                    if (!transform)
                         continue;

                    transform->SetPosition(
                         calculatedPositions[i]
                    );
               }
          }
     }


private:

     std::vector<Vector3> calculatedPositions;

     std::vector<bool> validObjects;
};



REFLECT_BASE(
     ObjectSwarmController,
     Component,

     FIELD(ObjectSwarmController, objects),

     FIELD(ObjectSwarmController, radius),
     FIELD(ObjectSwarmController, moveSpeed),
     FIELD(ObjectSwarmController, waveStrength),
     FIELD(ObjectSwarmController, spacing),

     FIELD(ObjectSwarmController, calculationIterations)
)

REGISTER_COMPONENT(ObjectSwarmController)

class ObjectSwarmControllerParallel : public Component//fast in release, slow in debug
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
          ENGINE_PROFILE_SCOPE("Swarm parallel update");

          Transform* controllerTransform =
               gameObject->transform;

          if (!controllerTransform)
          {
               return;
          }


          const Vector3 center =
               controllerTransform->GetPosition();


          constexpr float dt = 1.0f / 60.0f;

          time += dt * moveSpeed;


          const size_t objectCount =
               objects.size();


          if (objectCount == 0)
          {
               return;
          }


          // ============================================================
          // Prepare
          //
          // Game Thread
          // ============================================================

          { ENGINE_PROFILE_SCOPE("Swarm::Prepare");


               calculatedPositions.resize(
                    objectCount
               );


               validObjects.resize(
                    objectCount
               );


               for (size_t i = 0;
                    i < objectCount;
                    ++i)
               {
                    GameObject* obj =
                         objects[i].get();


                    if (!obj ||
                         !obj->transform)
                    {
                         validObjects[i] = false;

                         continue;
                    }


                    validObjects[i] = true;
               }
          }


          // ============================================================
          // Snapshot
          // ============================================================

          const float currentTime =
               time;

          const float currentRadius =
               radius;

          const float currentWaveStrength =
               waveStrength;

          const float currentSpacing =
               spacing;


          const int currentIterations =
               std::max(
                    1,
                    calculationIterations
               );


          JobGroup group;


          constexpr size_t batchSize =
               256;


          ThreadPool& threadPool =
               GetThreadPool();


          // ============================================================
          // Submit Jobs
          //
          // Game Thread
          //
          // 这里只测：
          //
          // Job 创建
          // JobGroup AddJob
          // Queue Push
          // Mutex
          // notify_one
          //
          // 不包含 Wait。
          // ============================================================

         


               for (size_t begin = 0;
                    begin < objectCount;
                    begin += batchSize)
               {
                    const size_t end =
                         std::min(
                              begin + batchSize,
                              objectCount
                         );


                    threadPool.Submit(

                         [this,
                         begin,
                         end,
                         center,
                         currentTime,
                         currentRadius,
                         currentWaveStrength,
                         currentSpacing,
                         currentIterations]()
                         {
                              std::string s = "Swarm::WorkerJob"
                                   + std::to_string(end - begin)
                                   + "Items";
                              ENGINE_PROFILE_SCOPE(s);
                              for (size_t i = begin;
                                   i < end;
                                   ++i)
                              {
                                   if (!validObjects[i])
                                   {
                                        continue;
                                   }


                                   const float index =
                                        static_cast<float>(
                                             i
                                             );


                                   const float angle =
                                        index *
                                        currentSpacing +
                                        currentTime;


                                   float x = 0.0f;
                                   float y = 0.0f;
                                   float z = 0.0f;


                                   for (int j = 0;
                                        j < currentIterations;
                                        ++j)
                                   {
                                        const float offset =
                                             static_cast<float>(
                                                  j
                                                  ) * 0.001f;


                                        x +=
                                             std::cos(
                                                  angle +
                                                  offset
                                             ) *
                                             currentRadius;


                                        z +=
                                             std::sin(
                                                  angle +
                                                  offset
                                             ) *
                                             currentRadius;


                                        y +=
                                             std::sin(
                                                  angle * 2.0f +
                                                  offset
                                             ) *
                                             currentWaveStrength;
                                   }


                                   const float invIterations =
                                        1.0f /
                                        static_cast<float>(
                                             currentIterations
                                             );


                                   x *= invIterations;
                                   y *= invIterations;
                                   z *= invIterations;


                                   calculatedPositions[i] =
                                        Vector3(
                                             center.x + x,
                                             center.y + y,
                                             center.z + z
                                        );
                              }
                         },

                         &group
                    );
               }
          


          // ============================================================
          // Wait
          //
          // Game Thread
          //
          // 这个 Scope 非常重要。
          //
          // 它告诉你 Game Thread 因为 Worker
          // 到底阻塞了多长时间。
          // ============================================================

          {ENGINE_PROFILE_SCOPE("Swarm::WaitWorkers");


               group.Wait();
          }


          // ============================================================
          // Apply
          //
          // Game Thread
          //
          // 真正操作 GameObject / Transform
          // ============================================================

          {ENGINE_PROFILE_SCOPE("Swarm::ApplyTransforms");


               for (size_t i = 0;
                    i < objectCount;
                    ++i)
               {
                    if (!validObjects[i])
                    {
                         continue;
                    }


                    GameObject* obj =
                         objects[i].get();


                    if (!obj)
                    {
                         continue;
                    }


                    Transform* transform =
                         obj->transform;


                    if (!transform)
                    {
                         continue;
                    }


                    transform->SetPosition(
                         calculatedPositions[i]
                    );
               }
          }
     }


private:

     std::vector<Vector3>
          calculatedPositions;


     std::vector<bool>
          validObjects;


     ThreadPool& GetThreadPool()
     {
          return
               MultiThread::Get()
               .GetThreadPool();
     }
};


REFLECT_BASE(
     ObjectSwarmControllerParallel,
     Component,

     FIELD(
          ObjectSwarmControllerParallel,
          objects
     ),

     FIELD(
          ObjectSwarmControllerParallel,
          radius
     ),

     FIELD(
          ObjectSwarmControllerParallel,
          moveSpeed
     ),

     FIELD(
          ObjectSwarmControllerParallel,
          waveStrength
     ),

     FIELD(
          ObjectSwarmControllerParallel,
          spacing
     ),

     FIELD(
          ObjectSwarmControllerParallel,
          calculationIterations
     )
)


REGISTER_COMPONENT(
     ObjectSwarmControllerParallel
)

#include <cstdint>
class ObjectSwarmControllerRemoveLock : public Component//fast in release and debug
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
          ENGINE_PROFILE_SCOPE("Swarm parallel update");

          Transform* controllerTransform =
               gameObject->transform;

          if (!controllerTransform)
          {
               return;
          }


          const Vector3 center =
               controllerTransform->GetPosition();


          constexpr float dt = 1.0f / 60.0f;

          time += dt * moveSpeed;


          const size_t objectCount =
               objects.size();


          if (objectCount == 0)
          {
               return;
          }


          // ============================================================
          // Prepare
          //
          // Game Thread
          // ============================================================

          {
               ENGINE_PROFILE_SCOPE("Swarm::Prepare");


               calculatedPositions.resize(
                    objectCount
               );


               validObjects.resize(
                    objectCount
               );


               for (size_t i = 0;
                    i < objectCount;
                    ++i)
               {
                    GameObject* obj =
                         objects[i].get();


                    if (!obj ||
                         !obj->transform)
                    {
                         validObjects[i] = 0;

                         continue;
                    }


                    validObjects[i] = 1;
               }
          }


          // ============================================================
          // Snapshot
          // ============================================================

          const float currentTime =
               time;

          const float currentRadius =
               radius;

          const float currentWaveStrength =
               waveStrength;

          const float currentSpacing =
               spacing;


          const int currentIterations =
               std::max(
                    1,
                    calculationIterations
               );


          JobGroup group;


          constexpr size_t batchSize =
               256;


          ThreadPool& threadPool =
               GetThreadPool();


          // ============================================================
          // Submit Jobs
          //
          // Game Thread
          //
          // 这里只测：
          //
          // Job 创建
          // JobGroup AddJob
          // Queue Push
          // Mutex
          // notify_one
          //
          // 不包含 Wait。
          // ============================================================




          for (size_t begin = 0;
               begin < objectCount;
               begin += batchSize)
          {
               const size_t end =
                    std::min(
                         begin + batchSize,
                         objectCount
                    );


               threadPool.Submit(

                    [this,
                    begin,
                    end,
                    center,
                    currentTime,
                    currentRadius,
                    currentWaveStrength,
                    currentSpacing,
                    currentIterations]()
                    {
                         std::string s = "Swarm::WorkerJob"
                              + std::to_string(end - begin)
                              + "Items";
                         ENGINE_PROFILE_SCOPE(s);
                         for (size_t i = begin;
                              i < end;
                              ++i)
                         {
                              if (!validObjects[i])
                              {
                                   continue;
                              }


                              const float index =
                                   static_cast<float>(
                                        i
                                        );


                              const float angle =
                                   index *
                                   currentSpacing +
                                   currentTime;


                              float x = 0.0f;
                              float y = 0.0f;
                              float z = 0.0f;


                              for (int j = 0;
                                   j < currentIterations;
                                   ++j)
                              {
                                   const float offset =
                                        static_cast<float>(
                                             j
                                             ) * 0.001f;


                                   x +=
                                        std::cos(
                                             angle +
                                             offset
                                        ) *
                                        currentRadius;


                                   z +=
                                        std::sin(
                                             angle +
                                             offset
                                        ) *
                                        currentRadius;


                                   y +=
                                        std::sin(
                                             angle * 2.0f +
                                             offset
                                        ) *
                                        currentWaveStrength;
                              }


                              const float invIterations =
                                   1.0f /
                                   static_cast<float>(
                                        currentIterations
                                        );


                              x *= invIterations;
                              y *= invIterations;
                              z *= invIterations;


                              calculatedPositions[i] =
                                   Vector3(
                                        center.x + x,
                                        center.y + y,
                                        center.z + z
                                   );
                         }
                    },

                    &group
               );
          }



          // ============================================================
          // Wait
          //
          // Game Thread
          //
          // 这个 Scope 非常重要。
          //
          // 它告诉你 Game Thread 因为 Worker
          // 到底阻塞了多长时间。
          // ============================================================

          {ENGINE_PROFILE_SCOPE("Swarm::WaitWorkers");


          group.Wait();
          }


          // ============================================================
          // Apply
          //
          // Game Thread
          //
          // 真正操作 GameObject / Transform
          // ============================================================

          {
               ENGINE_PROFILE_SCOPE("Swarm::ApplyTransforms");


               for (size_t i = 0;
                    i < objectCount;
                    ++i)
               {
                    if (!validObjects[i])
                    {
                         continue;
                    }


                    GameObject* obj =
                         objects[i].get();


                    if (!obj)
                    {
                         continue;
                    }


                    Transform* transform =
                         obj->transform;


                    if (!transform)
                    {
                         continue;
                    }


                    transform->SetPosition(
                         calculatedPositions[i]
                    );
               }
          }
     }


private:

     std::vector<Vector3>
          calculatedPositions;


     std::vector<std::uint8_t> validObjects;


     ThreadPool& GetThreadPool()
     {
          return
               MultiThread::Get()
               .GetThreadPool();
     }
};