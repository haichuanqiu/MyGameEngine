#pragma once

class EditorApplication
{
public:

     EditorApplication(int SceneframeBufferID)
          : EditorCamreaGameObject()
     {
          m_Engine =
               &Engine::Instance();


          EditorCamreaGameObject.name =
               "Camrea";


          EditorCamreaGameObject.transform
               ->SetPosition(
                    Vector3(0, 0, 5)
               );


          EditorCamera =
               EditorCamreaGameObject
               .AddComponent<Camera>();


          EditorCamera
               ->renderInfo_targetFramebuffer =
               SceneframeBufferID;
     }


     void UpdateSceneView()
     {
          m_Engine
               ->renderSystem
               .RenderForCamera(
                    *EditorCamera
               );
     }


public:

     Engine* m_Engine =
          nullptr;


     GameObject EditorCamreaGameObject;

     GameObject* ChoosenGameObject =
          nullptr;

     Camera* EditorCamera =
          nullptr;

     WindowLayoutController* WindowLayout =
          nullptr;

     int SceenChoosenItemHierarchyIndex =
          -1;
};