#ifndef _SP_SIMPLE_H_
#define _SP_SIMPLE_H_

#include <vector>
#include <string>
#include <memory>

#include <GL/glew.h>                                 // for GLuint
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 
#include "Camera.hpp"                                // for Camera
#include "Console.hpp"                               // for Console
#include "Game.hpp"                                  // for Game
#include "IQMModel.hpp"                              // for IQMModel
#include "MD5Model.hpp"                              // for MD5Model
#include "ModelView.hpp"                             // for ModelView
#include "Renderer.hpp"                              // for Renderer
#include "System.hpp"                                // for SystemInfo
#include "VertexBuffer.hpp"                          // for VertexBuffer

class SimpleGame :public Game {
public:
    SimpleGame()
        :pModel(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.5f, 1.0f, 0.5f)),
        iqmView(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.2f)),
        blockModel(glm::vec3(0.0f, 0.0f, 3.5f), glm::vec3(0.5f, 1.0f, 0.5f))
    {}
    virtual void Initialize();
    virtual void Run();

private:

    typedef unsigned int Handle;

    struct RenderComponent
    {
        Handle program;
        Handle model;
        Handle buffer;
    };

    void InitializeProgram();
    void InitEntities();

    void Init();
    void RenderEntities(glm::mat4 view);

    void DrawIQM();
    void DrawMD5();
    void DrawSkyBox();
    void DrawFloor();
    void DrawPlayer();
    void DrawBox(float delta);
    void Display(float delta);
    void Reshape (int w, int h);

    float animate = 0.0f;
    sp::Renderer renderer;
    sp::Camera gScreenCamera;

    Handle modelProgram;
    Handle planeProgram;
    Handle skyboxProgram;
    Handle playerProgram;

    Handle gunModel;

    sp::ModelView pModel;
    sp::ModelView iqmView;
    sp::ModelView blockModel;

    sp::VertexBuffer cube;
    sp::VertexBuffer plane;
    sp::VertexBuffer player;

    GLuint skyboxTexture;
    GLuint planeTexture;
    GLuint depthTexture;
    GLuint depthFBO;
    GLuint skyboxRotateLoc;

    sp::TextDefinition *textDef;
    sp::Console console;
    sp::SystemInfo sysInfo;

    MD5Model md5Model;
    sp::IQMModel iqmModel;

    std::vector<sp::GLProgram> programs;
    std::vector<sp::ModelView> modelViews;
    std::vector<sp::VertexBuffer> vertexBuffers;
    std::vector<RenderComponent> renderables;
};

#endif
