#include "model.h"

Model::Model() = default;

Model::Model(float p_max_value, glm::mat4 p_translation_matrix, glm::mat4 p_rotation_matrix, float p_scale_stength, char* vertex_path, char* fragment_path)
	:max_value(p_max_value), translation_matrix(p_translation_matrix), rotation_matrix(p_rotation_matrix), scale_stength(p_scale_stength)
{
    this->vbo = 0;
    this->vao = 0;
    this->ibo = 0;
    this->indexCount = 0;
    this->scaling_matrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(scale_stength));

    this->program_id = 0;

    std::string vertex_shader, fragment_shader;
    read_file(vertex_path, vertex_shader);
    read_file(fragment_path, fragment_shader);
    GLuint vertexShader, fragmentShader;

    vertexShader = createShader((GLchar*)vertex_shader.c_str(), GL_VERTEX_SHADER);
    fragmentShader = createShader((GLchar*)fragment_shader.c_str(), GL_FRAGMENT_SHADER);

    this->program_id = createProgram(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

GLuint Model::createProgram(GLuint vsh, GLuint fsh)
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
            char* infoLog = new char[infoLen];
            glGetProgramInfoLog(result, infoLen, NULL, infoLog);
            std::cout << "Shader program linking error" << std::endl << infoLog << std::endl;
            delete[] infoLog;
        }
        glDeleteProgram(result);
        return 0;
    }

    return result;
}

GLuint Model::createShader(const GLchar* code, GLenum type)
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
            std:: cout << "Shader compilation error" << std::endl << infoLog << std::endl;
            delete[] infoLog;
        }
        glDeleteShader(result);
        return 0;
    }

    return result;
}

bool Model::create(void (*callback) (GLfloat*&, GLuint*&, int& width, int& height, float& max_value), int& width, int& height)
{
    GLfloat* l_vertices = new GLfloat[width * height * 4 * 6];
    GLuint* l_indices = new GLuint[width * height * 6];
    callback(l_vertices, l_indices, width, height, this->max_value);

    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, width * height * 4 * 6 * sizeof(GLfloat), l_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &this->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, width * height * 6 * sizeof(GLuint), l_indices, GL_STATIC_DRAW);

    this->indexCount = width * height * 6;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    delete[] l_vertices;
    delete[] l_indices;
    return this->vbo != 0 && this->ibo != 0 && this->vao != 0;
}

void Model::translate(glm::vec3 position)
{
    this->translation_matrix = glm::translate(this->translation_matrix, position);
}

void Model::rotate(float radians, glm::vec3 rotate_vector)
{
    this->rotation_matrix = glm::rotate(this->rotation_matrix, radians, rotate_vector);
}

void Model::scale_scroll(float offset)
{
    if (offset < 0)
        this->scale_stength -= abs((float)offset) * scroll_speed;
    else
        this->scale_stength += abs((float)offset) * scroll_speed;
    if (this->scale_stength < this->min_scale_strength)
        this->scale_stength = this->min_scale_strength;
    this->scaling_matrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(scale_stength));
}

Model::~Model()
{
    if (program_id != 0)
        glDeleteProgram(program_id);
    if (this->vbo != 0)
        glDeleteBuffers(1, &this->vbo);
    if (this->ibo != 0)
        glDeleteBuffers(1, &this->ibo);
    if (this->vao != 0)
        glDeleteVertexArrays(1, &this->vao);
}


void Model::read_file(std::string path, std::string& result) {
    std::ifstream file(path);
    if (file.is_open())
        getline(file, result, '$');
    file.close();
}