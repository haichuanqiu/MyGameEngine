#pragma once

#include "ThreadPool.h"

#include <thread>
#include <iostream>


class MultiThread
{
public:

     // ============================================================
     // Singleton
     //
     // 第一次调用 Get() 时：
     //
     // static MultiThread instance
     //
     // 会自动构造。
     //
     // 构造函数内部会自动 Initialize()。
     // ============================================================

     static MultiThread& Get()
     {
          static MultiThread instance;

          return instance;
     }


     // ============================================================
     // 获取 ThreadPool
     //
     // 不需要再手动 Initialize。
     // ============================================================

     ThreadPool& GetThreadPool()
     {
          return threadPool;
     }


     const ThreadPool& GetThreadPool() const
     {
          return threadPool;
     }


     // ============================================================
     // Worker 数量
     // ============================================================

     unsigned int GetWorkerCount() const
     {
          return workerCount;
     }


     // ============================================================
     // 是否已经初始化
     // ============================================================

     bool IsInitialized() const
     {
          return initialized;
     }


     // ============================================================
     // Shutdown
     //
     // 仍然可以手动 Shutdown。
     //
     // 同时析构函数也会自动调用 Shutdown。
     // ============================================================

     void Shutdown()
     {
          if (!initialized)
          {
               return;
          }


          std::cout
               << "[MultiThread] Shutdown BEGIN\n";


          threadPool.Shutdown();


          workerCount = 0;

          initialized = false;


          std::cout
               << "[MultiThread] Shutdown END\n";
     }


private:

     // ============================================================
     // Constructor
     //
     // static instance 第一次创建的时候自动执行。
     // ============================================================

     MultiThread()
     {
          std::cout
               << "[MultiThread] Constructor\n";


          Initialize();
     }


     // ============================================================
     // Destructor
     //
     // 程序退出时 static instance 会被销毁。
     //
     // 自动 Shutdown。
     // ============================================================

     ~MultiThread()
     {
          std::cout
               << "[MultiThread] Destructor\n";


          Shutdown();
     }


     // ============================================================
     // Initialize
     //
     // 现在变成 private。
     //
     // 外部不需要调用。
     // ============================================================

     void Initialize()
     {
          if (initialized)
          {
               return;
          }


          std::cout
               << "[MultiThread] Initialize BEGIN\n";


          // ========================================================
          // 获取 Logical Processor 数量
          // ========================================================

          unsigned int hardwareThreads =
               std::thread::hardware_concurrency();


          // hardware_concurrency() 理论上允许返回 0。
          if (hardwareThreads == 0)
          {
               hardwareThreads = 4;
          }


          std::cout
               << "[MultiThread] Hardware Threads = "
               << hardwareThreads
               << "\n";


          // ========================================================
          // 第一版策略：
          //
          // 留一个 Logical Processor 给 Game Thread。
          // ========================================================

          workerCount =
               hardwareThreads > 1
               ? hardwareThreads - 1
               : 1;

          std::cout
               << "[MultiThread] Worker Count = "
               << workerCount
               << "\n";


          // ========================================================
          // 启动 ThreadPool
          // ========================================================

          threadPool.Start(
               static_cast<int>(workerCount)
          );


          initialized = true;


          std::cout
               << "[MultiThread] Initialize END\n";
     }


     // ============================================================
     // 禁止 Copy
     // ============================================================

     MultiThread(
          const MultiThread&
     ) = delete;


     MultiThread& operator=(
          const MultiThread&
          ) = delete;


private:

     // ============================================================
     // General CPU ThreadPool
     // ============================================================

     ThreadPool threadPool;


     // ============================================================
     // Worker 数量
     // ============================================================

     unsigned int workerCount = 0;


     // ============================================================
     // 初始化状态
     // ============================================================

     bool initialized = false;
};