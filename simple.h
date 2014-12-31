#ifndef _SP_SIMPLE_H_
#define _SP_SIMPLE_H_

#include <vector>
#include <string>
#include <memory>

#include "os_properties.h"
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 

#include "md5_model.h"
#include "md5_animation.h"
#include "iqm_model.h"
#include "camera.h"
#include "asset.h"
#include "geometry.h"
#include "util.h"
#include "logger.h"
#include "renderer.h"
#include "shader.h"
#include "buffer.h"
#include "font.h"
#include "error.h"
#include "system.h"
#include "gui.h"
#include "console.h"
#include "command.h"
#include "model_view.h"

#include "game.h"

class SimpleGame :public Game {
public:
    SimpleGame()
        :pModel(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.5f, 1.0f, 0.5f)),
        gun_model(glm::vec3(0.1f, -0.08f, -0.19f), glm::vec3(0.02f, 0.02f, 0.09f)),
        iqm_view(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.2f)),
        block_model(glm::vec3(0.0f, 0.0f, 3.5f), glm::vec3(0.5f, 1.0f, 0.5f))
    {}
    virtual void initialize();
    virtual void run();

private:

    typedef unsigned int Key;

    struct Renderable
    {
        Key program;
        Key model;
        Key buffer;
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

    Key model_program;
    Key plane_program;
    Key skybox_program;
    Key player_program;

    sp::ModelView pModel;
    sp::ModelView gun_model;
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
    std::vector<sp::ModelView> models;
    std::vector<sp::VertexBuffer> vertexBuffers;
    std::vector<Renderable> renderables;
};

#endif
