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
struct Camera {
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    float yaw;
    float pitch;
    float fov;

    float speed = 2.8f;
    float sensitivity = 0.1f;
};

void CameraUpdateVector(Camera &c) {
    glm::vec3 direction;
    direction.x = glm::cos(glm::radians(c.pitch)) * glm::cos(glm::radians(c.yaw));
    direction.y = glm::sin(glm::radians(c.pitch));
    direction.z = glm::cos(glm::radians(c.pitch)) * glm::sin(glm::radians(c.yaw));

    c.front = glm::normalize(direction);
    c.right = glm::normalize(glm::cross(c.front, c.up));
}

void CameraInit(Camera &c, glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov) {
    c.position = position;
    c.up = up;
    c.yaw = yaw;
    c.pitch = pitch;
    c.fov = fov;

    CameraUpdateVector(c);
}

void CameraRotate(Camera &c, float xOffset, float yOffset) {
    c.yaw += xOffset * c.sensitivity;
    c.pitch += yOffset * c.sensitivity;

    if (c.pitch > 89.0f) {
        c.pitch = 89.0f;
    }
    if (c.pitch < -89.0f) {
        c.pitch = -89.0f;
    }

    CameraUpdateVector(c);
}

enum CameraMoveDirection {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

void CameraMove(Camera &c, CameraMoveDirection dir, float deltaTime) {
    float framespeed = c.speed * deltaTime;
    if (dir == FORWARD) {
        c.position += framespeed * c.front;
    } else if (dir == BACKWARD) {
        c.position -= framespeed * c.front;
    } else if (dir == LEFT) {
        c.position -= c.right * framespeed;
    } else if (dir == RIGHT) {
        c.position += c.right * framespeed;
    }
    // c.position.y = 0;
}

void CameraZoom(Camera &c, float yoffset) {
    c.fov += yoffset;
    if (c.fov <= 1.0f)
        c.fov = 1.0f;
    if (c.fov >= 60.0f)
        c.fov = 60.0f;
}

glm::mat4 CameraLookAt(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp) {
    glm::vec3 direction = glm::normalize(position - target);
    glm::vec3 right = glm::normalize(glm::cross(glm::normalize(worldUp), direction));
    glm::vec3 up = glm::cross(direction, right);

    // glm::mat[column][row]
    // ---------------------------
    glm::mat4 rotation(1.0f);
    rotation[0][0] = right.x;
    rotation[1][0] = right.y;
    rotation[2][0] = right.z;
    rotation[0][1] = up.x;
    rotation[1][1] = up.y;
    rotation[2][1] = up.z;
    rotation[0][2] = direction.x;
    rotation[1][2] = direction.y;
    rotation[2][2] = direction.z;

    glm::mat4 translation(1.0f);
    translation[3][0] = -position.x;
    translation[3][1] = -position.y;
    translation[3][2] = -position.z;

    return rotation * translation;
}

glm::mat4 CameraGetViewMatrix(Camera &c) {
    return CameraLookAt(
        c.position,
        c.position + c.front,
        c.up
    );
}

glm::mat4 CameraGetPerspective(Camera &c) {
    return glm::perspective(
        glm::radians(c.fov),
        (float)WIDTH/(float)HEIGHT,
        0.1f,
        100.0f
    );
}

// Shader
// -------------------------------------
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

void ShaderSetVec3(const Shader &s, const char *name, float x, float y, float z) {
    int transformLocation = glGetUniformLocation(s.ID, name);
    assert(transformLocation != -1);
    glUniform3f(transformLocation, x, y, z);
}

void ShaderSetVec3(const Shader &s, const char *name, glm::vec3 v) {
    int transformLocation = glGetUniformLocation(s.ID, name);
    assert(transformLocation != -1);
    glUniform3f(transformLocation, v.x, v.y, v.z);
}

// Globals
// --------------------------------------
Camera camera;

float prevTime = (float)glfwGetTime();
float deltaTime = 0.0f;

float lastX = WIDTH/2;
float lastY = HEIGHT/2;
bool firstRender = true;

glm::vec3 lightPosition(1.2f, 1.0f, 2.0f);

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

    CameraRotate(camera, xOffset, yOffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    CameraZoom(camera, (float)yoffset);
}

void frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {  // Front
        CameraMove(camera, FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { // Back
        CameraMove(camera, BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { // Left
        CameraMove(camera, LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { // Right
        CameraMove(camera, RIGHT, deltaTime);
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
uint32_t loadTexture(const char *path) {
    uint32_t texture1;
    glGenTextures(1, &texture1);

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
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1) {
            format = GL_RED;
        }  else if (nrChannels == 3) {
            format = GL_RGB;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
        }
        glTexImage2D(GL_TEXTURE_2D,
           	0,
           	format,
           	width,
           	height,
           	0,
           	format,
           	GL_UNSIGNED_BYTE,
           	data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load image texture1" << std::endl;
        exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

    return texture1;
}

int main()
{
// Initialize app
// ---------------------------
CameraInit(
    camera,
    glm::vec3(-1.0f, -0.5f, 4.0f), // position
    glm::vec3(0.0f, 1.0f, 0.0f), // up
    -64.0f,                      // yaw
    16.0f,                        // pitch
    45.0f                        // fov
);

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
    Shader lightingShader{};
    ShaderInit(lightingShader, "./colors_vertex.glsl",  "./colors_fragment.glsl");

    Shader lightCubeShader{};
    ShaderInit(lightCubeShader,  "./light_cube_vertex.glsl",  "./light_cube_fragment.glsl");

    // Setup vertex data
    // ---------------------------
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    // Setup indices
    // ---------------------------
    uint32_t indices[] = {  // note that we start from 0!
        0, 1, 3,
        1, 2, 3,
    };

    // Initialization
    // ---------------------------
    uint32_t cubeVAO, VBO;
    // uint32_t EBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);

    // 1) Bind vertex buffer object with vertex
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 2) Bind vertex array object first
    glBindVertexArray(cubeVAO);

    // 3) Bind element buffer object with indices
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 4) Set the vertex attribute
    // layout (location = 0) in vec3 aPos;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // layout (location = 1) in vec3 aNormal;
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid *) (3*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(1);

    // layout (location = 2) in vec2 aTexCoords;
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid *) (6*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(2);

    uint32_t lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // layout (location = 0) in vec3 aPos;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // Setup textures
    // ---------------------------
    uint32_t texture1 = loadTexture( "./assets/container2.png");

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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind & activate texture
        // ---------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, texture2);

        {
            ShaderUse(lightingShader);

            ShaderSetInt(lightingShader, "material.diffuseMap", 0);

            ShaderSetVec3(lightingShader, "cameraPosition", camera.position.x, camera.position.y, camera.position.z); // uniform vec3 cameraPosition;

            // uniform Material material;
            ShaderSetVec3(lightingShader, "material.specular", 0.628281f,	0.555802f,	0.366065f);
            ShaderSetFloat(lightingShader, "material.shininess", 32.0f);

            glm::vec3 lightColor(1.0f);
            glm::vec3 diffuse = lightColor;
            glm::vec3 ambient = diffuse * glm::vec3(0.5f);

            // uniform Light light;
            ShaderSetVec3(lightingShader, "light.position", lightPosition);
            ShaderSetVec3(lightingShader, "light.ambient", ambient);
            ShaderSetVec3(lightingShader, "light.diffuse", diffuse);
            ShaderSetVec3(lightingShader, "light.specular", glm::vec3(1.0f, 1.0f, 1.0f));

            // Model matrix
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(0.0f,  0.0f,  0.0f));
            // model = glm::rotate(model, glm::radians(60.0f * (float)glfwGetTime()), glm::vec3(1.0f, 0.0f, 0.0f));
            ShaderSetTransformation(lightingShader, "model", glm::value_ptr(model));

            // Normal matrix
            glm::mat3 normalMatrix(glm::inverse(model));
            int transformLocation = glGetUniformLocation(lightingShader.ID,  "normalMatrix");
            assert(transformLocation != -1);
            glUniformMatrix3fv(transformLocation, 1, GL_TRUE, glm::value_ptr(normalMatrix));  // uniform mat3 normalMatrix;

            // View matrix
            auto view = CameraGetViewMatrix(camera);
            ShaderSetTransformation(lightingShader, "view", glm::value_ptr(view));

            // Perspective
            auto perspective = CameraGetPerspective(camera);
            ShaderSetTransformation(lightingShader, "perspective", glm::value_ptr(perspective));

            // Draw
            // ---------------------------
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        {
            ShaderUse(lightCubeShader);

            // Model matrix
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(lightPosition));
            model = glm::scale(model, glm::vec3(0.2f));
            ShaderSetTransformation(lightCubeShader, "model", glm::value_ptr(model));

            // View matrix
            auto view = CameraGetViewMatrix(camera);
            ShaderSetTransformation(lightCubeShader, "view", glm::value_ptr(view));

            // Perspective
            auto perspective = CameraGetPerspective(camera);
            ShaderSetTransformation(lightCubeShader, "perspective", glm::value_ptr(perspective));

            // Draw
            // ---------------------------
            glBindVertexArray(lightCubeVAO);
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
