#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <functional>
#include "format.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#define GLM_FORCE_RADIANS

using namespace rapidjson;
using std::vector;
using std::string;
using std::ifstream;
using glm::vec3;

int currentFile = 0;

vector<float> positions;
vector<float> colors;
vector<float> radius;

int i=0;
string fileFolder = ".";
string filePrefix = "output";
string fileSuffix = ".data";

GLFWwindow *window;

GLuint vao;
GLuint position_vbo;
GLuint radius_vbo;
GLuint color_vbo;
GLuint shader_program;
GLuint line_shader_program;
GLuint camera_ubo;

glm::vec3 cameraPosition;
glm::quat cameraRotation;

bool initialized;
bool draw_lines;

bool camera_move_right {false};
bool camera_move_left {false};
bool camera_move_forward {false};
bool camera_move_back {false};
bool camera_locked {true};

void parseLine(const std::string &s);
bool tryParseFile(string fileName);
void rebindBuffers();

static void OnKey(GLFWwindow *window, int key, int scancode, int action, int mods);
static void OnCursorPositionChanged(GLFWwindow *glfwWindow, double xPos, double yPos);
static void OnMouseButton(GLFWwindow *glfwWindow, int button, int action, int mods);

void createLineShaderProgram();
void createParticleShaderProgram();
void updatePerspective(float ratio);
void updateView();

void moveCamera(float deltaTime);

glm::mat4 getCameraTransform();

using std::cout;
using std::endl;

int main(int argc, char** argv)
{
    if(argc > 1)
    {
        fileFolder = std::string(argv[1]);
        cout<<fileFolder<<endl;
    }
    Document config;
    std::ifstream in("config.json");
    std::string contents((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    const char* conf = contents.c_str();
    config.Parse(conf);
    Value val;
    int count = config["count"].GetInt();
    std::string dist = config["distribution"].GetString();
    float M = config["M"].GetDouble();
    float N = config["N"].GetDouble();
    float kB = config["kB"].GetDouble();
    float K = config["K"].GetDouble();
    float n = config["n"].GetDouble();
    float z = config["z"].GetDouble();
    float h = config["h"].GetDouble();
    float T0 = config["T0"].GetDouble();
    float totalTime = config["totalTime"].GetDouble();
    float timeStep = config["timeStep"].GetDouble();
    glm::vec3 ldf;
    glm::vec3 rub;
    val = config["ldf"];
    for(int i=0; i<val.Size(); i++)
    {
        ldf[i]= (float) val[i].GetDouble();
    }
    val = config["rub"];
    for(int i=0; i<val.Size(); i++)
    {
        rub[i]= (float) val[i].GetDouble();
    }
        
    initialized = false;
    cameraRotation = glm::normalize(cameraRotation);
    glfwInit();
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int width = 1600, height = 900;
    window = glfwCreateWindow(width, height, "Particle visualization", NULL, NULL);

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.1f, 0.5f, 1.0f, 1);
    glfwSetKeyCallback(window, OnKey);
    glfwSetMouseButtonCallback(window, OnMouseButton);
    glfwSetCursorPosCallback(window, OnCursorPositionChanged);

    createParticleShaderProgram();
    createLineShaderProgram();
    
    updateView();
    updatePerspective((float)width / height);

    cameraPosition = glm::vec3(1.5, 0.5f, 6);
    cameraRotation = glm::angleAxis(0.0f, glm::vec3(0, 1, 0));
    cameraRotation = glm::normalize(glm::angleAxis((float)M_PI, glm::vec3(0, 1, 0))) * cameraRotation;
    glGenBuffers(1, &camera_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, camera_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, camera_ubo, 0, sizeof(glm::mat4) * 2);
    auto uniformBlockIndex = glGetUniformBlockIndex(shader_program, "Camera");
    auto lineUniformBlockIndex = glGetUniformBlockIndex(line_shader_program, "Camera");
    glUniformBlockBinding(shader_program, uniformBlockIndex, 0);
    glUniformBlockBinding(line_shader_program, lineUniformBlockIndex, 0);

    
    
    vector<glm::vec3> line_points;
    for(float i=ldf.x; i<= rub.x; i=std::min(i+h, rub.x))
    {
        for(float j=ldf.y; j<= rub.y; j=std::min(j+h, rub.y))
        {
            line_points.push_back(vec3(i, j, ldf.z));
            line_points.push_back(vec3(i, j, rub.z));
            if(j==rub.y) break;
        }
        if(i==rub.x) break;
    }
    for(float i=ldf.y; i<=rub.y; i=std::min(i+h, rub.y))
    {
        for(float j=ldf.z;j<=rub.z; j=std::min(j+h, rub.z))
        {
            line_points.push_back(vec3(ldf.x, i, j));
            line_points.push_back(vec3(rub.x, i, j));
            if(j==rub.z) break;
        }
        if(i==rub.y) break;
    }
    for(float i=ldf.x; i<=rub.x; i=std::min(i+h, rub.x))
    {
        for(float j=ldf.z;j<=rub.z; j=std::min(j+h, rub.z))
        {
            line_points.push_back(vec3(i, ldf.y, j));
            line_points.push_back(vec3(i, rub.y, j));
            if(j==rub.z) break;
        }
        if(i==rub.x) break;
    }

    GLuint vaolines, vbolines;
    glGenVertexArrays(1, &vaolines);
    glBindVertexArray (vaolines);
    glGenBuffers (1, &vbolines);

    glBindBuffer(GL_ARRAY_BUFFER, vbolines);
    glBufferData (
        GL_ARRAY_BUFFER,
        line_points.size() * 3 * sizeof (GLfloat),
        &line_points[0],
        GL_STATIC_DRAW
        );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

   

    double previousTime = glfwGetTime();
    double currentTime;
    double deltaTime;
    while (!glfwWindowShouldClose(window))
    {
        currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        if(deltaTime > 0.03) {
            previousTime = currentTime;
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);

            moveCamera(deltaTime);
            updateView();
            updatePerspective((float) width / height);

            if (initialized) {
                glUseProgram(shader_program);
                glBindVertexArray(vao);
                glDrawArrays(GL_POINTS, 0, positions.size() / 3);

            }

            if(draw_lines)
            {
                
                glUseProgram(line_shader_program);
                glBindVertexArray(vaolines);
                glDrawArrays(GL_LINES, 0, line_points.size());
            }
            glfwSwapBuffers(window);
        }
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

glm::mat4 getCameraTransform()
{
    return glm::translate(glm::mat4(1), cameraPosition) * glm::toMat4(cameraRotation);
}
glm::vec3 getCameraRight()
{
    auto right = glm::normalize(glm::vec3(cameraRotation * glm::vec4(-1, 0, 0, 0)));
    return right;
}

glm::vec3 getCameraForward()
{
    return glm::normalize(glm::vec3(cameraRotation * glm::vec4(0, 0, 1, 0)));
}


glm::vec3 getCameraUp()
{
    return glm::normalize(glm::vec3(cameraRotation * glm::vec4(0, 1, 0, 0)));
}

void moveCameraBy(glm::vec3 vec)
{
    cameraPosition += vec;
}

void moveCamera(float deltaTime) {
    float factor = 2;
    if(camera_move_forward)
        moveCameraBy(factor*deltaTime*getCameraForward());
    if(camera_move_back)
        moveCameraBy(-factor*deltaTime*getCameraForward());
    if(camera_move_right)
        moveCameraBy(factor*deltaTime*getCameraRight());
    if(camera_move_left)
        moveCameraBy(-factor*deltaTime*getCameraRight());
}

void updateView()
{
    glm::vec3 up = getCameraUp();
    glm::vec3 forward = getCameraForward();
    glm::mat4 worldToCamera = glm::lookAt(cameraPosition, cameraPosition + forward, up);

    glBindBuffer(GL_UNIFORM_BUFFER, camera_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(worldToCamera));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void updatePerspective(float ratio)
{
    glm::mat4 cameraToPerspective = (glm::mat4)glm::perspective(3.1415f/4, ratio, 0.1f, 100.0f);

    glBindBuffer(GL_UNIFORM_BUFFER, camera_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cameraToPerspective));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
std::string getFileContents(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return(contents.str());
    }
    throw(errno);
}
void GetShaderCompileLog(GLuint shader)
{
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<char> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
        std::cout<<&errorLog[0];
    }
}

void GetProgramLinkLog(GLuint program)
{
    int isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
        std::cout<<&infoLog[0];
    }
}

void createLineShaderProgram()
{
    auto vert = getFileContents("shaders/lineshader_vert.glsl");
    auto frag= getFileContents("shaders/lineshader_frag.glsl");
    const char* lineVertexShader = vert.c_str();
    const char* lineFragmentShader = frag.c_str();

    GLuint wvs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (wvs, 1, &lineVertexShader, NULL);
    glCompileShader (wvs);
    GetShaderCompileLog(wvs);

    GLuint wfs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (wfs, 1, &lineFragmentShader, NULL);
    glCompileShader (wfs);
    GetShaderCompileLog(wfs);

    line_shader_program = glCreateProgram ();
    glAttachShader (line_shader_program, wfs);
    glAttachShader (line_shader_program, wvs);
    glLinkProgram (line_shader_program);
    GetProgramLinkLog(line_shader_program);

}

void createParticleShaderProgram()
{
    auto vert = getFileContents("shaders/particleshader_vert.glsl");
    auto frag= getFileContents("shaders/particleshader_frag.glsl");
    auto geom = getFileContents("shaders/particleshader_geom.glsl");
    const char* waterVertexShader = vert.c_str();
    const char* waterFragmentShader = frag.c_str();
    const char* waterGeometryShader = geom.c_str();

    GLuint wvs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (wvs, 1, &waterVertexShader, NULL);
    glCompileShader (wvs);
    GetShaderCompileLog(wvs);

    GLuint wgs = glCreateShader (GL_GEOMETRY_SHADER);
    glShaderSource (wgs, 1, &waterGeometryShader, NULL);
    glCompileShader (wgs);
    GetShaderCompileLog(wgs);

    GLuint wfs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (wfs, 1, &waterFragmentShader, NULL);
    glCompileShader (wfs);
    GetShaderCompileLog(wfs);

    shader_program = glCreateProgram ();
    glAttachShader (shader_program, wfs);
    glAttachShader (shader_program, wgs);
    glAttachShader (shader_program, wvs);
    glLinkProgram (shader_program);
    GetProgramLinkLog(shader_program);
}

void rebindBuffers()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray (vao);
    glGenBuffers (1, &position_vbo);
    glGenBuffers (1, &radius_vbo);
    glGenBuffers (1, &color_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glBufferData (
        GL_ARRAY_BUFFER,
        positions.size() * sizeof (GLfloat),
        &positions[0],
        GL_DYNAMIC_DRAW
        );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glBufferData (
        GL_ARRAY_BUFFER,
        colors.size() * sizeof (GLfloat),
        &colors[0],
        GL_DYNAMIC_DRAW
        );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, radius_vbo);
    glBufferData (
        GL_ARRAY_BUFFER,
        radius.size() * sizeof (GLfloat),
        &radius[0],
        GL_DYNAMIC_DRAW
        );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
}

void parseLine(const std::string &s) {
    std::stringstream ss(s);
    std::string item;
    for(int i=0;i<4;i++)
    {
        std::getline(ss, item, ' ');
        if(i<3)
            positions.push_back(atof(item.c_str()));
        if(i==3) {
            int color = atoi(item.c_str());
            float r = ((color & 0x00ff0000) >> 16) / 255.0f;
            float g = ((color & 0x0000ff00) >> 8) / 255.0f;
            float b =  (color & 0x000000ff) / 255.0f;
            colors.push_back(r);
            colors.push_back(g);
            colors.push_back(b);
            radius.push_back(0.04f);
        }
    }
}

bool tryParseFile(string fileName)
{
    string line;
    ifstream myfile (fileName);
    if(!myfile.good()) return false;
    positions.clear();
    positions.reserve(1000000);
    colors.clear();
    colors.reserve(1000000);
    radius.clear();
    radius.reserve(333334);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            parseLine(line);
        }
        myfile.close();
    }
    return true;

}


static void OnKey(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if(tryParseFile(fmt::format("{0}/{1}{2}{3}",fileFolder, filePrefix, currentFile, fileSuffix)))
        {
            /*positions = {-1.0f, -1.0f, 0.0f,
              1.0f, -1.0f, 0.0f,
              0.0f,  1.0f, 0.0f,};
              colors = {255, 255, 255};
              radius = {0.0f, 0.0f, 0.0f};*/
            rebindBuffers();

            initialized = true;
            glfwSetWindowTitle(window, fmt::format("{0}{1}{2}: {3}", filePrefix, currentFile, fileSuffix, positions.size()).c_str());
        }
    }
    else if(key==GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        if(tryParseFile(fmt::format("{0}/{1}{2}{3}", fileFolder, filePrefix, currentFile-1, fileSuffix)))
        {
            currentFile--;
            rebindBuffers();
            glfwSetWindowTitle(window, fmt::format("{0}{1}{2}: {3}", filePrefix, currentFile, fileSuffix, positions.size()).c_str());
        }
    }
    else if(key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        if(tryParseFile(fmt::format("{0}/{1}{2}{3}", fileFolder, filePrefix, currentFile+1, fileSuffix)))
        {
            currentFile++;
            rebindBuffers();
            glfwSetWindowTitle(window, fmt::format("{0}{1}{2}: {3}", filePrefix, currentFile, fileSuffix, positions.size()).c_str());
        }
    }
    else if(key == GLFW_KEY_W)
    {
        if(action == GLFW_PRESS)
        {
            camera_move_forward = true;
        }
        else if(action == GLFW_RELEASE)
        {
            camera_move_forward = false;
        }
    }
    else if(key == GLFW_KEY_S)
    {
        if(action == GLFW_PRESS)
        {
            camera_move_back = true;
        }
        else if(action == GLFW_RELEASE)
        {
            camera_move_back = false;
        }
    }
    else if(key == GLFW_KEY_A)
    {
        if(action == GLFW_PRESS)
        {
            camera_move_left = true;
        }
        else if(action == GLFW_RELEASE)
        {
            camera_move_left = false;
        }
    }
    else if(key == GLFW_KEY_D)
    {
        if(action == GLFW_PRESS)
        {
            camera_move_right = true;
        }
        else if(action == GLFW_RELEASE)
        {
            camera_move_right = false;
        }
    }
    else if(key == GLFW_KEY_L)
    {
        if(action == GLFW_PRESS)
            draw_lines = !draw_lines;
    }
}

static void OnMouseButton(GLFWwindow *glfwWindow, int button, int action, int mods) {

    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        if(action == GLFW_PRESS) {
            camera_locked = false;
        } else if(action == GLFW_RELEASE) {
            camera_locked = true;
        }
    }
}

static void OnCursorPositionChanged(GLFWwindow *glfwWindow, double xPos, double yPos) {
    int width, height;
    glfwGetWindowSize(glfwWindow, &width, &height);
    int centerX = width >> 1, centerY = height >> 1;
    //event::IEventDataSPtr eventDataSPtr = std::make_shared<event::OnCursorPositionChanged>(glm::vec2((xPos - centerX)/(width), (centerY - yPos)/(height)));
    glm::vec2 movement = (glm::vec2((xPos - centerX)/(width), (centerY - yPos)/(height)));
    glfwSetCursorPos(glfwWindow, centerX, centerY);
    if(!camera_locked) {
        auto right = getCameraRight();
        cameraRotation = glm::normalize(glm::angleAxis(-movement.x, glm::vec3(0, 1, 0))) * cameraRotation;
        cameraRotation = glm::normalize(glm::angleAxis(movement.y, right)) * cameraRotation;
        updateView();
    }

}
