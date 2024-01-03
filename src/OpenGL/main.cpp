#include <Nuc/Log.h>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <thread>
#include "Shader.h"
#include <Nuc/Memory.h>
#include <fstream>
#include <Gem/Mat4.h>
#include <Nuc/Random.h>
#include <Nuc/Timer.h>
#include "Buffer.h"
#include "OpenGLCamera.h"
#include "GPUTimer.h"

int gObjectCount = 100000;
bool gUpdate = false;

int main(int argc, char* argv[]) {
    if(argc == 3){
        gObjectCount = std::stoi(argv[1]);
        gUpdate = std::stoi(argv[2]);
    }

    LOG_INFO("Belegarbeit Experiment: OpenGL Renderer");
    LOG_DEBUG("ObjectCount: {}   Updating: {}", gObjectCount,gUpdate);
    glfwInit();

    auto window = glfwCreateWindow(800,600,"Belegarbeit Experiment - OpenGL Renderer", nullptr, nullptr);
    //   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    OpenGlCamera camera(window);
    auto perspective = Gem::Perspective(90.f,800.f,600.f,0.01f,100.0f);
    camera.setCameraPos({-30,0,0});

    auto shader = MakeRef<Shader>("../assets/opengl-shader/shader.glsl",true);
    shader->Bind();
    glfwSwapInterval(0);

    const int objectCount = gObjectCount;

    struct ObjectLayout{
        Gem::fMat4 model;
        Gem::fVec3  color;
        uint16_t  padding;
    };
    LOG_DEBUG("Size: {}", sizeof(ObjectLayout));
    int maxRange = 25;
    int minRage = -25;
    std::vector<ObjectLayout> objectBuffer;
    objectBuffer.resize(objectCount);

    Nuc::Random random;
    for(int i = 0; i< objectCount;i++){
        objectBuffer[i].model = Gem::TranslationMatrix({random.next<float>(minRage,maxRange),random.next<float>(minRage,maxRange),random.next<float>(minRage,maxRange)});
        objectBuffer[i].color = {random.next<float>(0.0,1.0),random.next<float>(0.0,1.0),random.next<float>(0.0,1.0)};
    }

    auto shaderBuffer = MakeRef<ShaderBuffer>(objectBuffer.data(),objectBuffer.size() * sizeof(ObjectLayout),GL_DYNAMIC_DRAW);


    float data[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // A 0
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // B 1
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  // C 2
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // D 3
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // E 4
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   // F 5
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   // G 6
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   // H 7

            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,  // D 8
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  // A 9
            -0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  // E 10
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  // H 11
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   // B 12
            0.5f,  0.5f, -0.5f,  1.0f, 0.0f,   // C 13
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   // G 14
            0.5f, -0.5f,  0.5f,  0.0f, 1.0f,   // F 15

            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  // A 16
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   // B 17
            0.5f, -0.5f,  0.5f,  1.0f, 1.0f,   // F 18
            -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  // E 19
            0.5f,  0.5f, -0.5f,   0.0f, 0.0f,  // C 20
            -0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  // D 21
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // H 22
            0.5f,  0.5f,  0.5f,   0.0f, 1.0f,  // G 23
    };
    uint32_t indexData[] = {
            0, 3, 2,
            2, 1, 0,
            4, 5, 6,
            6, 7 ,4,
            // left and right
            11, 8, 9,
            9, 10, 11,
            12, 13, 14,
            14, 15, 12,
            // bottom and top
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
    };

    auto vertexBuffer = MakeRef<VertexBuffer>(data, sizeof(data),GL_STATIC_DRAW);
    auto indexBuffer = MakeRef<IndexBuffer>(indexData,sizeof(indexData),GL_STATIC_DRAW);

    auto vao = MakeRef<VertexArray>();
    BufferLayout layout;
    layout.Push("position",GL_FLOAT,3);
    layout.Push("uv",GL_FLOAT,2);
    vao->AddBuffer(*vertexBuffer,layout);
    vao->Bind();

    glEnable(GL_DEPTH_TEST);
  //  glEnable(GL_CULL_FACE);
  //  glCullFace(GL_BACK);
//    glFrontFace(GL_CCW);

    Nuc::Timer timer;
    Engine::GPUTimer gpuTimer;
    double delta = 0;
    int width, height;
    while(!glfwWindowShouldClose(window)){
        timer.start();
        glfwPollEvents();


        glfwGetWindowSize(window, &width, &height);
        gpuTimer.start();
        glClearColor(0.2,0.3,1.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera.update();

       if(gUpdate){
           for(int i = 0; i< objectCount;i++){
               objectBuffer[i].model = Gem::TranslationMatrix({random.next<float>(minRage,maxRange),random.next<float>(minRage,maxRange),random.next<float>(minRage,maxRange)});
               objectBuffer[i].color = {random.next<float>(0.0,1.0),random.next<float>(0.0,1.0),random.next<float>(0.0,1.0)};
           }
           shaderBuffer->SubData(objectBuffer.data(),objectBuffer.size() * sizeof(ObjectLayout),0);
       }
        Gem::fMat4 model = perspective * camera.getViewMatrix()  ;
        shader->Bind();
       shader->SetUniformMatrix4f("camera",model);
        shaderBuffer->Bind();
        shaderBuffer->BindBufferBase(0);
        vao->Bind();
        indexBuffer->Bind();

        glDrawElementsInstanced(GL_TRIANGLES,sizeof(indexData) / sizeof(uint32_t),GL_UNSIGNED_INT, nullptr, objectCount);
       // glDrawElementsInstanced(GL_TRIANGLES,sizeof(indexData) / sizeof(uint32_t),GL_UNSIGNED_INT, nullptr, objectCount);

        //std::this_thread::sleep_for(std::chrono::milliseconds(100));

        glfwSwapBuffers(window);
        gpuTimer.stop();

        timer.stop();
        delta = timer.time<std::chrono::microseconds>()  / 1000.0;
        auto gpuDelta = gpuTimer.time<std::chrono::microseconds>() / 1000.0;;
        std::string title = fmt::format("CPU Time: {:.2f}ms  |  GPU Time: {:.2f}ms",delta,gpuDelta);

        glfwSetWindowTitle(window, title.c_str());
        timer.reset();
        gpuTimer.reset();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
