#ifndef _SP_SIMPLE_H_
#define _SP_SIMPLE_H_

#include <vector>
#include <string>
#include <memory>

#include "OSProperties.hpp"
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 

#include "MD5Model.hpp"
#include "MD5Animation.hpp"
#include "IQMModel.hpp"
#include "Camera.hpp"
#include "Asset.hpp"
#include "Geometry.hpp"
#include "Util.hpp"
#include "Logger.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include "Font.hpp"
#include "Error.hpp"
#include "System.hpp"
#include "GUI.hpp"
#include "Console.hpp"
#include "Command.hpp"
#include "ModelView.hpp"
#include "Game.hpp"

class SimpleGame :public Game {
public:
    SimpleGame()
        :pModel(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.5f, 1.0f, 0.5f)),
        iqm_view(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.2f)),
        block_model(glm::vec3(0.0f, 0.0f, 3.5f), glm::vec3(0.5f, 1.0f, 0.5f))
    {}
    virtual void initialize();
    virtual void run();

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
    void DrawGun();
    void DrawBox(float delta);
    void Display(float delta);
    void Reshape (int w, int h);

    sp::Renderer renderer;
    sp::Camera gScreenCamera;

    Handle model_program;
    Handle plane_program;
    Handle skybox_program;
    Handle player_program;

    Handle gun_model;
    sp::ModelView pModel;
    sp::ModelView iqm_view;
    sp::ModelView block_model;

    sp::VertexBuffer cube;
    sp::VertexBuffer plane;
    sp::VertexBuffer player;

    GLuint skybox_tex;
    GLuint plane_tex;
    GLuint depth_texture;
    GLuint depth_fbo;
    GLuint skybox_rotate_loc;

    sp::TextDefinition *textDef;
    sp::Console console;
    sp::SystemInfo sys_info;

    MD5Model md5_model;
    sp::IQMModel iqm_model;

    float animate = 0.0f;

    std::vector<sp::GLProgram> programs;
    std::vector<sp::ModelView> modelViews;
    std::vector<sp::VertexBuffer> vertexBuffers;
    std::vector<RenderComponent> renderables;
};

#endif
