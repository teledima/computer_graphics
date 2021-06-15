#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include "arcball_camera.h"

class Model
{
public:
    Model();
    Model(float, glm::mat4, glm::mat4, float, char*, char*);
    GLuint createProgram(GLuint, GLuint);
    GLuint createShader(const GLchar*, GLenum);
    bool create(void (*) (GLfloat*&, GLuint*&, int& width, int& height, float& max_value), int& width, int& height);
    void rotate(float, glm::vec3);
    void translate(glm::vec3);
    void scale_scroll(float offset);
    void read_file(std::string, std::string&);

    float max_value = 1.0f;
    float scale_stength = 1.0f;
    float min_scale_strength = 1.0f;
    static float scroll_speed;

    glm::mat4 translation_matrix = glm::identity<glm::mat4>();
    glm::mat4 rotation_matrix = glm::identity<glm::mat4>();
    glm::mat4 scaling_matrix = glm::identity<glm::mat4>(); 

    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;
    GLsizei indexCount = 0;
    GLuint program_id = 0;
    ~Model();

};