
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <myprogram.hpp>

struct TestQuad
{
    Program program;
    GLuint vao;
    GLuint vbo;
    GLuint testTexture;

    const float testQuad[24] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    const std::string vShaderText = R"(
        #version 410 core

        layout(location = 0) in vec2 in_Position;
        layout(location = 1) in vec2 in_TexCoord;

        out vec2 texCoord;

        void main()
        {
            gl_Position = vec4(in_Position, 0, 1);
            texCoord = in_TexCoord;
        }
    )";

    const std::string fShaderText = R"(
        #version 410 core

        uniform sampler2D tex;

        in vec2 texCoord;

        out vec4 fragColor;

        void main()
        {
            fragColor = texture(tex, texCoord);
            // fragColor = vec4(1,0,0,1);
        }
    )";

    void init(GLFWwindow *window, GLuint texture)
    {
        testTexture = texture;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        program.loadShaderByText(vShaderText, fShaderText);
        program.linkShader();

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), testQuad, GL_STATIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, 0, 4 * sizeof(float), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, 0, 4 * sizeof(float), (const void *)(2 * sizeof(float)));
    }

    void render(GLFWwindow *window)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(program.programID);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0, 0, 0, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        GLuint textureLoc = glGetUniformLocation(program.programID, "tex");
        glUniform1i(textureLoc, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, testTexture);

        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};