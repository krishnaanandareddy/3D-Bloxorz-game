#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

GLFWwindow *window;

struct VAO
{
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;
    GLuint NormalBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int vertices_count;
};
typedef struct VAO VAO;

struct GLMatrices
{
    glm::mat4 Orthographic_Projection, Perspective_Projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint Program_ID;
int proj_type;
glm::vec3 tri_pos, rect_pos;

// Reference:  https://www.glfw.org/docs/3.3/quick.html
// Standard shaders code from OpenGL documentation
GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path)
{

    GLuint Vertex_Shader_ID = glCreateShader(GL_VERTEX_SHADER);
    GLuint Fragment_Shader_ID = glCreateShader(GL_FRAGMENT_SHADER);

    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open())
    {
        std::string Line = "";
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open())
    {
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(Vertex_Shader_ID, 1, &VertexSourcePointer, NULL);
    glCompileShader(Vertex_Shader_ID);

    glGetShaderiv(Vertex_Shader_ID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(Vertex_Shader_ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(Vertex_Shader_ID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);

    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(Fragment_Shader_ID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(Fragment_Shader_ID);

    glGetShaderiv(Fragment_Shader_ID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(Fragment_Shader_ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(Fragment_Shader_ID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);

    GLuint Program_ID = glCreateProgram();
    glAttachShader(Program_ID, Vertex_Shader_ID);
    glAttachShader(Program_ID, Fragment_Shader_ID);
    glLinkProgram(Program_ID);

    glGetProgramiv(Program_ID, GL_LINK_STATUS, &Result);
    glGetProgramiv(Program_ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage(max(InfoLogLength, int(1)));
    glGetProgramInfoLog(Program_ID, InfoLogLength, NULL, &ProgramErrorMessage[0]);

    glDeleteShader(Vertex_Shader_ID);
    glDeleteShader(Fragment_Shader_ID);
    return Program_ID;
}

static void print_error(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void initGLEW(void)
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
    }
    if (!GLEW_VERSION_3_3)
        fprintf(stderr, "3.3 version not available\n");
}

// Reference: https://github.com/anushkawakankar/3D-OpenGl/tree/master/src
// Creating a structure for 3D objects
struct VAO *create_object_3D(GLenum primitive_mode, int vertices_count, const GLfloat *vertex_buffer_data, const GLfloat *color_buffer_data, GLenum fill_mode = GL_FILL)
{
    // primitive_mode is like using which primitives you are doing solid modelling
    // like GL_TRIANGLES
    struct VAO *vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->vertices_count = vertices_count;
    vao->FillMode = fill_mode;

    glGenVertexArrays(1, &(vao->VertexArrayID));
    glGenBuffers(1, &(vao->VertexBuffer));
    glGenBuffers(1, &(vao->ColorBuffer));
    glGenBuffers(1, &(vao->NormalBuffer));

    glBindVertexArray(vao->VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 3 * vertices_count * sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, 3 * vertices_count * sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void *)0);

    GLfloat normal_buffer_data[] = {
        0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, // 1
        0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, // 2
        -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, // 3

        0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, // 4
        0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, // 5
        1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0  // 6
    };

    glBindBuffer(GL_ARRAY_BUFFER, vao->NormalBuffer);
    glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(GLfloat), normal_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void *)(0 * sizeof(float)));

    return vao;
}

struct VAO *create_object_3D(GLenum primitive_mode, int vertices_count, const GLfloat *vertex_buffer_data, const GLfloat Color, GLenum fill_mode = GL_FILL)
{
    GLfloat *color_buffer_data = new GLfloat[3 * vertices_count];
    for (int i = 0; i < vertices_count; i++)
    {
        color_buffer_data[3 * i] = Color;
        color_buffer_data[3 * i + 1] = Color;
        color_buffer_data[3 * i + 2] = Color;
    }

    return create_object_3D(primitive_mode, vertices_count, vertex_buffer_data, color_buffer_data, fill_mode);
}

void Draw_object_3D(struct VAO *vao)
{
    glPolygonMode(GL_FRONT_AND_BACK, vao->FillMode);

    glBindVertexArray(vao->VertexArrayID);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, vao->NormalBuffer);

    glDrawArrays(vao->PrimitiveMode, 0, vao->vertices_count);
}

int perspective = 0;
void window_resize(GLFWwindow *window, int width, int height)
{
    int fbwidth = width, fbheight = height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = M_PI / 2;

    glViewport(0, 0, (GLsizei)fbwidth, (GLsizei)fbheight);

    Matrices.Perspective_Projection = glm::perspective(fov, (GLfloat)fbwidth / (GLfloat)fbheight, 0.1f, 500.0f);

    Matrices.Orthographic_Projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

GLfloat *colour_init(float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3)
{

    GLfloat *color_buffer_data = (GLfloat *)malloc(sizeof(float) * 108);

    GLfloat Color[] = {
        // 6 sides and on each side 6 vertices.
        // this buffer is used as colour buffer for our initialised cell
        r1, g1, b1, r1, g1, b1, r1, g1, b1, r1 / 1.06f, g1 / 1.06f, b1 / 1.06f, r1 / 1.06f, g1 / 1.06f, b1 / 1.06f, r1 / 1.06f, g1 / 1.06f, b1 / 1.06f, // 1
        r2, g2, b2, r2, g2, b2, r2, g2, b2, r2 / 1.06f, g2 / 1.06f, b2 / 1.06f, r2 / 1.06f, g2 / 1.06f, b2 / 1.06f, r2 / 1.06f, g2 / 1.06f, b2 / 1.06f, // 2
        r3, g3, b3, r3, g3, b3, r3, g3, b3, r3 / 1.06f, g3 / 1.06f, b3 / 1.06f, r3 / 1.06f, g3 / 1.06f, b3 / 1.06f, r3 / 1.06f, g3 / 1.06f, b3 / 1.06f, // 3

        r1, g1, b1, r1, g1, b1, r1, g1, b1, r1 / 1.06f, g1 / 1.06f, b1 / 1.06f, r1 / 1.06f, g1 / 1.06f, b1 / 1.06f, r1 / 1.06f, g1 / 1.06f, b1 / 1.06f, // 4
        r2, g2, b2, r2, g2, b2, r2, g2, b2, r2 / 1.06f, g2 / 1.06f, b2 / 1.06f, r2 / 1.06f, g2 / 1.06f, b2 / 1.06f, r2 / 1.06f, g2 / 1.06f, b2 / 1.06f, // 5
        r3, g3, b3, r3, g3, b3, r3, g3, b3, r3 / 1.06f, g3 / 1.06f, b3 / 1.06f, r3 / 1.06f, g3 / 1.06f, b3 / 1.06f, r3 / 1.06f, g3 / 1.06f, b3 / 1.06f, // 6
    };

    for (int i = 0; i < 108; i++)
        color_buffer_data[i] = Color[i];

    return color_buffer_data;
}

VAO *initialize_cell(float l, float b, float h, GLfloat Color[])
{
    GLfloat vertex_buffer_data[] = {
        // 6 surfaces and 6 vertices on each surface 2 triangles
        0, 0, 0, b, 0, 0, b, h, 0, b, h, 0, 0, h, 0, 0, 0, 0, // 1
        0, 0, 0, 0, h, 0, 0, h, l, 0, h, l, 0, 0, l, 0, 0, 0, // 2
        0, 0, 0, 0, 0, l, b, 0, l, b, 0, l, b, 0, 0, 0, 0, 0, // 3

        0, 0, l, b, 0, l, b, h, l, b, h, l, 0, h, l, 0, 0, l, // 4
        b, 0, l, b, 0, 0, b, h, 0, b, h, 0, b, h, l, b, 0, l, // 5
        0, h, l, b, h, l, b, h, 0, b, h, 0, 0, h, 0, 0, h, l  // 6
    };

    return create_object_3D(GL_TRIANGLES, 36, vertex_buffer_data, Color, GL_FILL);
}

// Reference: https://github.com/sudheerachary/OpenGL-3D-Bloxorz/blob/master/Sample_GL3_2D.cpp
class create_graphical_object
{
public:
    VAO *object;
    float coord_x;
    float coord_y;
    float coord_z;
    float height;
    float length;
    char color;
    glm::mat4 inverse_translation_multiplier_matrix;
    glm::mat4 Inverse_rotation_multiplier_matrix;
    glm::mat4 translation_multiplier_matrix;
    glm::mat4 rotation_multiplier_matrix;

public:
    create_graphical_object(float X = 0, float Y = 0, float Z = 0, float H = 0, float L = 0, char colour = 'D')
    {
        coord_x = X;
        coord_y = Y;
        coord_z = Z;
        height = H;
        length = L;
        color = colour;
    }

    void Inverse_rotation(float rotation = 0, glm::vec3 rotating_vector = glm::vec3(0, 0, 1))
    {
        Inverse_rotation_multiplier_matrix = glm::rotate((float)(rotation * M_PI / 180.0f), rotating_vector);
    }

    void rotation(float rotation = 0, glm::vec3 rotating_vector = glm::vec3(0, 0, 1))
    {
        rotation_multiplier_matrix = glm::rotate((float)(rotation * M_PI / 180.0f), rotating_vector);
    }

    void translation(float x = 0, float y = 0, float z = 0)
    {
        translation_multiplier_matrix = glm::translate(glm::vec3(x, y, z));
    }

    void Inverse_translation(float x = 0, float y = 0, float z = 0)
    {
        inverse_translation_multiplier_matrix = glm::translate(glm::vec3(x, y, z));
    }

    void render()
    {
        glm::mat4 VP = (perspective ? Matrices.Perspective_Projection : Matrices.Orthographic_Projection) * Matrices.view;
        glm::mat4 MVP;
        Matrices.model = glm::mat4(1.0f);
        Matrices.model *= translation_multiplier_matrix * rotation_multiplier_matrix * inverse_translation_multiplier_matrix * Inverse_rotation_multiplier_matrix;

        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        Draw_object_3D(object);
    }
};

GLfloat top_horizontal[] = {
    0.1, 0.35, 0,
    -0.1, 0.35, 0,
    -0.1, 0.30, 0,

    -0.1, 0.30, 0,
    0.1, 0.30, 0,
    0.1, 0.35, 0};

GLfloat middle_horizontal[] = {
    0.1, 0.05, 0,
    -0.1, 0.05, 0,
    -0.1, -0.05, 0,

    -0.1, -0.05, 0,
    0.1, -0.05, 0,
    0.1, 0.05, 0};

GLfloat bottom_horizontal[] = {
    0.1, -0.30, 0,
    -0.1, -0.30, 0,
    -0.1, -0.35, 0,

    -0.1, -0.35, 0,
    0.1, -0.35, 0,
    0.1, -0.30, 0};

GLfloat letft_top_vertical[] = {
    -0.05, 0.30, 0,
    -0.1, 0.30, 0,
    -0.1, 0.05, 0,

    -0.1, 0.05, 0,
    -0.05, 0.05, 0,
    -0.05, 0.30, 0};

GLfloat left_bottom_vertical[] = {
    -0.05, -0.05, 0,
    -0.1, -0.05, 0,
    -0.1, -0.30, 0,

    -0.1, -0.30, 0,
    -0.05, -0.30, 0,
    -0.05, -0.05, 0};

GLfloat right_top_vertical[] = {
    0.1, 0.30, 0,
    0.05, 0.30, 0,
    0.05, 0.05, 0,

    0.05, 0.05, 0,
    0.1, 0.05, 0,
    0.1, 0.30, 0};
GLfloat right_bottom_vertical[] = {
    0.1, -0.05, 0,
    0.05, -0.05, 0,
    0.05, -0.30, 0,

    0.05, -0.30, 0,
    0.1, -0.30, 0,
    0.1, -0.05, 0};

GLfloat dark_yellow_color[] = {
    1, 0.5, 0,
    1, 0.5, 0,
    1, 0.5, 0,

    1, 0.5, 0,
    1, 0.5, 0,
    1, 0.5, 0};

GLfloat *White = colour_init(1, 0.96, 0.8, 1, 0.96, 0.8, 0.98, 0.97, 0.88);

GLfloat *Grey = colour_init(0.86, 0.85, 0.81, 0.86, 0.85, 0.81, 0.76, 0.75, 0.69);

GLfloat *Yellow = colour_init(0.89, 0.98, 0.35, 0.88, 1, 0.2, 0.85, 1, 0);

GLfloat *Blue = colour_init(0.08, 0.65, 0.7, 0, 0.7, 0.75, 0.2, 0.77, 0.86);

GLfloat *Green = colour_init(0.1, 0.6, 0.1, 0.1, 0.7, 0.1, 0.4, 0.7, 0.1);

GLfloat *Orange = colour_init(0.8, 0.6, 0.1, 0.9, 0.7, 0, 1, 0.8, 0.4);

static const int board_dimension = 20;

int board[board_dimension][board_dimension];

// Initialising board values of different stages
int stage1[board_dimension][board_dimension] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

int stage2[board_dimension][board_dimension] = {
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

int stage3[board_dimension][board_dimension] = {
    {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 4, 1, 0, 0, 1, 1, 5, 1, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 7, 7, 1, 1, 1, 1, 7, 7, 1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

int stage4[board_dimension][board_dimension] = {

};

glm::vec3 eye;
glm::vec3 target;

map<int, vector<int>> bridge_coords_Map;

float theta = 0.0f,
      coord_z = 0.0f,
      coord_y = 0.0f,
      coord_x = 0.0f,
      camera_rotation_angle = 70.0f;

int game_level = 1, begin_Stage = 1,
    old_Bridge = 4, OldBridge[10], bridge[10],
    present_Block_State = 0, future_Block_State = 0, direction = 5,
    views = 0, Block_Moves = 0,
    left_button = 0, right_button = 0;

VAO *axes,
    *cell,
    *background;

create_graphical_object Block,
    Board[20][20];

void compute_score(double x, double y, double z, int score)
{
    double coord_x = x, coord_y = y, coord_z = z;
    glm::mat4 VP = (perspective ? Matrices.Perspective_Projection : Matrices.Orthographic_Projection) * Matrices.view;
    glm::mat4 MVP;
    glm::mat4 Inverse_rotation = glm::rotate(0.0f, glm::vec3(0, 1, 0));
    glm::mat4 Inverse_translation = glm::translate(glm::vec3(0.1f, 0, 0));
    glm::mat4 translation = glm::translate(glm::vec3(coord_x, coord_y, coord_z));
    if (score == 0)
    {
        Matrices.model = glm::mat4(1.0f);
        Matrices.model *= translation * Inverse_rotation * Inverse_translation;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
        Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, letft_top_vertical, dark_yellow_color, GL_FILL));
        Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, left_bottom_vertical, dark_yellow_color, GL_FILL));
        Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, bottom_horizontal, dark_yellow_color, GL_FILL));
        Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
        Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
        return;
    }
    int tmp = score;
    while (tmp != 0)
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translation = glm::translate(glm::vec3(coord_x, coord_y, 0));
        // Defining which objects to draw based on the number we have
        switch (tmp % 10)
        {
        case 0:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, letft_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, left_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, bottom_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
            break;
        case 1:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
            break;
        case 2:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, middle_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, left_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, bottom_horizontal, dark_yellow_color, GL_FILL));
            break;
        case 3:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, bottom_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, middle_horizontal, dark_yellow_color, GL_FILL));
            break;
        case 4:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, letft_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, middle_horizontal, dark_yellow_color, GL_FILL));
            break;
        case 5:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, letft_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, middle_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, bottom_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            break;
        case 6:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, letft_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, middle_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, left_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, bottom_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            break;
        case 7:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
            break;
        case 8:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, letft_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, left_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, bottom_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, middle_horizontal, dark_yellow_color, GL_FILL));
            break;
        case 9:
            Matrices.model *= translation * Inverse_rotation * Inverse_translation;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, top_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, letft_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, bottom_horizontal, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_bottom_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, right_top_vertical, dark_yellow_color, GL_FILL));
            Draw_object_3D(create_object_3D(GL_TRIANGLES, 6, middle_horizontal, dark_yellow_color, GL_FILL));
            break;
        }
        tmp = tmp / 10;
        coord_x -= 0.3;
    }
}

void keyboard_Action(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
    {
        switch (key)
        {
        default:
            break;
        }
    }
    else if (action == GLFW_PRESS)
    {
        // based on the key pressed by user action is taken and state of block is changed
        // which helps in finding future coords of Block

        // when begin_stage is 1 we are creating board and block so untill begin_stage is 0 we shouldn't move the block
        switch (key)
        {
        case GLFW_KEY_V:
            views = (views + 1) % 4;
            break;

        case GLFW_KEY_UP:
            if (!begin_Stage)
            {
                if (present_Block_State == 0)
                    future_Block_State = 1;
                else if (present_Block_State == 1)
                    future_Block_State = 0;
                else
                    future_Block_State = 2;
                direction = 8; // up
                Block_Moves++;
                system("mpg123 -n 30 -i -q move.mp3 &");
            }
            break;

        case GLFW_KEY_DOWN:
            if (!begin_Stage)
            {
                if (present_Block_State == 0)
                    future_Block_State = 1;
                else if (present_Block_State == 1)
                    future_Block_State = 0;
                else if (present_Block_State == 2)
                    future_Block_State = 2;
                direction = 2;
                Block_Moves++;
                system("mpg123 -n 30 -i -q move.mp3 &");
            }
            break;

        case GLFW_KEY_LEFT:
            if (!begin_Stage)
            {
                if (present_Block_State == 0)
                    future_Block_State = 2;
                else if (present_Block_State == 1)
                    future_Block_State = 1;
                else if (present_Block_State == 2)
                    future_Block_State = 0;
                direction = 4;
                Block_Moves++;
                system("mpg123 -n 30 -i -q move.mp3 &");
            }
            break;

        case GLFW_KEY_RIGHT:
            if (!begin_Stage)
            {
                if (present_Block_State == 0)
                    future_Block_State = 2;
                else if (present_Block_State == 1)
                    future_Block_State = 1;
                else if (present_Block_State == 2)
                    future_Block_State = 0;
                direction = 6;
                Block_Moves++;
                system("mpg123 -n 30 -i -q move.mp3 &");
            }
            break;

        case GLFW_KEY_ESCAPE:
            quit(window);
            break;
        default:
            break;
        }
    }
}

// Function for quitting windoe in case of q or Q
void keyboardChar(GLFWwindow *window, unsigned int key)
{
    if (key == 'Q' || key == 'q')
    {
        quit(window);
    }
}

// Reference: https://github.com/sudheerachary/OpenGL-3D-Bloxorz/blob/master/Sample_GL3_2D.cpp
// Defines the moving/rolling dynamics of the block
void roll_Block()
{
    // changing theta by 10 degrees everytime gives the animation of block rotation
    if (theta < 90 && direction != 5)
        theta += 10;
    else
    {
        // based on the direction and current state of block we change the coordinates of block
        if (present_Block_State != future_Block_State || direction != 5)
        {
            if (present_Block_State == 0)
            {
                if (direction == 8)
                    Block.coord_z -= Block.height;
                else if (direction == 2)
                    Block.coord_z += Block.length;
                else if (direction == 4)
                    Block.coord_x -= Block.height;
                else
                    Block.coord_x += Block.length;
            }
            else if (present_Block_State == 1)
            {
                if (direction == 8)
                    Block.coord_z -= Block.length;
                else if (direction == 2)
                    Block.coord_z += Block.height;
                else if (direction == 4)
                    Block.coord_x -= Block.length;
                else
                    Block.coord_x += Block.length;
            }
            else
            {
                if (direction == 8)
                    Block.coord_z -= Block.length;
                else if (direction == 2)
                    Block.coord_z += Block.length;
                else if (direction == 4)
                    Block.coord_x -= Block.length;
                else
                    Block.coord_x += Block.height;
            }
        }
        present_Block_State = future_Block_State;
        theta = 0;
        // Direction 5 is default direction, which states no need to update our block's coordinates
        direction = 5;
    }

    if (present_Block_State == 0)
    {
        if (direction == 6)
        {
            Block.Inverse_rotation();
            Block.Inverse_translation(-Block.length, 0, 0);
            Block.rotation(-theta, glm::vec3(0, 0, 1));
            Block.translation(Block.coord_x + Block.length - 1, Block.coord_y, Block.coord_z - 1);
        }
        else if (direction == 4)
        {
            Block.Inverse_rotation();
            Block.Inverse_translation();
            Block.rotation(theta, glm::vec3(0, 0, 1));
            Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z - 1);
        }
        else if (direction == 8)
        {
            Block.Inverse_rotation();
            Block.Inverse_translation();
            Block.rotation(-theta, glm::vec3(1, 0, 0));
            Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z - 1);
        }
        else if (direction == 2)
        {
            Block.Inverse_rotation();
            Block.Inverse_translation(0, 0, -Block.length);
            Block.rotation(theta, glm::vec3(1, 0, 0));
            Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z + Block.length - 1);
        }
        else
        {
            Block.Inverse_rotation();
            Block.Inverse_translation();
            Block.rotation();
            Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z - 1);
        }
    }
    else if (present_Block_State == 1)
    {
        if (direction == 6)
        {
            Block.Inverse_rotation(-90, glm::vec3(1, 0, 0));
            Block.Inverse_translation(-Block.length, 0, 0);
            Block.rotation(-theta, glm::vec3(0, 0, 1));
            Block.translation(Block.coord_x + Block.length - 1, Block.coord_y, Block.coord_z + Block.height - 1);
        }
        else if (direction == 4)
        {
            Block.Inverse_rotation(-90, glm::vec3(1, 0, 0));
            Block.Inverse_translation();
            Block.rotation(theta, glm::vec3(0, 0, 1));
            Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z + Block.height - 1);
        }
        else if (direction == 8)
        {
            Block.Inverse_rotation(-90, glm::vec3(1, 0, 0));
            Block.Inverse_translation(0, 0, Block.height);
            Block.rotation(-theta, glm::vec3(1, 0, 0));
            Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z - 1);
        }
        else if (direction == 2)
        {
            Block.Inverse_rotation(-90, glm::vec3(1, 0, 0));
            Block.Inverse_translation();
            Block.rotation(theta, glm::vec3(1, 0, 0));
            Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z + Block.height - 1);
        }
        else
        {
            Block.Inverse_rotation(-90, glm::vec3(1, 0, 0));
            Block.Inverse_translation(Block.coord_x - 1, Block.coord_y, Block.coord_z + Block.height - 1);
            Block.rotation();
            Block.translation();
        }
    }
    else
    {
        if (direction == 6)
        {
            Block.Inverse_rotation(90, glm::vec3(0, 0, 1));
            Block.Inverse_translation();
            Block.rotation(-theta, glm::vec3(0, 0, 1));
            Block.translation(Block.coord_x + Block.height - 1, Block.coord_y, Block.coord_z - 1);
        }
        else if (direction == 4)
        {
            Block.Inverse_rotation(90, glm::vec3(0, 0, 1));
            Block.Inverse_translation(Block.height, 0, 0);
            Block.rotation(theta, glm::vec3(0, 0, 1));
            Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z - 1);
        }
        else if (direction == 8)
        {
            Block.Inverse_rotation(90, glm::vec3(0, 0, 1));
            Block.Inverse_translation();
            Block.rotation(-theta, glm::vec3(1, 0, 0));
            Block.translation(Block.coord_x + Block.height - 1, Block.coord_y, Block.coord_z - 1);
        }
        else if (direction == 2)
        {
            Block.Inverse_rotation(90, glm::vec3(0, 0, 1));
            Block.Inverse_translation(0, 0, -Block.length);
            Block.rotation(theta, glm::vec3(1, 0, 0));
            Block.translation(Block.coord_x + Block.height - 1, Block.coord_y, Block.coord_z + Block.length - 1);
        }
        else
        {
            Block.Inverse_rotation();
            Block.Inverse_translation();
            Block.rotation(90, glm::vec3(0, 0, 1));
            Block.translation(Block.coord_x + Block.height - 1, Block.coord_y, Block.coord_z - 1);
        }
    }
}

void bridgeConstruct()
{
    // vector<int> v;
    // bridges are also constructed in chk_block_status function
    // here we are just creating cordinates of brige when it needs to be created "bridge_coords_Map"
    map<int, vector<int>>::iterator itr;
    for (itr = bridge_coords_Map.begin(); itr != bridge_coords_Map.end(); itr++)
    {
        vector<int> V = bridge_coords_Map[itr->first];
        for (int i = 0; i < board_dimension; i++)
        {
            for (int j = 0; j < board_dimension; j++)
            {
                if (board[i][j] == itr->first)
                {
                    for (int i = 0; i < 4; i += 2)
                        board[V[i]][V[i + 1]] = 7;

                    bridge[itr->first] = 0;
                    OldBridge[itr->first] = 1;
                }
            }
        }
    }

    // Initialising Bridge 1 and Bridge 2 coordiates

    // Bridge 1
    //(3,4) (3,5) are briges coordinates
    bridge_coords_Map[4] = {3, 4, 3, 5};

    // Bridge 2
    //(3,10) (3,11) are briges coordinates
    bridge_coords_Map[5] = {3, 10, 3, 11};
}

void next_stage()
{

    // when game_level up change board to next stage
    game_level++;
    switch (game_level)
    {
    case 2:
        for (int i = 0; i < board_dimension; i++)
            for (int j = 0; j < board_dimension; j++)
                board[i][j] = stage2[i][j];
        break;
    case 3:
        for (int i = 0; i < board_dimension; i++)
            for (int j = 0; j < board_dimension; j++)
                board[i][j] = stage3[i][j];
        break;
    default:
        quit(window);
        break;
    };

    // since the board has changed initiase the new cells and display those graphical objects
    coord_z = 0.0f;
    for (int i = 0; i < board_dimension; i++)
    {
        VAO *cell;
        create_graphical_object temp;
        coord_x = 0.0f;
        for (int j = 0; j < board_dimension; j++)
        {
            coord_y = 0.0f;
            if (board[i][j] == 2)
            {
                cell = initialize_cell(0.3f, 0.3f, -0.1f, Yellow);
                temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'r');
            }
            else if (board[i][j] == 1)
            {
                if ((i + j) % 2 == 0)
                {
                    cell = initialize_cell(0.3f, 0.3f, -0.1f, White);
                    temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'r');
                }
                else
                {
                    cell = initialize_cell(0.3f, 0.3f, -0.1f, Grey);
                    temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'g');
                }
            }
            else if (board[i][j] == 3)
            {
                if ((i + j) % 2 == 0)
                {
                    cell = initialize_cell(0.3f, 0.3f, -0.1f, Orange);
                    temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'b');
                }
                else
                {
                    cell = initialize_cell(0.3f, 0.3f, -0.1f, Orange);
                    temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'b');
                }
            }
            else
            {
                cell = initialize_cell(0.3f, 0.3f, -0.1f, Green);
                temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'b');
            }
            temp.object = cell;
            Board[i][j] = temp;
            coord_x += 0.3f;
        }
        coord_z += 0.3f;
    }

    // call bridgeConstruct when game_levelup to store new bridge coords if exists
    bridgeConstruct();
}

void Draw_Floor_Board()
{
    for (int i = 0; i < board_dimension; i++)
    {
        for (int j = 0; j < board_dimension; j++)
        {
            Board[i][j].translation(Board[i][j].coord_x - 1,
                                    Board[i][j].coord_y,
                                    Board[i][j].coord_z - 1);
            if (board[i][j] != 0 && board[i][j] != 2 && board[i][j] != 7 && Board[i][j].coord_y > -4.0f)
                Board[i][j].render();
            // edited
            if (board[i][j] != 0 && board[i][j] != 7 && Board[i][j].coord_y > -4.0f)
                Board[i][j].render();
        }
    }
}

void Collapse_Board_Cells()
{
    if (Block.coord_y > -6.0f)
    {
        Block.coord_y -= 0.1f;
        Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z - 1);
        return;
    }
    for (int i = 0; i < board_dimension; i++)
    {
        for (int j = 0; j < board_dimension; j++)
        {
            if (Board[i][j].coord_y >= -5.0f &&
                board[i][j] != 0 &&
                board[i][j] != 2)
            {
                // give a value less than -5.0f to give an animation
                Board[i][j].coord_y -= 5.0f;
            }
        }
    }
    begin_Stage = 1;
}

void create_Board_Block()
{
    for (int i = 0; i < board_dimension; i++)
    {
        for (int j = 0; j < board_dimension; j++)
        {
            if (Board[i][j].coord_y < -0.1f && board[i][j] != 0 && board[i][j] != 2)
            {
                Board[i][j].coord_y += 5.0f;
                return;
            }
        }
    }
    begin_Stage = 0;
}

// Reference: https://github.com/sudheerachary/OpenGL-3D-Bloxorz/blob/master/Sample_GL3_2D.cpp
// checks the blocks present state and coordinates to verify if the block is in a valid state on top of the board
int chk_Block_State()
{
    // since each cell is of 0.3f and 0.3f we map them i,j borad format
    int i = (Block.coord_z * 10) / 3;
    int j = (Block.coord_x * 10) / 3;

    // reset case
    if ((i < 0 || j < 0) ||
        (present_Block_State == 0 && (board[i][j] == 0 || board[i][j] == 7 || board[i][j] == 3)) ||
        (present_Block_State == 1 && (board[i][j] == 0 || board[i + 1][j] == 0 || board[i + 1][j] == 7 || board[i][j + 1] == 7)) ||
        (present_Block_State == 2 && (board[i][j] == 0 || board[i][j + 1] == 0 || board[i][j] == 7 || board[i][j + 1] == 7)))
        return 1;

    // solution case
    if (present_Block_State == 0 && board[i][j] == 2)
        return 2;

    // bridging case since we are using >3 values for bridge
    if ((present_Block_State == 0 && board[i][j] > 3) ||
        (present_Block_State == 1 && (board[i][j] > 3 || board[i + 1][j] > 3)) ||
        (present_Block_State == 2 && (board[i][j] > 3 || board[i][j + 1] > 3)))
    {
        // we are finding which bridge activation cell our block is on  (a,b)

        int a = i, b = j;
        if (present_Block_State == 0)
        {
            a = i;
            b = j;
        }
        else if (present_Block_State == 1)
        {
            if (board[i][j] > 3)
            {
                a = i;
                b = j;
            }
            else
            {
                a = i + 1;
                b = j;
            }
        }
        else
        {
            if (board[i][j] > 3)
            {
                a = i;
                b = j;
            }
            else
            {
                a = i;
                b = j + 1;
            }
        }

        // since we found a, b
        vector<int> V = bridge_coords_Map[board[a][b]];

        // if there is no bridge activated for (a,b) then create one else remove that
        if (bridge[board[a][b]] == 0)
        {
            for (int k = 0; k < V.size(); k += 2)
                board[V[k]][V[k + 1]] = 1;

            OldBridge[board[a][b]] = 0;
        }
        else
        {
            for (int k = 0; k < V.size(); k += 2)
                board[V[k]][V[k + 1]] = 7;

            OldBridge[board[a][b]] = 1;
        }
        old_Bridge = board[a][b];
        return 0;
    }

    if (OldBridge[old_Bridge] == 0)
    {
        bridge[old_Bridge] = 1;
    }
    else
    {
        bridge[old_Bridge] = 0;
    }
    return 0;
}

void Viewer()
{
    switch (views)
    {

    case 0:
        perspective = 0;
        eye = glm::vec3(2, 3, 4);
        target = glm::vec3(0, 0, 0);
        break;
    case 1:
        perspective = 0;
        eye = glm::vec3(1, 10, 4);
        target = glm::vec3(1, 0, 0);
        break;
    case 2:

        perspective = 1;
        eye = glm::vec3(1, 2, 3);
        target = glm::vec3(0, 0, 0);
        break;
    case 3:

        perspective = 1;
        eye = glm::vec3(3, 2, 1);
        target = glm::vec3(0, 0, 0);
        break;

    default:

        break;
    }
}
void reset()
{
    map<int, vector<int>>::iterator it;
    for (it = bridge_coords_Map.begin(); it != bridge_coords_Map.end(); it++)
    {
        vector<int> V = bridge_coords_Map[it->first];
        for (int i = 0; i < board_dimension; i++)
        {
            for (int j = 0; j < board_dimension; j++)
            {
                if (board[i][j] == it->first)
                {
                    // in reset we are remooving bridges as well
                    for (int k = 0; k < 4; k += 2)
                        board[V[k]][V[k + 1]] = 7;
                    bridge[it->first] = 0;
                    OldBridge[it->first] = 1;
                }
            }
        }
    }
    Block_Moves = 0;
    theta = 0.0f;
    direction = 5;
    present_Block_State = future_Block_State = 0;
    Block.coord_y = 0.0f;
    Block.coord_x = 0.0f;
    Block.coord_z = 0.0f;
    Block.Inverse_rotation();
    Block.Inverse_translation();
    Block.rotation();
    Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z - 1);
}

void draw(GLFWwindow *window, float x, float y, float w, float h)
{
    int fbwidth, fbheight;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport((int)(x * fbwidth), (int)(y * fbheight), (int)(w * fbwidth), (int)(h * fbheight));

    glUseProgram(Program_ID);

    Viewer();

    glm::vec3 view_up(0, 1, 0);
    Matrices.view = glm::lookAt(eye, target, view_up);
    glm::mat4 VP = (perspective ? Matrices.Perspective_Projection : Matrices.Orthographic_Projection) * Matrices.view;

    glm::mat4 MVP;

    compute_score(0, 4, 0, Block_Moves);

    Matrices.model = glm::mat4(1.0f);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    if (begin_Stage)
    {
        create_Board_Block();
    }

    Draw_Floor_Board();

    if (!begin_Stage && Block.coord_y > 0.1)
    {
        Block.coord_y -= 0.1f;
    }

    roll_Block();

    Block.render();

    switch (chk_Block_State())
    {

    case 0:
        // Block.render();
        break;

    case 1:
        if (!begin_Stage)
            Collapse_Board_Cells();
        else
            reset();
        break;

    case 2:
        if (!begin_Stage)
            Collapse_Board_Cells();
        else
        {
            reset();
            next_stage();
            system("mpg123 -n 30 -i -q bonus.mp3 &");
        }
        break;

    default:
        break;
    }
}

GLFWwindow *initGLFW(int width, int height)
{
    GLFWwindow *window;
    glfwSetErrorCallback(print_error);
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

    window = glfwCreateWindow(width, height, "Bloxorz", NULL, NULL);

    if (!window)
    {
        exit(EXIT_FAILURE);
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetWindowCloseCallback(window, quit);
    glfwSetWindowTitle(window, "Bloxorz Game");
    glfwSetKeyCallback(window, keyboard_Action);
    // general keyboard input
    glfwSetCharCallback(window, keyboardChar);

    return window;
}

void init_block_and_board(GLFWwindow *window, int width, int height)
{
    coord_x = 0.0f;
    coord_y = 0.0f;
    coord_z = 0.0f;

    // Creating a Block of size 0.6f, 0.3f
    create_graphical_object temp = create_graphical_object(coord_x, coord_y, coord_z, 0.6f, 0.3f);
    temp.object = initialize_cell(0.3f, 0.3f, 0.6f, Blue);
    Block = temp;
    // Block animation at the start
    Block.translation(Block.coord_x - 1, Block.coord_y, Block.coord_z - 1);

    // BOARD
    // initialising with stage 1 board values
    for (int i = 0; i < board_dimension; i++)
        for (int j = 0; j < board_dimension; j++)
        {
            board[i][j] = stage1[i][j];
        }

    for (int i = 0; i < board_dimension; i++)
    {
        VAO *cell;
        create_graphical_object temp;
        coord_x = 0.0f;
        for (int j = 0; j < board_dimension; j++)
        {
            coord_y = 0.0f;
            // creating cell with different colours based on its function using the board[i][j] value
            if (board[i][j] == 2)
            {
                cell = initialize_cell(0.3f, 0.3f, -0.1f, Yellow);
                temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'r');
            }
            else if (board[i][j] == 1)
            {
                if ((i + j) % 2 == 0)
                {
                    // initialise cell creates a VAO handle for the dimensions and colour you have inputted
                    cell = initialize_cell(0.3f, 0.3f, -0.1f, White);
                    // create_graphical_object takes VAO handle for drawing and cordinates and height, length and colour
                    // and have functionalities like rotation, transltion, render etc which are used to know where to draw
                    temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'r');
                }
                else
                {
                    cell = initialize_cell(0.3f, 0.3f, -0.1f, Grey);
                    temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'g');
                }
            }
            else if (board[i][j] == 3)
            {
                if ((i + j) % 2 == 0)
                {
                    cell = initialize_cell(0.3f, 0.3f, -0.1f, Orange);
                    temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'b');
                }
                else
                {
                    cell = initialize_cell(0.3f, 0.3f, -0.1f, Orange);
                    temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'b');
                }
            }
            else
            {
                cell = initialize_cell(0.3f, 0.3f, -0.1f, Green);
                temp = create_graphical_object(coord_x, coord_y, coord_z, 0.1f, 0.3f, 'b');
            }
            // assigning the created VAO to graphical object
            temp.object = cell;
            Board[i][j] = temp;
            coord_x += 0.3f;
        }
        coord_z += 0.3f;
    }
    bridgeConstruct();

    // Loading vertex shaders and fragment shaders
    Program_ID = LoadShaders("Sample_GL.vert", "Sample_GL.frag");

    Matrices.MatrixID = glGetUniformLocation(Program_ID, "MVP");
    window_resize(window, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A

    //  Enabling GL depth test for visible surface detection
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

int main(int argc, char **argv)
{
    // Defining initial dimensions of our window
    int s1 = 1150; // width
    int s2 = 1150; // height

    window = initGLFW(s1, s2);
    initGLEW();
    init_block_and_board(window, s1, s2);
    // while window is not closed we are executing the draw finction
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw(window, 0, 0, 1, 1);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
