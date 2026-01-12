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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
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

    stbi_set_flip_vertically_on_load(false);

    // Read image and bind to texture1
    // --------------------------------------------
    int width, height, nrChannels;
    unsigned char *data = stbi_load("./assets/jangandikasihtau.jpg", &width, &height, &nrChannels, 0);
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
    data = stbi_load("./assets/tertawatapiokegas.jpg", &width, &height, &nrChannels, 0);
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
        // Input processing
        // ---------------------------
        processInput(window);

        // Enable zBuffer
        // ---------------------------
        glEnable(GL_DEPTH_TEST);

        // Reset pixel
        // ---------------------------
        glClearColor(
            glm::sin(glm::radians(90.0f)*(float)glfwGetTime()*5.0f),
            glm::sin(glm::radians(90.0f)*(float)glfwGetTime()*4.0f),
            glm::sin(glm::radians(90.0f)*(float)glfwGetTime()*7.0f),
            1.0f
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ShaderUse(s);

        // Bind & activate texture
        // ---------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // View matrix
        glm::mat4 view(1.0f);
        view = glm::translate(
            view,
            glm::vec3(0.0f, 0.0f, -3.5)
        );
        view = glm::rotate(view, (float)glm::radians(-60.0f), glm::vec3(1.0, 0.0, 0.0));
        ShaderSetTransformation(s, "view", glm::value_ptr(view));

        // Perspective
        float aspectRatio = (float)WIDTH/(float)HEIGHT;
        glm::mat4 perspective = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100.0f);
        ShaderSetTransformation(s, "perspective", glm::value_ptr(perspective));

        // Render
        // ---------------------------
        {
            // Matrices
            // ---------------------------

            // Model matrix
            glm::mat4 model(1.0f);
            float angle = 90.0f;
            model = glm::translate(
                model,
                glm::vec3(0.0f, glm::sin(glm::radians(90.0f) * (float)glfwGetTime() * 1.5f), 0.0f)
            );
            model = glm::translate(
                model,
                glm::vec3(-1.5f, 1.3f, 0.0f)
            );
            model = glm::rotate(model, (float)glfwGetTime()*1.5f*glm::radians(angle), glm::vec3(0.0, -0.69f, 1.0));
            ShaderSetTransformation(s, "model", glm::value_ptr(model));


            // Draw
            // ---------------------------
            glBindVertexArray(VAO);
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glDrawArrays(GL_TRIANGLES, 0, 36);

        }

        // Bind & activate texture
        // ---------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        {
            // Model matrix
            glm::mat4 model(1.0f);
            float angle = 90.0f;
            model = glm::translate(
                model,
                glm::vec3(glm::sin(glm::radians(60.0f) * (float)glfwGetTime()), glm::cos(glm::radians(60.0f) * (float)glfwGetTime()), 0.0f)
            );
            model = glm::translate(
                model,
                glm::vec3(0.5f, -1.0f, 0.0f)
            );
            model = glm::rotate(model, (float)glfwGetTime()*1.5f*glm::radians(angle), glm::vec3(-0.3, 0.0, -1.0));
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
