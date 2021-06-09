#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/glut.h>
#include <math.h>

// MV - Modal View Matrix
// Modal transform (translate, rotate, scale) is to convert from object space to world space.
// View transform is to convert from world space to eye space.

using namespace std;
double old_x = numeric_limits<double>::quiet_NaN(), old_y = numeric_limits<double>::quiet_NaN();
double new_x = numeric_limits<double>::quiet_NaN(), new_y = numeric_limits<double>::quiet_NaN();
glm::mat4 l_Model = glm::identity<glm::mat4>();

GLFWwindow* g_window;

GLuint g_shaderProgram;
GLint g_uMVP;

class Model
{
public:
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLsizei indexCount;
};

Model g_model;

GLuint createShader(const GLchar* code, GLenum type)
{
    GLuint result = glCreateShader(type);

    glShaderSource(result, 1, &code, NULL);
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
            delete infoLog;
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

    const GLchar vsh[] =
        "#version 330\n"
        ""
        "layout(location = 0) in vec3 a_position;"
        "layout(location = 1) in vec3 a_color;"
        ""
        "uniform mat4 u_mvp;"
        ""
        "out vec3 v_color;"
        ""
        "void main()"
        "{"
        "    v_color = a_color;"
        "    gl_Position = u_mvp * vec4(a_position, 1.0);"
        "}"
        ;

    const GLchar fsh[] =
        "#version 330\n"
        ""
        "in vec3 v_color;"
        ""
        "layout(location = 0) out vec4 o_color;"
        ""
        "void main()"
        "{"
        "   o_color = vec4(v_color, 1.0);"
        "}"
        ;

    GLuint vertexShader, fragmentShader;

    vertexShader = createShader(vsh, GL_VERTEX_SHADER);
    fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);

    g_shaderProgram = createProgram(vertexShader, fragmentShader);

    g_uMVP = glGetUniformLocation(g_shaderProgram, "u_mvp");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return g_shaderProgram != 0;
}

bool createModel()
{
    const GLfloat vertices[] =
    {
        -1.0, -1.0, 1.0, 1.0, 0.0, 0.0,
        1.0, -1.0, 1.0, 1.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
        -1.0, 1.0, 1.0, 1.0, 0.0, 0.0,

        1.0, -1.0, 1.0, 1.0, 1.0, 0.0,
        1.0, -1.0, -1.0, 1.0, 1.0, 0.0,
        1.0, 1.0, -1.0, 1.0, 1.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 0.0,

        1.0, 1.0, 1.0, 1.0, 0.0, 1.0,
        1.0, 1.0, -1.0, 1.0, 0.0, 1.0,
        -1.0, 1.0, -1.0, 1.0, 0.0, 1.0,
        -1.0, 1.0, 1.0, 1.0, 0.0, 1.0,

        -1.0, 1.0, 1.0, 0.0, 1.0, 1.0,
        -1.0, 1.0, -1.0, 0.0, 1.0, 1.0,
        -1.0, -1.0, -1.0, 0.0, 1.0, 1.0,
        -1.0, -1.0, 1.0, 0.0, 1.0, 1.0,

        -1.0, -1.0, 1.0, 0.0, 1.0, 0.0,
        -1.0, -1.0, -1.0, 0.0, 1.0, 0.0,
        1.0, -1.0, -1.0, 0.0, 1.0, 0.0,
        1.0, -1.0, 1.0, 0.0, 1.0, 0.0,

        -1.0, -1.0, -1.0, 0.0, 0.0, 1.0,
        -1.0, 1.0, -1.0, 0.0, 0.0, 1.0,
        1.0, 1.0, -1.0, 0.0, 0.0, 1.0,
        1.0, -1.0, -1.0, 0.0, 0.0, 1.0,
    };

    const GLuint indices[] =
    {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    glGenVertexArrays(1, &g_model.vao);
    glBindVertexArray(g_model.vao);

    glGenBuffers(1, &g_model.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * 6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_model.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);

    g_model.indexCount = 6 * 6;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

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

    
    glm::mat4 l_Projection;
    glm::mat4 l_mvp;

    l_Projection = glm::ortho(-800.0f / 600.0f, 800.0f / 600.0f, -1.0f, 1.0f, 0.1f, 1000.0f);
    l_Projection = glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 1000.0f) * l_Projection;

    l_mvp = l_Projection * l_Model;

    glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, glm::value_ptr(l_mvp) /*mvp*/);

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
            printf("(old_x: %f, old_y: %f), (new_x: %f, new_y: %f), deviation - %f;\n", old_x, old_y, new_x, new_y, new_x - old_x);

            l_Model = glm::rotate(l_Model, glm::radians(glm::f32(new_x - old_x)*360/800), glm::vec3(0, 0, 1));
            l_Model = glm::rotate(l_Model, glm::radians(glm::f32(new_y - old_y) * 360 / 600), glm::vec3(1, 0, 0));
        }
    }
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

    l_Model = glm::scale(l_Model, glm::vec3(0.25f, 0.25f, 0.25f));
    l_Model = glm::rotate(l_Model, glm::radians(0.0f), glm::vec3(1, 0, 0));
    l_Model = glm::rotate(l_Model, glm::radians(0.0f), glm::vec3(0, 0, 1));

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