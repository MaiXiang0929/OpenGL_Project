// SPDX-License-Identifier: MIT
#include "TranslucencyPass.h"

#include "ForwardPass.h"
#include "Renderer/View/RenderView.h"

void TranslucencyPass::Execute(RenderPassContext& context)
{
    m_ForwardPass.BindColorTarget();

    const GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLboolean depthWriteEnabled = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteEnabled);

    GLint blendSourceRgb = GL_ONE;
    GLint blendDestinationRgb = GL_ZERO;
    GLint blendSourceAlpha = GL_ONE;
    GLint blendDestinationAlpha = GL_ZERO;
    GLint blendEquationRgb = GL_FUNC_ADD;
    GLint blendEquationAlpha = GL_FUNC_ADD;
    glGetIntegerv(GL_BLEND_SRC_RGB, &blendSourceRgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &blendDestinationRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSourceAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDestinationAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEquationRgb);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEquationAlpha);

    // 透明表面仍需被不透明深度遮挡，但不能写深度，否则会错误遮挡后续透明层。
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_ForwardPass.RenderTranslucentSurface(context, context.mainView);

    if (!blendEnabled)
        glDisable(GL_BLEND);
    glBlendFuncSeparate(
        blendSourceRgb,
        blendDestinationRgb,
        blendSourceAlpha,
        blendDestinationAlpha);
    glBlendEquationSeparate(blendEquationRgb, blendEquationAlpha);
    if (!depthTestEnabled)
        glDisable(GL_DEPTH_TEST);
    glDepthMask(depthWriteEnabled);

    // 颜色目标至此已包含不透明与透明结果，随后才能生成供 Present 使用的完整 mip 链。
    m_ForwardPass.UnbindColorTarget();
}
