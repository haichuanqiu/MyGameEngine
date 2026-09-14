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
#include "Assets/AssetManager.h"

namespace testScene {
	Mesh RegisterMesh() {
		float squareVertices[] = {
			// 前面 (z = +0.5)
			-0.5f, -0.5f,  0.5f,  // 0
			 0.5f, -0.5f,  0.5f,  // 1
			 0.5f,  0.5f,  0.5f,  // 2
			-0.5f,  0.5f,  0.5f,  // 3

			// 后面 (z = -0.5)
			-0.5f, -0.5f, -0.5f,  // 4
			 0.5f, -0.5f, -0.5f,  // 5
			 0.5f,  0.5f, -0.5f,  // 6
			-0.5f,  0.5f, -0.5f   // 7
		};
		unsigned int indices[] = {
			// 前面
			0, 1, 2,
			2, 3, 0,

			// 后面
			4, 6, 5,
			6, 4, 7,

			// 左面
			4, 0, 3,
			3, 7, 4,

			// 右面
			1, 5, 6,
			6, 2, 1,

			// 上面
			3, 2, 6,
			6, 7, 3,

			// 下面
			4, 5, 1,
			1, 0, 4
		};
		Mesh squareMesh;
		squareMesh.vertexData.resize(sizeof(squareVertices));

		std::memcpy(
			squareMesh.vertexData.data(),
			squareVertices,
			sizeof(squareVertices)
		);
		squareMesh.indexData.resize(sizeof(indices));
		std::memcpy(
			squareMesh.indexData.data(),
			indices,
			sizeof(indices)
		);
		squareMesh.vertexLayout.stride[0] =
			3 * sizeof(float);


		squareMesh.vertexLayout.attributes.push_back({
		    VertexSemantic::Position,
		    VertexFormat::Float3,
		    0,      // offset
		    0       // stream
			});
		squareMesh.vertexCount = 8;
		squareMesh.indexCount = 36;
		return squareMesh;
	}
	void OnSceneCreated(
		EditorApplication& editor,
		Engine& engine,
		WindowLayoutController& layout
	)
	{
		std::cout << "StartCreate gameobjects" << std::endl;

		// --------------------------------
		// Mesh
		// --------------------------------

		Mesh cubeMesh = RegisterMesh();

		int cubeMeshID =
			engine.vertexDataController.registerMesh(cubeMesh);


		// --------------------------------
		// Light
		// --------------------------------

		GameObject& light =
			engine.currentScene.AddGameObject();

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
			engine.currentScene.AddGameObject();

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

		renderer->renderSystemIndex = cubeMeshID;

		engine.renderSystem.RegisterRenderer(
			renderer
		);

		// 根据 RuntimeID 查找 Material

		Asset* asset = 
			AssetManager::Instance()
			.Find<Material>(
				materialID
			);
		if (asset)
		{
			layout.m_InspectorView.SetTarget(asset);
		}
		ReferenceTest* Test = cube.AddComponent<ReferenceTest>();

		Test->material = AssetManager::Instance().Find<Material>(materialID);

		Test->myRefGameObject = engine.currentScene.GetObjectOfType<std::remove_reference_t<decltype(light)>>(light.ReferenceInfo.ObjectID);

		Test->rd = engine.currentScene.GetObjectOfType<std::remove_pointer_t<decltype(renderer)>>(renderer->ReferenceInfo.ObjectID);

		Test->ts = Engine::Instance().GetScene(light.ReferenceInfo.ScopeID)->GetObjectOfType<GameObject>(light.ReferenceInfo.ObjectID)->transform;
		
		Test->testInt = 123;
		Test->testFloat = 45.67f;
		Test->testBool = true;
		Test->testString = "ReferenceTest Primitive Test";
		Test->testString =
			"ReferenceTest Primitive Test";

		auto s=SceneSerializer::Serialize(engine.currentScene);
		SceneSerializer::SaveSceneTo(engine.currentScene,"../../../../CMakeProject1/assets/SceneData");
		
		AssetManager::Instance().Save<Material>(
			materialID,
			"../../../../CMakeProject1/assets/Material1.Material.AssetObject"
		);
		std::cout << s << std::endl;

		std::cout << std::filesystem::current_path() << std::endl;
	}
	
}