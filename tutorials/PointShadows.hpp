/*Copyright reserved by KenLee@2017 ken4000kl@gmail.com*/
#ifndef POINT_SHADOWS_HPP
#define POINT_SHADOWS_HPP
namespace PointShadows{

// 前置顶点声明
extern GLfloat cubeVertices[6*6*8];
extern GLfloat screenVertices[6*5];
// 光源位置
glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);
// ShadowMap的解析度
const GLuint SHADOW_MAP_WIDTH = 512;
const GLuint SHADOW_MAP_HEIGHT = 512;

void tutorial(){
    GLFWwindow *window = initWindow("ShadowMapping", 800, 600);
    glfwSwapInterval(0);
    showEnviroment();

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    CameraController::bindControl(window);

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    Camera *cam = &CameraController::camera;
    CoordinateAxes ca(cam);
    ca.showGrid(false);
    // 着色器
    Shader objectShader("shaders/PointShadows/object.vs", "shaders/PointShadows/object.frag");
    Shader shadowShader("shaders/PointShadows/shadow.vs", "shaders/PointShadows/shadow.frag");
    shadowShader.addOptionalShader("shaders/PointShadows/shadow.geom", GL_GEOMETRY_SHADER);
    Shader objectWithShadow("shaders/PointShadows/object_shadowed.vs", "shaders/PointShadows/object_shadowed.frag");
    // 物体
    Object *cubeMother = new Object(cubeVertices, 36, POSITIONS_NORMALS_TEXTURES, GL_TRIANGLES);
    cubeMother->setCamera(cam);
    cubeMother->setShader(&objectShader);
    Object* cubes[6];
    for(int i = 0; i < 6; ++i){
        cubes[i] = cubeMother->clone();
    }
    cubes[0]->model = glm::translate(cubes[0]->model, glm::vec3(4.0f, -3.5f, 0.0f));
    cubes[1]->model = glm::translate(cubes[1]->model, glm::vec3(2.0f, 3.0f, 1.0f));
    cubes[1]->model = glm::scale(cubes[1]->model, glm::vec3(1.5f));
    cubes[2]->model = glm::translate(cubes[2]->model, glm::vec3(-3.0f, -1.0f, 0.0f));
    cubes[3]->model = glm::translate(cubes[3]->model, glm::vec3(-1.5f, 1.0f, 1.5f));
    cubes[4]->model = glm::translate(cubes[4]->model, glm::vec3(-1.0f, 2.0f, -3.0f));
    cubes[4]->model = glm::rotate(cubes[4]->model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    cubes[4]->model = glm::scale(cubes[4]->model, glm::vec3(1.5f));
    cubes[5]->model = glm::scale(cubes[5]->model, glm::vec3(20.0f));
    // 贴图
    TextureManager *tm = TextureManager::getManager();
    if(!tm->loadTexture("textures/wood.jpg", 0, GL_BGR,GL_RGB))
        return ;
    // 生成ShadowMap纹理附件
    GLuint depthCubeMap;
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        for(int i = 0; i < 6; ++i){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // 生成ShadowMap的FBO, 绑定纹理
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 光源视角空间变换矩阵
    GLfloat aspect = (GLfloat)SHADOW_MAP_WIDTH / (GLfloat)SHADOW_MAP_HEIGHT;
    GLfloat near = 1.0f;
    GLfloat far = 25.0f;
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, near, far);
    // 因为有6个面，所以需要6个view矩阵
    glm::mat4 lightViews[6];
    lightViews[0] = glm::lookAt(lightPosition, lightPosition + glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // 朝右
    lightViews[1] = glm::lookAt(lightPosition, lightPosition + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // 朝左
    lightViews[2] = glm::lookAt(lightPosition, lightPosition + glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f)); // 朝上
    lightViews[3] = glm::lookAt(lightPosition, lightPosition + glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f)); // 朝下
    lightViews[4] = glm::lookAt(lightPosition, lightPosition + glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // 朝前
    lightViews[5] = glm::lookAt(lightPosition, lightPosition + glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // 朝后
    glm::mat4 lightSpaceMatrices[6];
    for(int i = 0; i < 6; ++i){
        lightSpaceMatrices[i] = lightProjection * lightViews[i];
    }
    //
    FPSCounter fc;
    // 主循环
    char lightSpaceMatricesUniformName[64];
    while(!glfwWindowShouldClose(window)){
        // 处理按键
        glfwPollEvents();
        CameraController::update();
        // 第一次渲染，生成全方位ShadowMap
        glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            shadowShader.use();
            glUniform1f(glGetUniformLocation(shadowShader.programID, "far_plane"), far);
            glUniform3fv(glGetUniformLocation(shadowShader.programID, "lightPosition"), 1, glm::value_ptr(lightPosition));
            for(int i = 0; i < 6; ++i){
                sprintf(lightSpaceMatricesUniformName, "lightSpaceMatrices[%d]", i);
                glUniformMatrix4fv(glGetUniformLocation(shadowShader.programID, lightSpaceMatricesUniformName), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrices[i]));
            }
            for(int i = 0; i < 6; ++i){
                cubes[i]->setShader(nullptr);
                cubes[i]->setCamera(nullptr);
                glUniformMatrix4fv(glGetUniformLocation(shadowShader.programID, "model"), 1, GL_FALSE, glm::value_ptr(cubes[i]->model));
                cubes[i]->draw();
            }
        // 第二次渲染，根据ShadowMap渲染场景及阴影
        glViewport(0, 0, 800, 600);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
            glActiveTexture(GL_TEXTURE1);
            tm->bindTexture(0);
            objectWithShadow.use();
            glUniform1f(glGetUniformLocation(objectWithShadow.programID, "far_plane"), far);
            glUniform1i(glGetUniformLocation(objectWithShadow.programID, "diffuseTexture"), 1);
            glUniform1i(glGetUniformLocation(objectWithShadow.programID, "shadowMap"), 0);
            glUniform3fv(glGetUniformLocation(objectWithShadow.programID, "lightPosition"), 1, glm::value_ptr(lightPosition));
            glUniform3fv(glGetUniformLocation(objectWithShadow.programID, "viewPosition"), 1, glm::value_ptr(cam->cameraPos));
            glUniform1i(glGetUniformLocation(objectWithShadow.programID, "invertNormal"), 0);
            for(int i = 0; i < 5; ++i){
                cubes[i]->setShader(&objectWithShadow);
                cubes[i]->setCamera(cam);
                cubes[i]->draw();
            }
            glUniform1i(glGetUniformLocation(objectWithShadow.programID, "invertNormal"), 1);
            cubes[5]->setShader(&objectWithShadow);
            cubes[5]->setCamera(cam);
            cubes[5]->draw();
            ca.draw();
        glfwSwapBuffers(window);
        fc.update();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

GLfloat cubeVertices[6*6*8] = {
    // Back face
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
    // Front face
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
    // Left face
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
    -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
    // Right face
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
    0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
    // Bottom face
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
    0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
    -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
    // Top face
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
};
};
#endif // POINT_SHADOWS_HPP
