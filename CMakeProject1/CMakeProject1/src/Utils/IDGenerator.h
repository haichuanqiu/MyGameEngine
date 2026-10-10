#pragma once

#include <cstdint>
#include <functional>
#include <random>
#include <iostream>

class IDGenerator
{
public:
     static int GenerateObjectID(
          const std::function<bool(int)>& isUsed)
     {
          static std::mt19937 generator{
              std::random_device{}()
          };

          static std::uniform_int_distribution<int> distribution(
               1,
               std::numeric_limits<int>::max()
          );

          int id;

          do
          {
               id = distribution(generator);
          } while (isUsed(id));

//          std::cout << id << std::endl;

          return id;
     }
};