#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WIDTH 800
#define HEIGHT 600

// Camera
// ---------------------
glm::vec3 cameraPosition(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float cameraYaw = -90.0f;
float cameraPitch = 0.0f;
float cameraFOV = 45.0f;

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

float prevTime = (float)glfwGetTime();
float deltaTime = 0.0f;

float lastX = WIDTH/2;
float lastY = HEIGHT/2;
bool firstRender = true;

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    if (firstRender) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstRender = false;
    }

    float xOffset = (float)xpos - lastX;
    float yOffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    const float sensitivity = 0.1f;
    cameraYaw += xOffset * sensitivity;
    cameraPitch += yOffset * sensitivity;

    if (cameraPitch > 89.0f) {
        cameraPitch = 89.0f;
    }
    if (cameraPitch < -89.0f) {
        cameraPitch = -89.0f;
    }

    glm::vec3 direction;
    direction.x = glm::cos(glm::radians(cameraPitch)) * glm::cos(glm::radians(cameraYaw));
    direction.y = glm::sin(glm::radians(cameraPitch));
    direction.z = glm::cos(glm::radians(cameraPitch)) * glm::sin(glm::radians(cameraYaw));
    cameraFront = glm::normalize(direction);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraFOV -= (float)yoffset;
    if (cameraFOV <= 1.0f)
        cameraFOV = 1.0f;
    if (cameraFOV > 60.0f)
        cameraFOV = 60.0f;
}

struct Shader {
    uint32_t ID;
};

void ShaderInit(Shader &s, const char *vertexPath, const char *fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream;
        std::stringstream fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch(std::ifstream::failure &e) {
        std::cerr << "Cannot create shader because : " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    uint32_t vertexShader{};
    uint32_t fragmentShader{};

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    glCompileShader(vertexShader);

    int32_t success{};
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        const size_t msgLen = 512;
        char msg[msgLen];
        glGetShaderInfoLog(vertexShader, msgLen, nullptr, msg);
        std::cerr << "Cannot compile the vertex shader with message : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        const size_t msgLen = 512;
        char msg[msgLen];
        glGetShaderInfoLog(fragmentShader, msgLen, nullptr, msg);
        std::cerr << "Cannot compile the fragment shader with message : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    uint32_t shaderProgram{};
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        const size_t msgLen = 512;
        char msg[msgLen];
        glGetProgramInfoLog(shaderProgram, msgLen, nullptr, msg);
        std::cerr << "Cannot link the shader program with message : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    s.ID = shaderProgram;
}

void ShaderUse(const Shader &s){
    glUseProgram(s.ID);
}

void ShaderSetFloat(const Shader &s, const char *name, float value) {
    int vertexColorLocation = glGetUniformLocation(s.ID, name);
    assert(vertexColorLocation != -1);
    glUniform1f(vertexColorLocation, value);
}

void ShaderSetInt(const Shader &s, const char *name, int value) {
    int vertexColorLocation = glGetUniformLocation(s.ID, name);
    assert(vertexColorLocation != -1);
    glUniform1i(vertexColorLocation, value);
}

void ShaderSetTransformation(const Shader &s, const char *name, const GLfloat* value) {
    int transformLocation = glGetUniformLocation(s.ID, name);
    assert(transformLocation != -1);
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, value);
}

void ShaderSetBool(const Shader &s, const char *name, bool value) {
    int vertexColorLocation = glGetUniformLocation(s.ID, name);
    assert(vertexColorLocation != -1);
    glUniform1i(vertexColorLocation, value);
}

void frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}


void processInput(GLFWwindow *window) {
    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {  // Front
        cameraPosition += cameraSpeed * cameraFront ;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { // Back
        cameraPosition -= cameraSpeed * cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { // Left
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { // Right
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

void renderElement(const Shader &s, uint32_t VAO, glm::mat4 trans) {
    // Use shader
    // ---------------------------
    ShaderUse(s);

    // Setting up transform uniform
    // ---------------------------
    ShaderSetTransformation(s, "transform", glm::value_ptr(trans));

    // Draw
    // ---------------------------
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main()
{
    // Initialize OpenGL
    // ---------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window Creation
    // ---------------------------
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Hello from OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create a window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

    // Registering mouse callbacks
    // ---------------------------
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, scrollCallback);

    // Load OpenGL functions
    // ---------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initalize GLAD" << std::endl;
        return -1;
    }

    // Build and compile shader
    // ---------------------------
    Shader s{};
    const char *vPath = "./vertex_shader.glsl";
    const char *fPath = "./fragment_shader.glsl";
    ShaderInit(s, vPath, fPath);

    // Setup vertex data
    // ---------------------------
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    // Setup indices
    // ---------------------------
    uint32_t indices[] = {  // note that we start from 0!
        0, 1, 3,
        1, 2, 3,
    };

    // Initialization
    // ---------------------------
    uint32_t VAO, VBO;
    // uint32_t EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);

    // 1) Bind vertex array object first
    glBindVertexArray(VAO);

    // 2) Bind vertex buffer object with vertex
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3) Bind element buffer object with indices
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 4) Set the vertex attribute
    // layout (location = 0) in vec3 aPos;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // layout (location = 1) in vec2 aTexPos;
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid *)(3*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(1);

    // Setup textures
    // ---------------------------
    uint32_t texture1 ,texture2;
    glGenTextures(1, &texture1);
    glGenTextures(1, &texture2);

    // Bind texture1 with params
    // ---------------------------
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    // Read image and bind to texture1
    // --------------------------------------------
    int width, height, nrChannels;
    unsigned char *data = stbi_load("./assets/container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D,
           	0,
           	GL_RGB,
           	width,
           	height,
           	0,
           	GL_RGB,
           	GL_UNSIGNED_BYTE,
           	data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load image texture1" << std::endl;
        exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

    // Bind texture2 with params
    // ---------------------------
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Read image and bind to texture2
    // --------------------------------------------
    data = stbi_load("./assets/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D,
           	0,
           	GL_RGBA,
           	width,
           	height,
           	0,
           	GL_RGBA,
           	GL_UNSIGNED_BYTE,
           	data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load image texture2" << std::endl;
        exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

    // Setting up texture uniforms
    // ---------------------------
    ShaderUse(s);

    // uniform sampler2D ourTexture1;
    ShaderSetInt(s, "ourTexture1", 0);

    // uniform sampler2D ourTexture2;
    ShaderSetInt(s, "ourTexture2", 1);

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // ---------------------------
        float currentTime = (float)glfwGetTime();
        deltaTime = currentTime - prevTime;
        prevTime = currentTime;

        // Input processing
        // ---------------------------
        processInput(window);

        // Enable zBuffer
        // ---------------------------
        glEnable(GL_DEPTH_TEST);

        // Reset pixel
        // ---------------------------
        glClearColor(0.53f, 0.53f, 0.53f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ShaderUse(s);

        // Bind & activate texture
        // ---------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // Matrices
        // ---------------------------
        // View matrix
        auto view = glm::lookAt(
            cameraPosition,
            cameraPosition + cameraFront,
            cameraUp
        );
        ShaderSetTransformation(s, "view", glm::value_ptr(view));

        // Perspective
        glm::mat4 perspective = glm::perspective(
            glm::radians(cameraFOV),
            (float)WIDTH/(float)HEIGHT,
            0.1f,
            100.0f
        );
        ShaderSetTransformation(s, "perspective", glm::value_ptr(perspective));

        for (int i = 0; i < 10; ++i) {
            // Model matrix
            glm::mat4 model(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(90.0f)*i, glm::vec3(0.0, -0.69f, 1.0));
            ShaderSetTransformation(s, "model", glm::value_ptr(model));

            // Draw
            // ---------------------------
            glBindVertexArray(VAO);
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        // Swap buffer and poll IO events
        // ---------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
