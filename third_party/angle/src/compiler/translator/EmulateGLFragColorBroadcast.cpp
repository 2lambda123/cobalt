//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl_FragColor needs to broadcast to all color buffers in ES2 if
// GL_EXT_draw_buffers is explicitly enabled in a fragment shader.
//
// We emulate this by replacing all gl_FragColor with gl_FragData[0], and in the end
// of main() function, assigning gl_FragData[1], ..., gl_FragData[maxDrawBuffers-1]
// with gl_FragData[0].
//

#include "compiler/translator/EmulateGLFragColorBroadcast.h"

#include "compiler/translator/FindMain.h"
#include "compiler/translator/IntermNode.h"

namespace sh
{

namespace
{

class GLFragColorBroadcastTraverser : public TIntermTraverser
{
  public:
    GLFragColorBroadcastTraverser(int maxDrawBuffers)
        : TIntermTraverser(true, false, false),
          mGLFragColorUsed(false),
          mMaxDrawBuffers(maxDrawBuffers)
    {
    }

    void broadcastGLFragColor(TIntermBlock *root);

    bool isGLFragColorUsed() const { return mGLFragColorUsed; }

  protected:
    void visitSymbol(TIntermSymbol *node) override;

    TIntermBinary *constructGLFragDataNode(int index) const;
    TIntermBinary *constructGLFragDataAssignNode(int index) const;

  private:
    bool mGLFragColorUsed;
    int mMaxDrawBuffers;
};

TIntermBinary *GLFragColorBroadcastTraverser::constructGLFragDataNode(int index) const
{
    TType gl_FragDataType = TType(EbtFloat, EbpMedium, EvqFragData, 4);
    gl_FragDataType.setArraySize(mMaxDrawBuffers);

    TIntermSymbol *symbol   = new TIntermSymbol(0, "gl_FragData", gl_FragDataType);
    TIntermTyped *indexNode = TIntermTyped::CreateIndexNode(index);

    TIntermBinary *binary = new TIntermBinary(EOpIndexDirect, symbol, indexNode);
    return binary;
}

TIntermBinary *GLFragColorBroadcastTraverser::constructGLFragDataAssignNode(int index) const
{
    TIntermTyped *fragDataIndex = constructGLFragDataNode(index);
    TIntermTyped *fragDataZero  = constructGLFragDataNode(0);

    return new TIntermBinary(EOpAssign, fragDataIndex, fragDataZero);
}

void GLFragColorBroadcastTraverser::visitSymbol(TIntermSymbol *node)
{
    if (node->getSymbol() == "gl_FragColor")
    {
        queueReplacement(node, constructGLFragDataNode(0), OriginalNode::IS_DROPPED);
        mGLFragColorUsed = true;
    }
}

void GLFragColorBroadcastTraverser::broadcastGLFragColor(TIntermBlock *root)
{
    ASSERT(mMaxDrawBuffers > 1);
    if (!mGLFragColorUsed)
    {
        return;
    }
    TIntermSequence *mainSequence = FindMain(root)->getBody()->getSequence();
    // Now insert statements
    //   gl_FragData[1] = gl_FragData[0];
    //   ...
    //   gl_FragData[maxDrawBuffers - 1] = gl_FragData[0];
    for (int colorIndex = 1; colorIndex < mMaxDrawBuffers; ++colorIndex)
    {
        mainSequence->insert(mainSequence->end(), constructGLFragDataAssignNode(colorIndex));
    }
}

}  // namespace anonymous

void EmulateGLFragColorBroadcast(TIntermBlock *root,
                                 int maxDrawBuffers,
                                 std::vector<sh::OutputVariable> *outputVariables)
{
    ASSERT(maxDrawBuffers > 1);
    GLFragColorBroadcastTraverser traverser(maxDrawBuffers);
    root->traverse(&traverser);
    if (traverser.isGLFragColorUsed())
    {
        traverser.updateTree();
        traverser.broadcastGLFragColor(root);
        for (auto &var : *outputVariables)
        {
            if (var.name == "gl_FragColor")
            {
                // TODO(zmo): Find a way to keep the original variable information.
                var.name       = "gl_FragData";
                var.mappedName = "gl_FragData";
                var.arraySize  = maxDrawBuffers;
            }
        }
    }
}

}  // namespace sh
