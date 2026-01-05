#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define WIDTH 800
#define HEIGHT 600

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char *fragmentShaderSource1 = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

const char *fragmentShaderSource2 = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
"}\0";

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Hello from OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create a window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initalize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // compile shader
    // ========================================
    uint32_t vertexShader{};
    uint32_t fragmentShaderOrange{};
    uint32_t fragmentShaderYellow{};
    uint32_t shaderProgramOrange{};
    uint32_t shaderProgramYellow{};

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShaderYellow = glCreateShader(GL_FRAGMENT_SHADER);
    fragmentShaderOrange = glCreateShader(GL_FRAGMENT_SHADER);
    shaderProgramOrange = glCreateProgram();
    shaderProgramYellow = glCreateProgram();

    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glShaderSource(fragmentShaderOrange, 1, &fragmentShaderSource1, nullptr);
    glShaderSource(fragmentShaderYellow, 1, &fragmentShaderSource2, nullptr);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShaderOrange);
    glCompileShader(fragmentShaderYellow);

    int32_t success{};
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        const size_t msgLen = 512;
        char msg[msgLen];
        glGetShaderInfoLog(vertexShader, msgLen, nullptr, msg);
        std::cerr << "Cannot compile the vertex shader with message : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    glGetShaderiv(fragmentShaderOrange, GL_COMPILE_STATUS, &success);
    if (!success) {
        const size_t msgLen = 512;
        char msg[msgLen];
        glGetShaderInfoLog(fragmentShaderOrange, msgLen, nullptr, msg);
        std::cerr << "Cannot compile the fragment shader with message : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    glGetShaderiv(fragmentShaderYellow, GL_COMPILE_STATUS, &success);
    if (!success) {
        const size_t msgLen = 512;
        char msg[msgLen];
        glGetShaderInfoLog(fragmentShaderYellow, msgLen, nullptr, msg);
        std::cerr << "Cannot compile the fragment shader with message : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    glAttachShader(shaderProgramOrange, vertexShader);
    glAttachShader(shaderProgramOrange, fragmentShaderOrange);
    glLinkProgram(shaderProgramOrange);

    glAttachShader(shaderProgramYellow, vertexShader);
    glAttachShader(shaderProgramYellow, fragmentShaderYellow);
    glLinkProgram(shaderProgramYellow);

    glGetProgramiv(shaderProgramOrange, GL_LINK_STATUS, &success);
    if (!success) {
        const size_t msgLen = 512;
        char msg[msgLen];
        glGetProgramInfoLog(shaderProgramOrange, msgLen, nullptr, msg);
        std::cerr << "Cannot link the shader program with message : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    glGetProgramiv(shaderProgramYellow, GL_LINK_STATUS, &success);
    if (!success) {
        const size_t msgLen = 512;
        char msg[msgLen];
        glGetProgramInfoLog(shaderProgramYellow, msgLen, nullptr, msg);
        std::cerr << "Cannot link the shader program with message : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShaderOrange);
    glDeleteShader(fragmentShaderYellow);

    // setup vertex data
    // ========================================
    float firstTriangle[] = {
        -0.9f, -0.5f, 0.0f,  // left
        -0.0f, -0.5f, 0.0f,  // right
        -0.45f, 0.5f, 0.0f,  // top
    };
    float secondTriangle[] = {
        0.0f, -0.5f, 0.0f,  // left
        0.9f, -0.5f, 0.0f,  // right
        0.45f, 0.5f, 0.0f   // top
    };
    // uint32_t indices[] = {  // note that we start from 0!
    //     0, 1, 3,   // first triangle
    //     1, 2, 3    // second triangle
    // };

    uint32_t VAO[2], VBO[2];
    uint32_t EBO;
    (void)EBO;
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    // glGenBuffers(1, &EBO);

    glBindVertexArray(VAO[0]);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(firstTriangle), firstTriangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid *)0);
    glEnableVertexAttribArray(0); // Bind the attrb inside shader

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondTriangle), secondTriangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.53f, 0.53f, 0.53f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgramOrange);
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(shaderProgramYellow);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
