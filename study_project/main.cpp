#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <fstream>
#include <string>
#include "arcball_camera.h"

// MV - Modal View Matrix
// Modal transform (translate, rotate, scale) is to convert from object space to world space.
// View transform is to convert from world space to eye space.

using namespace std;
const GLfloat max_value_unsigned = 10.0f;
const float scroll_speed = 0.01f;

ArcballCamera arcball_camera = ArcballCamera(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 1, 0.1, glm::radians(30.0f), glm::radians(45.0f));

double old_x = numeric_limits<double>::quiet_NaN(), old_y = numeric_limits<double>::quiet_NaN();
double new_x = numeric_limits<double>::quiet_NaN(), new_y = numeric_limits<double>::quiet_NaN();

float start_scale = 1.0f;

glm::mat4 l_model = glm::identity<glm::mat4>();
glm::mat4 l_scale = glm::identity<glm::mat4>();
glm::mat4 l_rotation = glm::identity<glm::mat4>();
glm::mat4 l_translate = glm::identity<glm::mat4>();
glm::mat4 l_Projection = glm::identity<glm::mat4>();

GLFWwindow* g_window;

GLuint g_shaderProgram;
GLint g_uMVP;
GLint g_uMaxValueUnsigned;

class Model
{
public:
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLsizei indexCount;
};

Model g_model;

void read_file(string path, string& result) {
    ifstream file(path);
    if (file.is_open())
        getline(file, result, '$');
    file.close();
}

GLuint createShader(const GLchar* code, GLenum type)
{
    GLuint result = glCreateShader(type);

    glShaderSource(result, 1, &code, nullptr);
    glCompileShader(result);

    GLint compiled;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            char* infoLog = new char[infoLen];
            glGetShaderInfoLog(result, infoLen, NULL, infoLog);
            cout << "Shader compilation error" << endl << infoLog << endl;
            delete[] infoLog;
        }
        glDeleteShader(result);
        return 0;
    }

    return result;
}

GLuint createProgram(GLuint vsh, GLuint fsh)
{
    GLuint result = glCreateProgram();

    glAttachShader(result, vsh);
    glAttachShader(result, fsh);

    glLinkProgram(result);

    GLint linked;
    glGetProgramiv(result, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(result, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            char* infoLog = (char*)alloca(infoLen);
            glGetProgramInfoLog(result, infoLen, NULL, infoLog);
            cout << "Shader program linking error" << endl << infoLog << endl;
        }
        glDeleteProgram(result);
        return 0;
    }

    return result;
}

bool createShaderProgram()
{
    g_shaderProgram = 0;

    string vertex_shader, fragment_shader;
    read_file("vertex_shader.txt", vertex_shader);
    read_file("fragment_shader.txt", fragment_shader);
    GLuint vertexShader, fragmentShader;

    vertexShader = createShader((GLchar*)vertex_shader.c_str(), GL_VERTEX_SHADER);
    fragmentShader = createShader((GLchar*)fragment_shader.c_str(), GL_FRAGMENT_SHADER);

    g_shaderProgram = createProgram(vertexShader, fragmentShader);

    g_uMVP = glGetUniformLocation(g_shaderProgram, "u_mvp");
    g_uMaxValueUnsigned = glGetUniformLocation(g_shaderProgram, "u_max_value_unsigned");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return g_shaderProgram != 0;
}

bool createModel()
{
    const int cubes = 1000;

    GLfloat* vertices = new GLfloat[cubes * cubes * 4 * 6];

    for (int i = 0, position = 0; i < cubes; i++)
        for (int j = 0; j < cubes; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                vertices[position] = (i + (k == 1 || k == 2 ? 1 : 0)) * max_value_unsigned /cubes - max_value_unsigned /2.0f;
                vertices[position + 1] = 0;
                vertices[position + 2] = (j + (k == 2 || k == 3 ? 1 : 0)) * max_value_unsigned / cubes - max_value_unsigned / 2.0f;
                vertices[position + 3] = 0;
                vertices[position + 4] = 1;
                vertices[position + 5] = 0;
                position += 6;
            }
        }

    GLuint* indices = new GLuint[cubes * cubes * 6];

    for (int i = 0, position = 0, start_index = 0; i < cubes; i++)
        for (int j = 0; j < cubes; j++)
        {
            indices[position] = start_index;
            indices[position + 1] = start_index + 1;
            indices[position + 2] = start_index + 2;
            indices[position + 3] = start_index + 2;
            indices[position + 4] = start_index + 3;
            indices[position + 5] = start_index;
            position += 6;
            start_index += 4;
        }

    glGenVertexArrays(1, &g_model.vao);
    glBindVertexArray(g_model.vao);

    glGenBuffers(1, &g_model.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
    glBufferData(GL_ARRAY_BUFFER, cubes * cubes * 4 * 6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_model.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubes * cubes * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);

    g_model.indexCount = cubes * cubes * 6;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    delete[] vertices;
    delete[] indices;

    return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

bool init()
{
    // Set initial color of color buffer to white.
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    return createShaderProgram() && createModel();
}

void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void draw(void)
{
    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_shaderProgram);
    glBindVertexArray(g_model.vao);

    glm::mat4 l_mvp;

    l_model = l_translate * l_rotation * l_scale;

    l_mvp = l_Projection * arcball_camera.getViewMatrix() * l_model;

    glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, glm::value_ptr(l_mvp));
    glUniform1fv(g_uMaxValueUnsigned, 1, &max_value_unsigned);

    glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, NULL);
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isnan(new_x) && isnan(new_y)) glfwGetCursorPos(window, &new_x, &new_y);
    else
    {
        old_x = new_x;
        old_y = new_y;
        glfwGetCursorPos(window, &new_x, &new_y);
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!isnan(old_x) && !isnan(new_y))
        {
            arcball_camera.rotateAzimuth(glm::radians(glm::f32(new_x - old_x) * 180 / 600));
            arcball_camera.rotatePolar(glm::radians(glm::f32(new_y - old_y) * 180 / 600));
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffest)
{
    if (yoffest < 0)
        start_scale -= abs(yoffest) * scroll_speed;
    else
        start_scale += abs(yoffest) * scroll_speed;
    l_scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(start_scale));
}

void cleanup()
{
    if (g_shaderProgram != 0)
        glDeleteProgram(g_shaderProgram);
    if (g_model.vbo != 0)
        glDeleteBuffers(1, &g_model.vbo);
    if (g_model.ibo != 0)
        glDeleteBuffers(1, &g_model.ibo);
    if (g_model.vao != 0)
        glDeleteVertexArrays(1, &g_model.vao);
}

bool initOpenGL()
{
    // Initialize GLFW functions.
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        return false;
    }

    // Request OpenGL 3.3 without obsoleted functions.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window.
    g_window = glfwCreateWindow(800, 600, "OpenGL Test", NULL, NULL);

    // Set display function
    glfwSetCursorPosCallback(g_window, cursor_callback);
    glfwSetScrollCallback(g_window, scroll_callback);

    if (g_window == NULL)
    {
        cout << "Failed to open GLFW window" << endl;
        glfwTerminate();
        return false;
    }

    // Initialize OpenGL context with.
    glfwMakeContextCurrent(g_window);

    // Set internal GLEW variable to activate OpenGL core profile.
    glewExperimental = true;

    // Initialize GLEW functions.
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return false;
    }
    l_translate = glm::translate(l_scale,glm::vec3(0.0f, 0.0f, 0.0f));
    l_scale = glm::scale(l_scale, glm::vec3(start_scale));

    l_Projection = glm::ortho(-1.0f, 1.0f, -4.0f/3.0f, 4.0f/3.0f, 0.1f, 100.0f);

    // Ensure we can capture the escape key being pressed.
    glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);

    // Set callback for framebuffer resizing event.
    glfwSetFramebufferSizeCallback(g_window, reshape);

    return true;
}

void tearDownOpenGL()
{
    // Terminate GLFW.
    glfwTerminate();
}

int main()
{
    // Initialize OpenGL
    if (!initOpenGL())
        return -1;

    // Initialize graphical resources.
    bool isOk = init();
    if (isOk)
    {
        // Main loop until window closed or escape pressed.
        while (glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(g_window) == 0)
        {
            // Draw scene.
            draw();

            // Swap buffers.
            glfwSwapBuffers(g_window);
            // Poll window events.
            glfwPollEvents();
        }
    }

    // Cleanup graphical resources.
    cleanup();

    // Tear down OpenGL.
    tearDownOpenGL();

    return isOk ? 0 : -1;
}