#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <fstream>
#include <string>
#include "stb_image.h"
#include "arcball_camera.h"
#include "model.h"

// MV - Modal View Matrix
// Modal transform (translate, rotate, scale) is to convert from object space to world space.
// View transform is to convert from world space to eye space.

using namespace std;
const float max_value_unsigned = 8.0f;
const int quads_surface = 1000;
const int width_box = 4;
const int height_box = 6;

float Model::scroll_speed = 0.01f;

double old_x = numeric_limits<double>::quiet_NaN(), old_y = numeric_limits<double>::quiet_NaN();
double new_x = numeric_limits<double>::quiet_NaN(), new_y = numeric_limits<double>::quiet_NaN();

unsigned int sand_texture;
unsigned int water_texture;


glm::mat4 l_model = glm::identity<glm::mat4>();
glm::mat4 l_Projection = glm::ortho(-1.0f, 1.0f, -4.0f / 3.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
glm::vec3 light_position = glm::vec3(1.0f, 1.0f, 1.0f);

GLFWwindow* g_window;
GLuint program_id;

ArcballCamera* arcball_camera = new ArcballCamera(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 2, 0.1f, glm::radians<float>(30.0f), glm::radians<float>(45.0f));

Model* g_surface_model; 
Model* g_lightbox;

void callbackCreateSurface(GLfloat*& vertices, GLuint*& indices, int &width, int &height, float& max_value)
{
    for (int i = 0, position = 0; i < height; i++)
        for (int j = 0; j < width; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                vertices[position] = (i + (k == 1 || k == 2 ? 1 : 0)) * max_value / height - max_value / 2.0f;
                vertices[position + 1] = 0;
                vertices[position + 2] = (j + (k == 2 || k == 3 ? 1 : 0)) * max_value / width - max_value / 2.0f;
                vertices[position + 3] = 1;
                vertices[position + 4] = 0.7;
                vertices[position + 5] = 0;
                position += 6;
            }
        }

    for (int i = 0, position = 0, start_index = 0; i < height; i++)
        for (int j = 0; j < width; j++)
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
}

void callbackCreateLightBox(GLfloat*& vertices, GLuint*& indices, int& width, int& height, float& max_value)
{
    vertices = new GLfloat[]
    {
        -1.0, -1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, -1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        -1.0, 1.0, 1.0, 1.0, 1.0, 1.0,

        1.0, -1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, -1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0,

        1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        -1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        -1.0, 1.0, 1.0, 1.0, 1.0, 1.0,

        -1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        -1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        -1.0, -1.0, -1.0, 1.0, 1.0, 1.0,
        -1.0, -1.0, 1.0, 1.0, 1.0, 1.0,

        -1.0, -1.0, 1.0, 1.0, 1.0, 1.0,
        -1.0, -1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, -1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, -1.0, 1.0, 1.0, 1.0, 1.0,

        -1.0, -1.0, -1.0, 1.0, 1.0, 1.0,
        -1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, -1.0, -1.0, 1.0, 1.0, 1.0,
    };

    indices = new GLuint[]
    {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
}

bool init()
{
    // Set initial color of color buffer to white.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


    glEnable(GL_DEPTH_TEST);
    g_surface_model = new Model(max_value_unsigned, glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), 1.5f, (char*)"vertex_shader.txt", (char*)"fragment_shader.txt");
    g_lightbox = new Model(1.0, g_surface_model->translation_matrix, g_surface_model->rotation_matrix, 0.01f, (char*)"vertex_shader_lightbox.txt", (char*)"fragment_shader_lightbox.txt");
    bool success_model_create = g_surface_model->create(callbackCreateSurface, (int&)quads_surface, (int&)quads_surface);
    bool succes_lightbox_create = g_lightbox->create(callbackCreateLightBox, (int&)width_box, (int&)height_box);

    glGenTextures(1, &sand_texture);
    glBindTexture(GL_TEXTURE_2D, sand_texture);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("sand.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load sand texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
    glGenTextures(1, &water_texture);
    glBindTexture(GL_TEXTURE_2D, water_texture);
    data = stbi_load("water.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load water texture" << std::endl;
    }
    stbi_image_free(data);


    if (g_surface_model->program_id != 0 && success_model_create && g_lightbox -> program_id != 0 && succes_lightbox_create)
    {
        g_surface_model->translate(glm::vec3(0.0f, 0.0f, 0.0f));

        g_lightbox->translate(light_position);
        return 1;
    }
    else return 0;
}

void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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
            arcball_camera->rotateAzimuth(glm::radians(glm::f32(new_x - old_x) * 180 / 600));
            arcball_camera->rotatePolar(glm::radians(glm::f32(new_y - old_y) * 180 / 600));
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_surface_model->scale_scroll((float)yoffset);
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
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sand_texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, water_texture);

            glUseProgram(g_surface_model->program_id);
            glBindVertexArray(g_surface_model->vao);

            glm::mat4 l_mvp;
            glm::mat4 l_m;
            glm::mat3 l_normal_m;
            l_mvp = l_Projection * arcball_camera->getViewMatrix() * g_surface_model->translation_matrix * g_surface_model->rotation_matrix * g_surface_model->scaling_matrix;
            l_m = g_surface_model->translation_matrix * g_surface_model->rotation_matrix * g_surface_model->scaling_matrix;
            l_normal_m = (glm::transpose(glm::inverse(glm::mat3(l_m))));

            glUniformMatrix4fv(glGetUniformLocation(g_surface_model->program_id, "u_mvp"), 1, GL_FALSE, glm::value_ptr(l_mvp));
            glUniform1fv(glGetUniformLocation(g_surface_model->program_id, "u_max_value_unsigned"), 1, &g_surface_model->max_value);
            glUniformMatrix4fv(glGetUniformLocation(g_surface_model->program_id, "u_m"), 1, GL_FALSE, glm::value_ptr(l_m));
            glUniformMatrix3fv(glGetUniformLocation(g_surface_model->program_id, "u_normal_m"), 1, GL_FALSE, glm::value_ptr(l_normal_m));
            glUniform3fv(glGetUniformLocation(g_surface_model->program_id, "u_light_pos"), 1, glm::value_ptr(glm::vec4(light_position, 1.0f)));
            glUniform3fv(glGetUniformLocation(g_surface_model->program_id, "u_view_pos"), 1, glm::value_ptr(arcball_camera->getEye()));
            glUniform1i(glGetUniformLocation(g_surface_model->program_id, "u_texture_sand"), 0);
            glUniform1i(glGetUniformLocation(g_surface_model->program_id, "u_texture_water"), 1);

            glDrawElements(GL_TRIANGLES, g_surface_model->indexCount, GL_UNSIGNED_INT, NULL);
            
            glUseProgram(g_lightbox->program_id);
            glBindVertexArray(g_lightbox->vao);

            l_mvp = l_Projection * arcball_camera->getViewMatrix() * g_lightbox->translation_matrix * g_lightbox->rotation_matrix * g_surface_model->scaling_matrix * g_lightbox->scaling_matrix;

            glUniformMatrix4fv(glGetUniformLocation(g_lightbox->program_id, "u_mvp"), 1, GL_FALSE, glm::value_ptr(l_mvp));
            glUniform1fv(glGetUniformLocation(g_lightbox->program_id, "u_max_value_unsigned"), 1, &g_lightbox->max_value);

            glDrawElements(GL_TRIANGLES, g_lightbox->indexCount, GL_UNSIGNED_INT, NULL);

            // Swap buffers.
            glfwSwapBuffers(g_window);
            // Poll window events.
            glfwPollEvents();
        }
    }

    // Cleanup graphical resources.
    delete g_surface_model;
    delete g_lightbox;
    delete arcball_camera;
    glDeleteTextures(1, &sand_texture);
    glDeleteTextures(1, &water_texture);

    // Tear down OpenGL.
    tearDownOpenGL();

    return isOk ? 0 : -1;
}