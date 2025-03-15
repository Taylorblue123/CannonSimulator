// main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "CannonSystem.h"  // Simulator
#include <iostream>
#include <vector>
#include <cmath>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// OpenGL Object
GLuint trajectoryVAO, trajectoryVBO;
GLuint projectileVAO, projectileVBO;
GLuint cubeVAO, cubeVBO;
GLuint shaderProgram;

// Camera
glm::vec3 cameraPos(0.0f, 1.0f, 3.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
float fov = 60.0f;

// Time Control
float deltaTime = 0.0f, lastFrame = 0.0f;

// ProjectileRadius
float projectileRadius = 0.2f;         // Radius of the projectile
int projectileVertexCount = 0;         
std::vector<glm::vec3> currentTrajectory;  // the trajectory of the projectile
int trajectoryIndex = 0;               // current display point on trajectory
bool trajectoryActive = false;         

//bool spaceKeyPressed = false;

//Cannon Position
glm::vec3 cannonPos(0.0f, 0.0f, -1.0f);


bool cursorEnabled = false;

// ---------------------
// Generate Projectile Vertexes Data
// ---------------------
void generateSphere(std::vector<float>& vertices, float radius, int sectors, int stacks) {
    const float PI = acos(-1.0f);
    float sectorStep = 2 * PI / sectors; //Horiental anle 360 degree
    float stackStep = PI / stacks;//Vertical angle
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }
}

// ---------------------
// Generate Cannon Data
// ---------------------
void initCube() {
    float cubeVertices[] = {
        // front face
        -0.1f, -0.1f,  0.1f,   0.1f, -0.1f,  0.1f,   0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,  -0.1f,  0.1f,  0.1f,  -0.1f, -0.1f,  0.1f,
         // back face
         -0.1f, -0.1f, -0.1f,  -0.1f,  0.1f, -0.1f,   0.1f,  0.1f, -0.1f,
          0.1f,  0.1f, -0.1f,   0.1f, -0.1f, -0.1f,  -0.1f, -0.1f, -0.1f,
          // left face
          -0.1f,  0.1f, -0.1f,  -0.1f,  0.1f,  0.1f,  -0.1f, -0.1f,  0.1f,
          -0.1f, -0.1f,  0.1f,  -0.1f, -0.1f, -0.1f,  -0.1f,  0.1f, -0.1f,
          // right face
           0.1f,  0.1f, -0.1f,   0.1f, -0.1f, -0.1f,   0.1f, -0.1f,  0.1f,
           0.1f, -0.1f,  0.1f,   0.1f,  0.1f,  0.1f,   0.1f,  0.1f, -0.1f,
           // top face
           -0.1f,  0.1f, -0.1f,   0.1f,  0.1f, -0.1f,   0.1f,  0.1f,  0.1f,
            0.1f,  0.1f,  0.1f,  -0.1f,  0.1f,  0.1f,  -0.1f,  0.1f, -0.1f,
            // bottom face
            -0.1f, -0.1f, -0.1f,  -0.1f, -0.1f,  0.1f,   0.1f, -0.1f,  0.1f,
             0.1f, -0.1f,  0.1f,   0.1f, -0.1f, -0.1f,  -0.1f, -0.1f, -0.1f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// ---------------------
// Initialize Shader and OpenGL Program
// ---------------------
void initGLObjects() {
    // Trajectory VAO/VBO��(Dynamic Update)
    glGenVertexArrays(1, &trajectoryVAO);
    glGenBuffers(1, &trajectoryVBO);
    glBindVertexArray(trajectoryVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Projectile VAO/VBO
    std::vector<float> sphereVertices;
    generateSphere(sphereVertices, projectileRadius, 54, 36);
    projectileVertexCount = sphereVertices.size() / 3;
    glGenVertexArrays(1, &projectileVAO);
    glGenBuffers(1, &projectileVBO);
    glBindVertexArray(projectileVAO);
    glBindBuffer(GL_ARRAY_BUFFER, projectileVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Cannon cube VAO/VBO
    initCube();

    // Shader Program��OpenGL 3.3 Core��
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.8, 0.5, 0.2, 1.0);
        }
    )";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

// ---------------------
// ImGui Control Menu
// ---------------------
void drawImGuiControls(float& mass, float& barrelLength, float& powder, float& azimuth, float& elevation, int& method) {
    ImGui::Begin("Cannon Parameters");
    ImGui::SliderFloat("Mass (kg)", &mass, 0.1f, 20.0f);
    ImGui::SliderFloat("Barrel Length (m)", &barrelLength, 0.1f, 5.0f);
    ImGui::SliderFloat("Powder (kg)", &powder, 0.01f, 1.0f);
    ImGui::SliderFloat("Azimuth (deg)", &azimuth, 0.0f, 360.0f);
    ImGui::SliderFloat("Elevation (deg)", &elevation, 0.0f, 90.0f);
    //Select the method of integration
    ImGui::Combo("Method", &method, "Euler\0Midpoint\0RK4\0");

    if (ImGui::Button("Fire Cannon")) {
        CannonSystem* newCannon = new CannonSystem(mass, barrelLength, powder, azimuth, elevation, method);
        newCannon->simulate(0.001f, method);
        currentTrajectory = newCannon->getTrajectory();
        trajectoryIndex = 0;
        trajectoryActive = true;
        delete newCannon;
    }
    ImGui::End();
}

// ---------------------
// Mouse Callback
// ---------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // if cursorEnabled, then no control for camera view
    if (cursorEnabled)
        return;

    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // y inverse
    lastX = (float)xpos;
    lastY = (float)ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// ---------------------
// Keyboard Callbacks
// ---------------------
void processInput(GLFWwindow* window) {
    float speed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;

    //  ESC Keyboard
    static bool EscKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (!EscKeyPressed) {
            cursorEnabled = !cursorEnabled;
            if (cursorEnabled) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true; 
            }
            EscKeyPressed = true;
        }
    }
    else {
        EscKeyPressed = false;
    }
}

// ---------------------
// Core Rendering Function�����ƴ��ڡ��켣�͵�ǰ�ڵ�λ�ã�ͬʱ���� ImGui ����
// ---------------------
void render(GLFWwindow* window) {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // MVP Matrix
    glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // Draw Cannon
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cannonPos);
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 2.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Draw trajectory frame by frame 
    if (trajectoryActive && !currentTrajectory.empty()) {
        trajectoryIndex += 1; // Update points
        if (trajectoryIndex > currentTrajectory.size())
            trajectoryIndex = currentTrajectory.size();
        // Update VBO for more points
        glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
        glBufferData(GL_ARRAY_BUFFER, trajectoryIndex * sizeof(glm::vec3), currentTrajectory.data(), GL_DYNAMIC_DRAW);
        glBindVertexArray(trajectoryVAO);
        glDrawArrays(GL_LINE_STRIP, 0, trajectoryIndex);

        // Draw the projectile at the end of trajectory
        glm::vec3 projPos = currentTrajectory[trajectoryIndex - 1];
        model = glm::mat4(1.0f);
        model = glm::translate(model, projPos);
        model = glm::scale(model, glm::vec3(projectileRadius));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(projectileVAO);
        glDrawArrays(GL_TRIANGLES, 0, projectileVertexCount);
    }

    // ImGui Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

// ---------------------
// Main Function
// ---------------------
int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW ��ʼ��ʧ��" << std::endl;
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "Ballistic Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "���ڴ���ʧ��" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    // Default Mouse Control
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD ��ʼ��ʧ��" << std::endl;
        return -1;
    }

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    initGLObjects();

    // Init CannonSystem parameters
    float cannonMass = 10.0f;
    float cannonBarrelLength = 0.5f;
    float cannonPowder = 0.05f;
    float cannonAzimuth = 60.0f;
    float cannonElevation = 60.0f;
    int method = 2;

    while (!glfwWindowShouldClose(window)) {
        // Init ImGui New Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw components
        drawImGuiControls(cannonMass, cannonBarrelLength, cannonPowder, cannonAzimuth, cannonElevation, method = 2);

        //Main Rendering
        render(window);
        glfwPollEvents();
    }

    // Clear ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
