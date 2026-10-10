#pragma once
#include "Engine/Components/preformanceTest.h"
namespace heavyloadTestScene
{
	void OnSceneCreated(Engine& engine)
	{
		constexpr int OBJECT_COUNT = 10240;
		constexpr int Iteration = 1;
		constexpr int GRID_WIDTH = 100;
		constexpr float OBJECT_SPACING = 1.5f;

		std::cout
			<< "Start creating multithread test scene..."
			<< std::endl;


		// ============================================================
		// Light
		// ============================================================

		GameObject& light =
			engine.currentScene->AddGameObject();

		light.name = "Light";

		light.transform->SetPosition(
			Vector3(
				2.0f,
				10.0f,
				2.0f
			)
		);

		PointLight* lightComponent =
			light.AddComponent<PointLight>();

		lightComponent->Intensity = 10.0f;

		lightComponent->UpdateLightData(
			engine.renderSystem
		);

		lightComponent->SetDefaultTarget(
			&engine.renderSystem
		);

		engine.renderSystem
			.GetLighting()
			.UpdateAmbientLightData(
				glm::vec3(
					0.2f,
					0.2f,
					0.2f
				)
			);


		// ============================================================
		// Shared Material
		// ============================================================

		Material material;

		material.setVertShaderPath(
			"../../../../CMakeProject1/assets/shaders/vert.vs"
		);

		material.setFragShaderPath(
			"../../../../CMakeProject1/assets/shaders/frag.fs"
		);

		int materialID =
			AssetManager::Instance().Register(
				std::move(material)
			);

		Material* sharedMaterial =
			AssetManager::Instance().Find<Material>(
				materialID
			);

		if (
			sharedMaterial == nullptr ||
			sharedMaterial->shader == nullptr
			)
		{
			std::cout
				<< "Failed to create material!"
				<< std::endl;

			return;
		}


		// ============================================================
		// Shared Vertices
		// ============================================================

		Vertices vertices;

		vertices.SetPreset(0);

		int verticesID =
			AssetManager::Instance().Register(
				std::move(vertices)
			);

		Vertices* sharedVertices =
			AssetManager::Instance().Find<Vertices>(
				verticesID
			);

		if (sharedVertices == nullptr)
		{
			std::cout
				<< "Failed to create vertices!"
				<< std::endl;

			return;
		}


		// ============================================================
		// Material Settings
		// ============================================================

		sharedMaterial->shader->setVec4(
			"u_Color",
			1.0f,
			1.0f,
			1.0f,
			1.0f
		);


		// ============================================================
		// Controller 1
		//
		// Single Thread Controller
		// ============================================================

		GameObject& controllerSingle =
			engine.currentScene->AddGameObject();

		controllerSingle.name =
			"SwarmController_Single";

		controllerSingle.transform->SetPosition(
			Vector3(
				-30.0f,
				0.0f,
				-40.0f
			)
		);

		ObjectSwarmController* swarmSingle =
			controllerSingle.AddComponent<
			ObjectSwarmController
			>();


		// ============================================================
		// Single Thread Controller Settings
		// ============================================================

		swarmSingle->radius = 20.0f;

		swarmSingle->moveSpeed = 1.0f;

		swarmSingle->waveStrength = 5.0f;

		swarmSingle->spacing = 0.05f;

		swarmSingle->calculationIterations = Iteration;

		swarmSingle->objects.reserve(
			OBJECT_COUNT
		);


		// ============================================================
		// Controller 2
		//
		// Parallel Controller
		// ============================================================

		GameObject& controllerParallel =
			engine.currentScene->AddGameObject();

		controllerParallel.name =
			"SwarmController_Parallel";

		controllerParallel.transform->SetPosition(
			Vector3(
				30.0f,
				0.0f,
				-40.0f
			)
		);

		ObjectSwarmControllerRemoveLock* swarmParallel =
			controllerParallel.AddComponent<
			ObjectSwarmControllerRemoveLock
			>();


		// ============================================================
		// Parallel Controller Settings
		//
		// 保持和 Single 完全相同
		// ============================================================

		swarmParallel->radius = 20.0f;

		swarmParallel->moveSpeed = 1.0f;

		swarmParallel->waveStrength = 5.0f;

		swarmParallel->spacing = 0.05f;

		swarmParallel->calculationIterations = Iteration;

		swarmParallel->objects.reserve(
			OBJECT_COUNT
		);


		// ============================================================
		// Create Single Thread Objects
		// ============================================================

		std::cout
			<< "Creating "
			<< OBJECT_COUNT
			<< " Single Thread rendered objects..."
			<< std::endl;


		for (int i = 0; i < OBJECT_COUNT; ++i)
		{
			// --------------------------------------------------------
			// GameObject
			// --------------------------------------------------------

			GameObject& obj =
				engine.currentScene->AddGameObject();

			obj.name =
				"Single_SwarmObject_" +
				std::to_string(i);


			// --------------------------------------------------------
			// Grid Position
			// --------------------------------------------------------

			int xIndex =
				i % GRID_WIDTH;

			int zIndex =
				i / GRID_WIDTH;


			float x =
				static_cast<float>(xIndex)
				* OBJECT_SPACING
				- 100.0f;

			float z =
				static_cast<float>(zIndex)
				* OBJECT_SPACING
				- 20.0f;


			obj.transform->SetPosition(
				Vector3(
					x,
					0.0f,
					z
				)
			);


			// --------------------------------------------------------
			// Scale
			// --------------------------------------------------------

			obj.transform->SetScale(
				Vector3(
					0.1f,
					0.1f,
					0.1f
				)
			);


			// --------------------------------------------------------
			// Renderer
			// --------------------------------------------------------

			Renderer* renderer =
				obj.AddComponent<Renderer>();


			// --------------------------------------------------------
			// Shared Material
			// --------------------------------------------------------

			renderer->material =
				sharedMaterial;


			// --------------------------------------------------------
			// Shared Vertex Data
			// --------------------------------------------------------

			renderer->vertexData.SetReferenceInfo(
				sharedVertices->ReferenceInfo
			);


			// --------------------------------------------------------
			// Add Handle To Single Thread Controller
			// --------------------------------------------------------

			EngineObjectHandle<GameObject> handle;

			handle.SetReferenceInfo(
				obj.ReferenceInfo
			);

			swarmSingle->objects.push_back(
				std::move(handle)
			);
		}


		// ============================================================
		// Create Parallel Objects
		// ============================================================

		std::cout
			<< "Creating "
			<< OBJECT_COUNT
			<< " Parallel rendered objects..."
			<< std::endl;


		for (int i = 0; i < OBJECT_COUNT; ++i)
		{
			// --------------------------------------------------------
			// GameObject
			// --------------------------------------------------------

			GameObject& obj =
				engine.currentScene->AddGameObject();

			obj.name =
				"Parallel_SwarmObject_" +
				std::to_string(i);


			// --------------------------------------------------------
			// Grid Position
			// --------------------------------------------------------

			int xIndex =
				i % GRID_WIDTH;

			int zIndex =
				i / GRID_WIDTH;


			float x =
				static_cast<float>(xIndex)
				* OBJECT_SPACING
				+ 100.0f;

			float z =
				static_cast<float>(zIndex)
				* OBJECT_SPACING
				- 20.0f;


			obj.transform->SetPosition(
				Vector3(
					x,
					0.0f,
					z
				)
			);


			// --------------------------------------------------------
			// Scale
			// --------------------------------------------------------

			obj.transform->SetScale(
				Vector3(
					0.1f,
					0.1f,
					0.1f
				)
			);


			// --------------------------------------------------------
			// Renderer
			// --------------------------------------------------------

			Renderer* renderer =
				obj.AddComponent<Renderer>();


			// --------------------------------------------------------
			// Shared Material
			// --------------------------------------------------------

			renderer->material =
				sharedMaterial;


			// --------------------------------------------------------
			// Shared Vertex Data
			// --------------------------------------------------------

			renderer->vertexData.SetReferenceInfo(
				sharedVertices->ReferenceInfo
			);


			// --------------------------------------------------------
			// Add Handle To Parallel Controller
			// --------------------------------------------------------

			EngineObjectHandle<GameObject> handle;

			handle.SetReferenceInfo(
				obj.ReferenceInfo
			);

			swarmParallel->objects.push_back(
				std::move(handle)
			);
		}


		// ============================================================
		// Save Shared Assets
		// ============================================================

		AssetManager::Instance().Save<Material>(
			materialID,
			"../../../../CMakeProject1/assets/"
			"MultiThreadTest.Material.AssetObject"
		);

		AssetManager::Instance().Save<Vertices>(
			verticesID,
			"../../../../CMakeProject1/assets/"
			"MultiThreadTest.vertex.AssetObject"
		);


		// ============================================================
		// Scene Serialize
		// ============================================================

		/*
		auto s =
			SceneSerializer::Serialize(
				*engine.currentScene
			);

		SceneSerializer::SaveSceneTo(
			*engine.currentScene,
			"../../../../CMakeProject1/assets/"
			"MultiThreadTestScene"
		);

		std::cout
			<< s
			<< std::endl;
		*/


		// ============================================================
		// Result
		// ============================================================

		std::cout
			<< "========================================"
			<< std::endl;

		std::cout
			<< "Finished creating multithread test scene."
			<< std::endl;


		std::cout
			<< "Single Thread Object Count: "
			<< swarmSingle->objects.size()
			<< std::endl;


		std::cout
			<< "Parallel Object Count: "
			<< swarmParallel->objects.size()
			<< std::endl;


		std::cout
			<< "Total Swarm Object Count: "
			<< (
				swarmSingle->objects.size() +
				swarmParallel->objects.size()
				)
			<< std::endl;


		std::cout
			<< "Calculation Iterations: "
			<< swarmSingle->calculationIterations
			<< std::endl;


		std::cout
			<< "========================================"
			<< std::endl;


		std::cout
			<< std::filesystem::current_path()
			<< std::endl;
	}
}