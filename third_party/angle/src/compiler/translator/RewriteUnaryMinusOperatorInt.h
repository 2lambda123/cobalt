// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This mutating tree traversal works around a bug on evaluating unary
// integer variable on Intel D3D driver. It works by rewriting -(int) to
// ~(int) + 1 when evaluating unary integer variables.

#ifndef COMPILER_TRANSLATOR_REWRITEUNARYMINUSOPERATORINT_H_
#define COMPILER_TRANSLATOR_REWRITEUNARYMINUSOPERATORINT_H_

class TIntermNode;
namespace sh
{

void RewriteUnaryMinusOperatorInt(TIntermNode *root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_REWRITEUNARYMINUSOPERATORINT_H_