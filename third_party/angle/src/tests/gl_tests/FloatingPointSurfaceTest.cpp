//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FloatingPointSurfaceTest.cpp : Test functionality of the EGL_EXT_pixel_format_float extension.

#include "test_utils/ANGLETest.h"

using namespace angle;

class FloatingPointSurfaceTest : public ANGLETest
{
  protected:
    FloatingPointSurfaceTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(16);
        setConfigGreenBits(16);
        setConfigBlueBits(16);
        setConfigAlphaBits(16);
        setConfigComponentType(EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        const std::string vsSource =
            "precision highp float;\n"
            "attribute vec4 position;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = position;\n"
            "}\n";

        const std::string fsSource =
            "precision highp float;\n"
            "void main()\n"
            "{\n"
            "   gl_FragColor = vec4(1.0, 2.0, 3.0, 4.0);\n"
            "}\n";

        mProgram = CompileProgram(vsSource, fsSource);
        ASSERT_NE(0u, mProgram) << "shader compilation failed.";

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

// Test clearing and checking the color is correct
TEST_P(FloatingPointSurfaceTest, Clearing)
{
    GLColor32F clearColor(0.0f, 1.0f, 2.0f, 3.0f);
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR32F_EQ(0, 0, clearColor);
}

// Test drawing and checking the color is correct
TEST_P(FloatingPointSurfaceTest, Drawing)
{
    glUseProgram(mProgram);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_32F_EQ(0, 0, 1.0f, 2.0f, 3.0f, 4.0f);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(FloatingPointSurfaceTest,
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_D3D11(EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE));
