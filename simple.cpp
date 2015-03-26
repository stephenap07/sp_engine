#include "Simple.hpp"

void SimpleGame::Initialize()
{
    renderer.Init();
    Init();
}

void SimpleGame::Run()
{
    SDL_Event sdl_event;
    const Uint8 *state = nullptr;

    unsigned long elapsed = SDL_GetTicks();
    float delta = 0.0f;

    float player_speed = 10.0f;

    bool hide_mouse = false;
    bool quit = false;
    bool jumping = false;

    const float kGravity = 0.7f;
    float player_vel_y = 0.0f;

	sp::CommandManager::AddCommand("exit",
			[&quit](const sp::CommandArg &args) {
				quit = true;
			});
    sp::CommandManager::AddCommand("fov",
            [&](const sp::CommandArg &args) {
				renderer.SetAngleOfView(args.GetAs<float>(1));
			});

    float gunRotTime = 0.0f;
    bool leftMouseButtonDown = false;

    while (!quit) {
        delta = (SDL_GetTicks() - elapsed) / 1000.0f;
        elapsed = SDL_GetTicks();
        animate += 10.0f * delta;

        SDL_StartTextInput();
        while(SDL_PollEvent(&sdl_event)) {
            switch(sdl_event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    Reshape(sdl_event.window.data1, sdl_event.window.data2);
                    break;
            }

            switch(sdl_event.type) {
                case SDL_MOUSEMOTION:
                    gScreenCamera.HandleMouse(sdl_event.motion.xrel,
                            sdl_event.motion.yrel, delta);
                    pModel.rot = glm::angleAxis(-10.0f *
                            sdl_event.motion.xrel * delta,
                            glm::vec3(0.0f, 1.0f, 0.0f)) * pModel.rot;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    // rotate the gun when the player presses the left mouse button
                    if (sdl_event.button.button == SDL_BUTTON_LEFT) {
                        leftMouseButtonDown = true;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (sdl_event.button.button == SDL_BUTTON_LEFT) {
                        leftMouseButtonDown = false;
                    }
                case SDL_KEYUP:
                    if (sdl_event.key.keysym.sym == SDLK_k) {
                        hide_mouse = !hide_mouse;
                        SDL_SetRelativeMouseMode(hide_mouse ? SDL_TRUE : SDL_FALSE); // hide mouse
                    }
                    if (sdl_event.key.keysym.sym == SDLK_BACKQUOTE) {
                        if (console.FrameIsOpen()) {
                            console.CloseFrame();
                        } else {
                            console.OpenFrame();
                        }
                    }
                    if (sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = true;
                    }
                    break;
                case SDL_QUIT:
                    quit = true;
                    break;
            }

            if (console.FrameIsOpen()) {
                console.HandleEvent(sdl_event);
            }
        }
        SDL_StopTextInput();

        console.Update(delta);

        if (leftMouseButtonDown) {
            gunRotTime += delta;
            modelViews[gunModel].rot = glm::angleAxis(500.0f*gunRotTime, glm::vec3(0, 0, 1));
        }

        player_vel_y -= kGravity * delta;
        pModel.origin.y += player_vel_y;

        if (pModel.origin.y < 0.0f) {
            pModel.origin.y = 0.0f;
            player_vel_y = 0.0f;
            jumping = false;
        } else {
            jumping = true;
        }

        // Player Controller
        glm::vec3 side = glm::normalize(glm::cross(gScreenCamera.dir, gScreenCamera.up));
        glm::vec3 forward = glm::normalize(glm::cross(gScreenCamera.up, side));

        if (!console.FrameIsOpen()) {
            state = SDL_GetKeyboardState(nullptr);

            if (state[SDL_SCANCODE_W] && state[SDL_SCANCODE_LGUI]) quit = true;

            if (state[SDL_SCANCODE_W]) pModel.origin += delta * player_speed * forward;
            if (state[SDL_SCANCODE_S]) pModel.origin -= delta * player_speed * forward;

            if (state[SDL_SCANCODE_D]) pModel.origin += delta * player_speed * side;
            if (state[SDL_SCANCODE_A]) pModel.origin -= delta * player_speed * side;

            if (state[SDL_SCANCODE_SPACE] && !jumping) player_vel_y = 0.2f;

            if (state[SDL_SCANCODE_UP]) iqmView.origin += delta * player_speed * glm::vec3(0, 0, -1);
            if (state[SDL_SCANCODE_DOWN]) iqmView.origin -= delta * player_speed * glm::vec3(0, 0, -1);

            if (state[SDL_SCANCODE_RIGHT]) iqmView.origin += delta * player_speed * glm::vec3(1, 0, 0);
            if (state[SDL_SCANCODE_LEFT]) iqmView.origin -= delta * player_speed * glm::vec3(1, 0, 0);

            //gScreenCamera.FreeRoam(delta);
            gScreenCamera.pos = pModel.origin + glm::vec3(0.0f, 0.8f, 0.0f);
        }

        md5Model.Update(delta);
        Display(delta);
    }
}

void SimpleGame::InitializeProgram()
{
    programs.push_back(sp::backend::CreateProgram({
            {"assets/shaders/basic_animated.vert", GL_VERTEX_SHADER},
            {"assets/shaders/gouroud.frag", GL_FRAGMENT_SHADER}
            }));
    modelProgram = 0;

    programs.push_back(sp::backend::CreateProgram({
            {"assets/shaders/basic_texture.vs", GL_VERTEX_SHADER},
            {"assets/shaders/gouroud.frag", GL_FRAGMENT_SHADER}
            }));
    planeProgram = 1;

    programs.push_back(sp::backend::CreateProgram({
            {"assets/shaders/skybox.vert", GL_VERTEX_SHADER},
            {"assets/shaders/skybox.frag", GL_FRAGMENT_SHADER}
            }));
    skyboxProgram = 2;

    programs.push_back(sp::backend::CreateProgram({
            {"assets/shaders/pass_through.vert", GL_VERTEX_SHADER},
            {"assets/shaders/gouroud.frag", GL_FRAGMENT_SHADER}
            }));
    playerProgram = 3;

    for (auto & program : programs) {
        renderer.LoadGlobalUniforms(program.id);
    }
}

inline void SimpleGame::InitEntities()
{
    modelViews.push_back(sp::ModelView(glm::vec3(0.1f, -0.08f, -0.19f), glm::vec3(0.02f, 0.02f, 0.09f))); 
    gunModel = 0;
    vertexBuffers.push_back(player);
    renderables.push_back({playerProgram, 0, 0});
}

void SimpleGame::Init()
{
    gScreenCamera.Init(
        glm::mat3(
            glm::vec3(0.5f, 0.5f, 2.5f),
            glm::vec3(0.0f, -0.25f, -1.0f),
            glm::vec3(0.0f, 1.0f, 0.0f))
    );

    // OGL initial states
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    sysInfo.QuerySystemInformation();

    InitializeProgram();

    iqmView.rot = glm::angleAxis(90.0f, glm::vec3(0, 1, 0));

    glm::mat4 model;
    sp::backend::SetUniform(programs[modelProgram], sp::kMatrix4fv, "model_matrix", glm::value_ptr(model));
    iqmModel.LoadModel("assets/models/mrfixit/mrfixit.iqm");

    sp::MakeTexturedQuad(&plane);
    sp::MakeCube(&cube, false);

    skyboxTexture = sp::MakeTexture("assets/textures/skybox_texture.jpg", GL_TEXTURE_CUBE_MAP);
    glm::mat4 rotate_matrix = glm::scale(glm::mat4(), glm::vec3(300.0f));
    sp::backend::SetUniform(programs[skyboxProgram], sp::kMatrix4fv, "rotate_matrix", glm::value_ptr(rotate_matrix));

    planeTexture = sp::MakeTexture("assets/textures/checker.tga", GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, planeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    sp::MakeCube(&player, true);
    glm::vec4 player_color(0.0f, 1.0f, 1.0f, 1.0f);
    //playerProgram->SetUniform(sp::k4fv, "color_diffuse", glm::value_ptr(player_color));

    sp::backend::SetUniform(programs[modelProgram], sp::k1i, "is_textured", true);
    sp::backend::SetUniform(programs[modelProgram], sp::k1i, "is_rigged", true);
    sp::backend::SetUniform(programs[planeProgram], sp::k1i, "is_textured", true);

    console.Init((float)renderer.GetWidth(), (float)renderer.GetHeight());
    sp::font::Init((float)renderer.GetWidth(), (float)renderer.GetHeight());
    textDef = sp::font::GetTextDef("SPFont.ttf");

    InitEntities();
}

inline void SimpleGame::DrawIQM()
{
    sp::backend::Bind(programs[modelProgram]);

    glm::mat4 transform = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 0, 1)
    );

    glm::mat4 model = iqmView.GetModel() * transform;
    sp::backend::SetUniform(programs[modelProgram], sp::kMatrix4fv, "model_matrix", glm::value_ptr(model));

    iqmModel.Animate(animate);

    std::vector<glm::mat4> &bones = iqmModel.GetBones();
    sp::backend::SetUniform(programs[modelProgram], sp::kMatrix4fv, "bone_matrices",
                             (GLsizei)bones.size(),
                             glm::value_ptr(bones[0]));
    iqmModel.Render();
}

inline void SimpleGame::DrawMD5()
{
    sp::backend::Bind(programs[modelProgram]);

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.029f));
    glm::mat4 transform = glm::mat4(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 0, -1, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 0, 1)
    );
    model = glm::rotate(model, -55.0f, glm::vec3(0, 1, 0)) * transform;
    model = glm::translate(model, glm::vec3(0.0f, -32.0f, 0.0f));
    sp::backend::SetUniform(programs[modelProgram], sp::kMatrix4fv, "model_matrix", glm::value_ptr(model));

    glDisable(GL_CULL_FACE);
    md5Model.Render();
    glEnable(GL_CULL_FACE);

    glUseProgram(0);
}

inline void SimpleGame::DrawSkyBox()
{
    sp::backend::Bind(programs[skyboxProgram]);

    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

    glBindVertexArray(cube.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ebo);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glDrawElements(GL_TRIANGLE_STRIP, 17, GL_UNSIGNED_SHORT, NULL);
    glDisable(GL_PRIMITIVE_RESTART);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glEnable(GL_CULL_FACE);

    glUseProgram(0);
}

inline void SimpleGame::DrawFloor()
{
    sp::backend::Bind(programs[planeProgram]);

    glm::mat4 plane_model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.0f, 0));
    plane_model = glm::scale(plane_model, glm::vec3(10.0f, 1.0f, 10.0f));
    plane_model = glm::rotate(plane_model, -90.0f, glm::vec3(1, 0, 0));
    sp::backend::SetUniform(programs[planeProgram], sp::kMatrix4fv, "model_matrix", glm::value_ptr(plane_model));

    glBindVertexArray(plane.vao);
    glBindBuffer(GL_ARRAY_BUFFER, plane.vbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planeTexture);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

inline void SimpleGame::DrawPlayer()
{
    /// SCALE
    /// 1.0 - Player Height
    /// 0.5 - Player Width/Depth

    sp::backend::Bind(programs[playerProgram]);
    glm::mat4 player_model = pModel.GetModel();
    sp::backend::SetUniform(programs[playerProgram], sp::kMatrix4fv, "model_matrix", glm::value_ptr(player_model));

    glBindVertexArray(player.vao);
    glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player.ebo);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glDrawElements(GL_TRIANGLE_STRIP, 17, GL_UNSIGNED_SHORT, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

inline void SimpleGame::RenderEntities(glm::mat4 view)
{
    for (auto &renderable : renderables) {
        sp::backend::Bind(programs[renderable.program]);
        glm::mat4 g_model = glm::inverse(view) * modelViews[renderable.model].GetModel();
        glm::mat4 gw_model = gScreenCamera.LookAt() * g_model;

        sp::backend::SetUniform(programs[renderable.program], sp::kMatrix4fv, "model_matrix", glm::value_ptr(g_model));
        sp::backend::SetUniform(programs[renderable.program], sp::kMatrix4fv, "mv_matrix", glm::value_ptr(gw_model));

        // Assumptions of render method
        vertexBuffers[renderable.buffer].Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

inline void SimpleGame::DrawBox(float delta)
{
    sp::backend::Bind(programs[playerProgram]);
    blockModel.rot = blockModel.rot * glm::angleAxis(delta * 180.0f, glm::vec3(0, 1, 0));

    glm::mat4 rot_mat = glm::mat4_cast(blockModel.rot);
    glm::mat4 trans_mat = glm::translate(blockModel.origin);
    glm::mat4 scale_mat = glm::scale(blockModel.scale);

    glm::mat4 model = rot_mat * trans_mat * scale_mat;

    glm::mat4 b_model = model;
    glm::mat4 bv_model = gScreenCamera.LookAt() * model;

    sp::backend::SetUniform(programs[playerProgram], sp::kMatrix4fv, "model_matrix", glm::value_ptr(b_model));
    sp::backend::SetUniform(programs[playerProgram], sp::kMatrix4fv, "mv_matrix", glm::value_ptr(bv_model));

    glBindVertexArray(player.vao);
    glBindBuffer(GL_ARRAY_BUFFER, player.vbo);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void SimpleGame::Display(float delta)
{
    //static float ang = 0.0f;
    renderer.BeginFrame();
    //ang += 30.0f * delta;
    //gScreenCamera.Rotate(2.0f * sin(ang), glm::vec3(0.0f, 0.0f, 1.0f));
    renderer.SetView(gScreenCamera.LookAt());

    DrawSkyBox();
    //DrawPlayer();
    DrawBox(delta);
    DrawFloor();
    //DrawMD5();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawIQM();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    RenderEntities(gScreenCamera.LookAt());

    glDisable(GL_DEPTH_TEST);

    textDef->DrawText(std::string("FPS: ") + std::to_string((int)std::ceil((1 / delta))), 8, 35);
    //textDef->DrawText(std::string("Platform: ") + sysInfo.platform, 8, 50);
    //textDef->DrawText(std::string("CPU Count: ") + std::to_string(sysInfo.num_cpus), 8, 65);
    //textDef->DrawText(std::string("System Ram: ") + std::to_string(sysInfo.ram) + std::string("mb"), 8, 80);
    //textDef->DrawText(std::string("L1 cache: ") + std::to_string(sysInfo.l1_cache) + std::string("kb"), 8, 95);
    //textDef->DrawText(std::string("Vendor: ") + (char*)sysInfo.vendor, 8, 110);
    //textDef->DrawText(std::string("Renderer: ") + (char*)sysInfo.renderer, 8, 125);
    //textDef->DrawText(std::string("GL Version: ") + (char*)sysInfo.version, 8, 140);

    console.Draw();
    glEnable(GL_DEPTH_TEST);
    renderer.EndFrame();
}

void SimpleGame::Reshape (int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}
