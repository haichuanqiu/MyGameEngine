#pragma once
class EditorApplication
{
public:
     EditorApplication(int SceneframeBufferID):EditorCamreaGameObject()
     {
          m_Engine = std::make_unique<Engine>();

          



         
          EditorCamreaGameObject.name = "Camrea";
          EditorCamreaGameObject.transform->SetPosition(Vector3(0, 0, 5));

           EditorCamera = EditorCamreaGameObject.AddComponent<Camera>();
           EditorCamera->renderInfo_targetFramebuffer = SceneframeBufferID;
          
     }
     std::unique_ptr<Engine> m_Engine;

     void UpdateSceneView() {
          m_Engine->renderSystem.RenderForCamera(*EditorCamera);
     }
     GameObject EditorCamreaGameObject;
     GameObject* ChoosenGameObject;
     Camera* EditorCamera;
     WindowLayoutController* WindowLayout;
     int SceenChoosenItemHierarchyIndex;
private:
     
};