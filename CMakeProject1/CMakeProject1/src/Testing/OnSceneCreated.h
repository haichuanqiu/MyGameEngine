#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <filesystem>
#include "Engine/Engine.h"
#include "Engine/Components/ReferenceTester.h"
#include "Engine/Scene.h"
#include "rendering/VertexDataController.h"
#include "rendering/ShaderManager.h"
#include "Editor/EditorApplication.h"
#include "Serialization/SceneSerializer.h"
#include "Assets/Material.h"
#include "Assets/Vertices.h"
#include "Assets/AssetManager.h"

namespace testScene {

	void OnSceneCreated(
		
		Engine& engine
	)
	{
		std::cout << "StartCreate gameobjects" << std::endl;

		// --------------------------------
		// Light
		// --------------------------------

		GameObject& light =
			engine.currentScene->AddGameObject();

		light.name = "Light";

		light.transform->SetPosition(
			Vector3(2.0f, 3.0f, 2.0f)
		);
		
		PointLight* lightComponent =
			light.AddComponent<PointLight>();

		lightComponent->Intensity = 10.0f;

		lightComponent->UpdateLightData(
			engine.renderSystem
		);
		lightComponent->SetDefaultTarget(&engine.renderSystem);
		engine.renderSystem.GetLighting().UpdateAmbientLightData(
			glm::vec3(0.2f, 0.2f, 0.2f)
		);


		// --------------------------------
		// Cube
		// --------------------------------

		GameObject& cube =
			engine.currentScene->AddGameObject();

		cube.name = "Cube";

		cube.transform->SetPosition(
			Vector3(0.0f, 0.0f, 0.0f)
		);
		cube.transform->SetRotation(
			Quaternion(
				0.167731f,  // x
				0.254887f,  // y
				0.044943f,  // z
				0.951251f   // w
			)
			
		);
		Renderer* renderer = cube.AddComponent<Renderer>();
	
		Material m;
		
		m.setVertShaderPath(
			"../../../../CMakeProject1/assets/shaders/vert.vs"
		);

		m.setFragShaderPath(
			"../../../../CMakeProject1/assets/shaders/frag.fs"
		);
		int materialID =
			AssetManager::Instance().Register(std::move(m));
		Vertices v1;
		v1.SetPreset(0);
		int verticesID1 =AssetManager::Instance().Register(std::move(v1));
		Vertices v2;
		v2.SetPreset(1);
		int verticesID2 = AssetManager::Instance().Register(std::move(v2));
		Vertices v3;
		v3.SetPreset(2);
		int verticesID3 = AssetManager::Instance().Register(std::move(v3));
		renderer->material =
			AssetManager::Instance().Find<Material>(materialID);

		if (renderer->material == nullptr ||
			renderer->material->shader == nullptr)
		{
			// Material / Shader 创建失败
			return;
		}

		renderer->material->shader->setVec4(
			"u_Color",
			1.0f,
			1.0f,
			1.0f,
			1.0f
		);

		//renderer->renderSystemIndex = AssetManager::Instance().Find<Vertices>(verticesID)->RenderSystemIndex;
		
		ReferenceTest* Test = cube.AddComponent<ReferenceTest>();

		Test->material.SetReferenceInfo ( AssetManager::Instance().Find<Material>(materialID)->ReferenceInfo);

		Test->myRefGameObject.SetReferenceInfo(light.ReferenceInfo);
		
		//Test->myRefGameObject = engine.currentScene.GetObjectOfType<std::remove_reference_t<decltype(light)>>(light.ReferenceInfo.ObjectID);

		//Test->rd = engine.currentScene.GetObjectOfType<std::remove_pointer_t<decltype(renderer)>>(renderer->ReferenceInfo.ObjectID);
		Test->rd.SetReferenceInfo(renderer->ReferenceInfo);
		//Test->ts = Engine::Instance().GetScene(light.ReferenceInfo.ScopeID)->GetObjectOfType<GameObject>(light.ReferenceInfo.ObjectID)->transform;
		Test->ts.SetReferenceInfo(light.ReferenceInfo);
		Test->testInt = 123;
		Test->testFloat = 45.67f;
		Test->testBool = true;
		Test->testString = "ReferenceTest Primitive Test";

		auto s=SceneSerializer::Serialize(*engine.currentScene);
		SceneSerializer::SaveSceneTo(*engine.currentScene,"../../../../CMakeProject1/assets/SceneData");
		
		AssetManager::Instance().Save<Material>(
			materialID,
			"../../../../CMakeProject1/assets/Material1.Material.AssetObject"
		);
		AssetManager::Instance().Save<Vertices>(
			verticesID1,
			"../../../../CMakeProject1/assets/Square.vertex.AssetObject"
		);
		AssetManager::Instance().Save<Vertices>(
			verticesID2,
			"../../../../CMakeProject1/assets/Triangle.vertex.AssetObject"
		);
		AssetManager::Instance().Save<Vertices>(
			verticesID3,
			"../../../../CMakeProject1/assets/Plane.vertex.AssetObject"
		);
		std::cout << s << std::endl;

		std::cout << std::filesystem::current_path() << std::endl;
	}
	
}