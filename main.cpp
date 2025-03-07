#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <objreader.hpp>
#include <myprogram.hpp>
#include <testquad.hpp>

#include <iostream>

GLFWwindow *window;

Program program;

ObjData obj;

GLuint vao;
GLuint positionvbo, normalvbo;
GLuint elementvbo;

GLuint fbo;
GLuint texture;
GLuint renderBufferobject;

// test

TestQuad tq;

// callbacks

const float PI = 3.1415926535f;
float fovy = 45 * PI / 180.0f;

namespace comparator
{
    float max(const float &a, const float &b)
    {
        return a > b ? a : b;
    }
    float min(const float &a, const float &b)
    {
        return a > b ? b : a;
    }
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    fovy -= yoffset / 10;
    fovy = comparator::max(0.01f, comparator::min(fovy, PI - 0.01f));
}

float cameraTheta, cameraPhi;

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    static double lastX = 0;
    static double lastY = 0;
    // when left mouse button clicked
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
    {
        double dx = xpos - lastX;
        double dy = ypos - lastY;
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        // rotate 180 degree per each width/height dragging
        cameraTheta -= dx / w * PI; // related with y-axis rotation
        cameraPhi -= dy / h * PI;   // related with x-axis rotation
        cameraPhi = comparator::max(-PI / 2 + 0.01f, comparator::min(cameraPhi, PI / 2 - 0.01f));
        // printf("%.3f %.3f\n", cameraTheta, cameraPhi);
    }
    // whenever, save current cursor position as previous one
    lastX = xpos;
    lastY = ypos;
}

void init(GLFWwindow *window)
{
    obj.loadObject("obj", "teapot.obj");
    program.loadShader("edgedetection.vert", "edgedetection.frag");
    program.linkShader();

    // pass #1

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &positionvbo);
    glBindBuffer(GL_ARRAY_BUFFER, positionvbo);
    glBufferData(GL_ARRAY_BUFFER, obj.nVertices * sizeof(glm::vec3), obj.vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glGenBuffers(1, &normalvbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalvbo);
    glBufferData(GL_ARRAY_BUFFER, obj.nSyncedNormals * sizeof(glm::vec3), obj.syncedNormals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glGenBuffers(1, &elementvbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementvbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.nElements3 * sizeof(glm::u16vec3), obj.elements3.data(), GL_STATIC_DRAW);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, texture);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // To enable depth test on texture.
    glGenRenderbuffers(1, &renderBufferobject);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferobject);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferobject);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // test quads
    tq.init(window, texture);
}

glm::vec3 lightPosition(10, 10, 5);
glm::vec3 lightColor(140);
glm::vec3 diffuseColor(1, 1, 1);
glm::vec3 specularColor(0.33, 0.33, 0.33);
float shininess = 12;

void render(GLFWwindow *window)
{
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    // pass #1

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(program.programID);

    glViewport(0, 0, w, h);

    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 mm = glm::translate(glm::vec3(0, -1.4, 0));
    glm::vec3 eye(0, 0, 5);
    glm::mat4 rotateX = glm::rotate(cameraTheta, glm::vec3(0, 1, 0));
    glm::mat4 rotateY = glm::rotate(cameraPhi, glm::vec3(1, 0, 0));
    glm::vec3 eyePosition = rotateX * rotateY * glm::vec4(eye, 1);
    glm::mat4 vw = glm::lookAt(eyePosition, glm::vec3(0), glm::vec3(0, 1, 0));
    glm::mat4 pj = glm::perspective(55 * 3.141592f / 180.0f, w / (float)h, 0.0001f, 1000.0f);
    GLuint mmLoc = glGetUniformLocation(program.programID, "modelMat");
    GLuint vwLoc = glGetUniformLocation(program.programID, "viewMat");
    GLuint pjLoc = glGetUniformLocation(program.programID, "projMat");
    glUniformMatrix4fv(mmLoc, 1, GL_FALSE, glm::value_ptr(mm));
    glUniformMatrix4fv(vwLoc, 1, GL_FALSE, glm::value_ptr(vw));
    glUniformMatrix4fv(pjLoc, 1, GL_FALSE, glm::value_ptr(pj));

    GLuint index = glGetSubroutineUniformLocation(program.programID, GL_FRAGMENT_SHADER, "renderPass");
    if (index == -1)
    {
        std::cout << "Subroutine indexing error" << std::endl;
        return;
    }

    GLuint pass1 = glGetSubroutineIndex(program.programID, GL_FRAGMENT_SHADER, "pass1");
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1);

    GLuint lightPositionLoc = glGetUniformLocation(program.programID, "lightPosition");
    GLuint eyePositionLoc = glGetUniformLocation(program.programID, "eyePosition");
    GLuint lightColorLoc = glGetUniformLocation(program.programID, "lightColor");
    GLuint diffuseColorLoc = glGetUniformLocation(program.programID, "diffuseColor");
    GLuint specularColorLoc = glGetUniformLocation(program.programID, "specularColor");
    GLuint shininessLoc = glGetUniformLocation(program.programID, "shininess");

    glUniform3fv(lightPositionLoc, 1, glm::value_ptr(lightPosition));
    glUniform3fv(eyePositionLoc, 1, glm::value_ptr(eyePosition));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform3fv(diffuseColorLoc, 1, glm::value_ptr(diffuseColor));
    glUniform3fv(specularColorLoc, 1, glm::value_ptr(specularColor));
    glUniform1f(shininessLoc, shininess);

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementvbo);

    glDrawElements(GL_TRIANGLES, obj.nElements3 * 3, GL_UNSIGNED_SHORT, 0);

    // pass #2

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, w, h);

    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mmLoc = glGetUniformLocation(program.programID, "modelMat");
    vwLoc = glGetUniformLocation(program.programID, "viewMat");
    pjLoc = glGetUniformLocation(program.programID, "projMat");
    glUniformMatrix4fv(mmLoc, 1, GL_FALSE, glm::value_ptr(mm));
    glUniformMatrix4fv(vwLoc, 1, GL_FALSE, glm::value_ptr(vw));
    glUniformMatrix4fv(pjLoc, 1, GL_FALSE, glm::value_ptr(pj));

    GLuint pass2 = glGetSubroutineIndex(program.programID, GL_FRAGMENT_SHADER, "pass2");
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2);

    GLuint renderTex = glGetUniformLocation(program.programID, "renderTex");
    glUniform1i(renderTex, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLuint edgeThresholdLoc = glGetUniformLocation(program.programID, "edgeThreshold");
    glUniform1f(edgeThresholdLoc, 0.01);
    GLuint widthLoc = glGetUniformLocation(program.programID, "width");
    glUniform1i(widthLoc, w);
    GLuint heightLoc = glGetUniformLocation(program.programID, "height");
    glUniform1i(heightLoc, h);

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementvbo);

    glDrawElements(GL_TRIANGLES, obj.nElements3 * 3, GL_UNSIGNED_SHORT, 0);

    // tq.render(window);
    glfwSwapBuffers(window);
}

// main //

int main()
{

    if (!glfwInit())
    {
        std::cout << "GLFW Init Error" << std::endl;
        return -1;
    }

// If MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    GLFWwindow *window = glfwCreateWindow(640, 480, "Edge Detection", 0, 0);
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW Init Error" << std::endl;
        return -1;
    }

    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    init(window);

    // render

    while (!glfwWindowShouldClose(window))
    {
        render(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}