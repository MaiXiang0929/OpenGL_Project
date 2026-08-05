// SPDX-License-Identifier: MIT
/// @file Renderer.cpp
/// @brief 渲染器核心类的实现文件
/// @details 该文件实现了 Renderer 类的核心功能，包括初始化、渲染循环、资源管理等。
/// @author MaiX
/// @date 2026-08-01

#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "lodepng.h"

/// @brief 构造函数
Renderer::Renderer()
{
    
}

/// @brief 析构函数，释放 OpenGL 资源
Renderer::~Renderer()
{
	// 删除纹理对象
    if (m_DiffuseTexture) glDeleteTextures(1, &m_DiffuseTexture);
    if (m_SpecularTexture) glDeleteTextures(1, &m_SpecularTexture);
    if (m_CubemapTexture) glDeleteTextures(1, &m_CubemapTexture);

	// 删除顶点缓冲对象和顶点数组对象

}

/// @brief 初始化 Renderer
void Renderer::Init()
{
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);

	// 加载着色器
    m_MainShader.Load(
        "assets/shaders/triangle.vert",
        "assets/shaders/triangle.frag"
    );

    m_PlaneShader.Load(
        "assets/shaders/plane.vert",
        "assets/shaders/plane.frag"
    );

    m_SkyboxShader.Load(
        "assets/shaders/skybox.vert",
        "assets/shaders/skybox.frag"
    );

    // 最终显示平面位于 XY 平面，由两个逆时针三角形组成，UV 覆盖完整离屏纹理。
    const std::vector<Vertex> planeVertices = {
        { cy::Vec3f(-1.0f, -1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec2f(0.0f, 0.0f) },
        { cy::Vec3f( 1.0f, -1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec2f(1.0f, 0.0f) },
        { cy::Vec3f( 1.0f,  1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec2f(1.0f, 1.0f) },
        { cy::Vec3f(-1.0f, -1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec2f(0.0f, 0.0f) },
        { cy::Vec3f( 1.0f,  1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec2f(1.0f, 1.0f) },
        { cy::Vec3f(-1.0f,  1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec2f(0.0f, 1.0f) }
    };
    m_PlaneMesh.Upload(planeVertices);

    // 天空盒立方体：1x1x1 立方体，36 个顶点（6 个面 × 2 个三角形 × 3 个顶点）
    // 仅需要位置数据，法线和纹理坐标填零
    {
        const float s = 1.0f; // 半边长
        const std::vector<Vertex> cubeVertices = {
            // +X 面 (右)
            {{ s, -s, -s}, {0,0,0}, {0,0}}, {{ s,  s, -s}, {0,0,0}, {0,0}}, {{ s,  s,  s}, {0,0,0}, {0,0}},
            {{ s, -s, -s}, {0,0,0}, {0,0}}, {{ s,  s,  s}, {0,0,0}, {0,0}}, {{ s, -s,  s}, {0,0,0}, {0,0}},
            // -X 面 (左)
            {{-s, -s,  s}, {0,0,0}, {0,0}}, {{-s,  s,  s}, {0,0,0}, {0,0}}, {{-s,  s, -s}, {0,0,0}, {0,0}},
            {{-s, -s,  s}, {0,0,0}, {0,0}}, {{-s,  s, -s}, {0,0,0}, {0,0}}, {{-s, -s, -s}, {0,0,0}, {0,0}},
            // +Y 面 (上) — y-up 约定
            {{ s,  s, -s}, {0,0,0}, {0,0}}, {{-s,  s, -s}, {0,0,0}, {0,0}}, {{-s,  s,  s}, {0,0,0}, {0,0}},
            {{ s,  s, -s}, {0,0,0}, {0,0}}, {{-s,  s,  s}, {0,0,0}, {0,0}}, {{ s,  s,  s}, {0,0,0}, {0,0}},
            // -Y 面 (下)
            {{ s, -s,  s}, {0,0,0}, {0,0}}, {{-s, -s,  s}, {0,0,0}, {0,0}}, {{-s, -s, -s}, {0,0,0}, {0,0}},
            {{ s, -s,  s}, {0,0,0}, {0,0}}, {{-s, -s, -s}, {0,0,0}, {0,0}}, {{ s, -s, -s}, {0,0,0}, {0,0}},
            // +Z 面 (前)
            {{ s, -s,  s}, {0,0,0}, {0,0}}, {{ s,  s,  s}, {0,0,0}, {0,0}}, {{-s,  s,  s}, {0,0,0}, {0,0}},
            {{ s, -s,  s}, {0,0,0}, {0,0}}, {{-s,  s,  s}, {0,0,0}, {0,0}}, {{-s, -s,  s}, {0,0,0}, {0,0}},
            // -Z 面 (后)
            {{-s, -s, -s}, {0,0,0}, {0,0}}, {{-s,  s, -s}, {0,0,0}, {0,0}}, {{ s,  s, -s}, {0,0,0}, {0,0}},
            {{-s, -s, -s}, {0,0,0}, {0,0}}, {{ s,  s, -s}, {0,0,0}, {0,0}}, {{ s, -s, -s}, {0,0,0}, {0,0}},
        };
        m_SkyboxMesh.Upload(cubeVertices);
    }

    // 加载天空盒立方体贴图
    LoadCubemap("assets/models/cubemap");

    // 初始化 Gizmo
    m_LightGizmo.Init(
        "assets/shaders/billboard.vert",
        "assets/shaders/billboard.frag"
    );

    // 加载地面着色器
    m_GroundShader.Load(
        "assets/shaders/ground.vert",
        "assets/shaders/ground.frag"
    );

    // 地面平面：位于 XZ 平面（Y=0），由 model 矩阵定位到场景中
    // 单位大小（-1 到 1），由 model 矩阵的 scale 控制实际大小
    {
        const std::vector<Vertex> groundVertices = {
            {{-1.0f, 0.0f, -1.0f}, {0,1,0}, {0,0}}, {{ 1.0f, 0.0f, -1.0f}, {0,1,0}, {0,0}}, {{ 1.0f, 0.0f,  1.0f}, {0,1,0}, {0,0}},
            {{-1.0f, 0.0f, -1.0f}, {0,1,0}, {0,0}}, {{ 1.0f, 0.0f,  1.0f}, {0,1,0}, {0,0}}, {{-1.0f, 0.0f,  1.0f}, {0,1,0}, {0,0}},
        };
        m_GroundPlaneMesh.Upload(groundVertices);
    }

    // 初始化 1024x1024 分辨率的离屏缓冲
    m_Framebuffer.Init(1024, 1024);
    // 初始化 512x512 分辨率的反射缓冲区
    m_ReflectionFramebuffer.Init(512, 512);
}

/// @brief 开始一帧渲染
void Renderer::BeginFrame(unsigned int viewportWidth, unsigned int viewportHeight)
{
    // 离屏阶段会改变 FBO 和视口，此处显式恢复窗口对应的默认渲染目标。
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, static_cast<GLsizei>(viewportWidth), static_cast<GLsizei>(viewportHeight));
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色与深度缓冲
}

void Renderer::BeginObjectPass()
{
    // Framebuffer::Bind 同时会把视口切换为离屏纹理尺寸。
    m_Framebuffer.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndObjectPass()
{
    m_Framebuffer.Unbind();

    // 物体绘制只更新了纹理第 0 层，缩小过滤所需的其他层级需要重新生成。
    m_Framebuffer.GenerateMipmaps();
}

/// @brief 渲染场景
/// @param mvp 
/// @param mv 
/// @param lightPosView 
void Renderer::RenderScene(const cy::Matrix4f& mvp, const cy::Matrix4f& mv, const cy::Vec3f& lightPosView, const cy::Matrix4f& view)
{
	// 使用着色器程序
	m_MainShader.Bind();

    // 传递矩阵与光照 Uniform
    m_MainShader.SetMatrix4(
        "mvp",
        &mvp.cell[0]
    );

    m_MainShader.SetMatrix4(
        "mv",
        &mv.cell[0]
    );

    m_MainShader.SetVec3(
        "lightPos",
        lightPosView.x,
        lightPosView.y,
        lightPosView.z
    );

    // --- 环境反射：计算 view→world 旋转矩阵（视图矩阵左上 3×3 的转置） ---
    float viewToWorldData[9] = {
        view.cell[0], view.cell[4], view.cell[8],
        view.cell[1], view.cell[5], view.cell[9],
        view.cell[2], view.cell[6], view.cell[10]
    };
    GLint vtLoc = glGetUniformLocation(m_MainShader.GetProgramID(), "viewToWorld");
    if (vtLoc != -1) {
        glUniformMatrix3fv(vtLoc, 1, GL_FALSE, viewToWorldData);
    }

    // 2. 绑定纹理并传递标志位
    if (m_DiffuseTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_DiffuseTexture);

        m_MainShader.SetInt("texDiffuse", 0);

        m_MainShader.SetInt("hasDiffuseMap", 1);
    }
    else {
        m_MainShader.SetInt("hasDiffuseMap", 0);
    }

    if (m_SpecularTexture != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_SpecularTexture);
        m_MainShader.SetInt("texSpecular", 1);
        m_MainShader.SetInt("hasSpecularMap", 1);
    }
    else {
        m_MainShader.SetInt("hasSpecularMap", 0);
    }

    // --- 绑定 Cubemap 到纹理单元 2 用于环境反射 ---
    if (m_CubemapTexture != 0) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTexture);
        m_MainShader.SetInt("cubemap", 2);
    }

	// 绑定 VAO
	// 绘制三角形
    m_Mesh.Draw();
}

void Renderer::RenderPlane(const cy::Matrix4f& mvp)
{
    m_PlaneShader.Bind();
    m_PlaneShader.SetMatrix4("mvp", &mvp.cell[0]);

    // 将离屏颜色附件作为普通二维纹理交给平面片段着色器采样。
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_Framebuffer.GetColorTexture());
    m_PlaneShader.SetInt("renderedTexture", 0);
    m_PlaneMesh.Draw();
}

/// @brief 结束一帧渲染
void Renderer::EndFrame()
{

}

/// @brief 上传模型数据到GPU
/// @param vertices 
/// @param normals 
/// @param texCoords 
#if 0
void Renderer::SetMesh(
    const std::vector<cy::Vec3f>& vertices,
    const std::vector<cy::Vec3f>& normals,
    const std::vector<cy::Vec2f>& texCoords
)
{
	// 创建并绑定 VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

	// 创建 VBO
    glGenBuffers( 3, m_VBO);

    // 数据上传GPU
    // position
    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO[0]
    );
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(cy::Vec3f),
        vertices.data(),
        GL_STATIC_DRAW
    );
    // normal
    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO[1]
    );
    glBufferData(
        GL_ARRAY_BUFFER,
        normals.size() * sizeof(cy::Vec3f),
        normals.data(),
        GL_STATIC_DRAW
    );
    // texcoord
    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO[2]
    );
    glBufferData(
        GL_ARRAY_BUFFER,
        texCoords.size() * sizeof(cy::Vec2f),
        texCoords.data(),
        GL_STATIC_DRAW
    );

    // 动态获取顶点属性在着色器中的入口位置
    GLint posLoc =
        glGetAttribLocation(
            m_MainShader.GetProgramID(),
            "pos"
        );
    GLint normalLoc =
        glGetAttribLocation(
            m_MainShader.GetProgramID(),
            "normal"
        );
    GLint texLoc =
        glGetAttribLocation(
            m_MainShader.GetProgramID(),
            "texCoord"
        );

    // =======================================
    // 安全检查：如果名字拼错或者着色器里没用到该变量，OpenGL 会返回 -1
    // =======================================
    if (posLoc == -1) {
        std::cerr << "[Error] Vertex attribute 'pos' not found in shader." << std::endl;
    }
    if (normalLoc == -1) {
        std::cerr << "[Error] Vertex attribute 'normal' not found in shader." << std::endl;
    }
    if (texLoc == -1) {
        std::cerr << "[Error] Vertex attribute 'texCoord' not found in shader." << std::endl;
    }

    // 告诉 OpenGL 如何解析顶点数据
	// position
    if (posLoc != -1)
    {
        glBindBuffer(
            GL_ARRAY_BUFFER,
            m_VBO[0]
        );

        glVertexAttribPointer(
            posLoc,                     // 属性位置
			3,                          // 属性大小 (vec3)
			GL_FLOAT,                   // 数据类型
			GL_FALSE,                   // 是否归一化
			0,                          // 步长 (0 = 紧密排列)
            (GLvoid*)0                  // 偏移量
        );
		// 启用顶点属性数组
        glEnableVertexAttribArray(posLoc);
    }
	// normal
    if (normalLoc != -1)
    {
        glBindBuffer(
            GL_ARRAY_BUFFER,
            m_VBO[1]
        );

        glVertexAttribPointer(
            normalLoc,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (GLvoid*)0
        );

        glEnableVertexAttribArray(normalLoc);
    }
	// texcoord
    if (texLoc != -1)
    {
        glBindBuffer(
            GL_ARRAY_BUFFER,
            m_VBO[2]
        );

        glVertexAttribPointer(
            texLoc,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            (GLvoid*)0
        );

        glEnableVertexAttribArray(texLoc);
    }

    // 解绑（非必须，为了保持状态机干净）
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_VertexCount =
        static_cast<int>(
            vertices.size()
            );
}




// 内部 PNG 加载逻辑
#endif

void Renderer::SetMesh(const std::vector<Vertex>& vertices)
{
    m_Mesh.Upload(vertices);
}

GLuint Renderer::LoadTexturePNG(const std::string& filePath) {
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, filePath);

    if (error) {
        std::cerr << "[LodePNG Error] " << lodepng_error_text(error) << " File: " << filePath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

// --- 新增：暴露给外部的纹理加载接口 ---
void Renderer::LoadTextures(const std::string& diffusePath, const std::string& specularPath) {
    // 允许重复加载模型：替换纹理前先释放上一组 GPU 资源。
    if (m_DiffuseTexture != 0) glDeleteTextures(1, &m_DiffuseTexture);
    if (m_SpecularTexture != 0) glDeleteTextures(1, &m_SpecularTexture);

    m_DiffuseTexture = diffusePath.empty() ? 0 : LoadTexturePNG(diffusePath);
    m_SpecularTexture = specularPath.empty() ? 0 : LoadTexturePNG(specularPath);
}

// --- 新增：立方体贴图加载 ---
void Renderer::LoadCubemap(const std::string& dirPath) {
    // 六个面的文件名后缀（OpenGL cubemap 枚举顺序）
    const std::string faceNames[6] = {
        "posx", "negx",
        "posy", "negy",
        "posz", "negz"
    };

    glGenTextures(1, &m_CubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTexture);

    for (int i = 0; i < 6; ++i) {
        const std::string filePath = dirPath + "/cubemap_" + faceNames[i] + ".png";

        std::vector<unsigned char> image;
        unsigned width, height;
        unsigned error = lodepng::decode(image, width, height, filePath);

        if (error) {
            std::cerr << "[Cubemap Error] " << lodepng_error_text(error)
                      << " File: " << filePath << std::endl;
            continue;
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGBA,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            0, GL_RGBA, GL_UNSIGNED_BYTE,
            image.data()
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    std::cout << "[Renderer] Cubemap loaded from: " << dirPath << std::endl;
}

// --- 新增：天空盒渲染 ---
void Renderer::RenderSkybox(const cy::Matrix4f& projection, const cy::Matrix4f& view) {
    if (m_CubemapTexture == 0) return;

    // 保存当前深度测试状态
    m_SkyboxShader.Bind();

    // 去除视图矩阵的平移分量，使天空盒始终以相机为中心
    cy::Matrix4f skyboxView = view;
    skyboxView.cell[12] = 0.0f;
    skyboxView.cell[13] = 0.0f;
    skyboxView.cell[14] = 0.0f;

    m_SkyboxShader.SetMatrix4("projection", &projection.cell[0]);
    m_SkyboxShader.SetMatrix4("view", &skyboxView.cell[0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTexture);
    m_SkyboxShader.SetInt("skybox", 0);

    // 天空盒渲染状态：禁止深度写入，使用 GL_LEQUAL
    // .xyww 技巧使天空盒深度恒为 1.0（远平面），LEQUAL 使其在初始深度 1.0 处可见
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    m_SkyboxMesh.Draw();

    // 恢复默认深度状态
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// --- 新增：反射 Pass ---
void Renderer::BeginReflectionPass() {
    m_ReflectionFramebuffer.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndReflectionPass() {
    m_ReflectionFramebuffer.Unbind();
    m_ReflectionFramebuffer.GenerateMipmaps();
}

// --- 新增：渲染反射地面 ---
void Renderer::RenderGroundPlane(
    const cy::Matrix4f& mvp,
    const cy::Matrix4f& model,
    const cy::Matrix4f& reflectionVP,
    const cy::Vec3f& cameraWorldPos)
{
    m_GroundShader.Bind();

    m_GroundShader.SetMatrix4("mvp", &mvp.cell[0]);
    m_GroundShader.SetMatrix4("model", &model.cell[0]);
    m_GroundShader.SetMatrix4("reflectionVP", &reflectionVP.cell[0]);
    m_GroundShader.SetVec3("cameraWorldPos",
        cameraWorldPos.x, cameraWorldPos.y, cameraWorldPos.z);

    // 绑定 cubemap 到纹理单元 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTexture);
    m_GroundShader.SetInt("cubemap", 0);

    // 绑定反射纹理到纹理单元 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_ReflectionFramebuffer.GetColorTexture());
    m_GroundShader.SetInt("reflectionTex", 1);

    m_GroundPlaneMesh.Draw();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// --- 新增：重新加载着色器的接口 ---
void Renderer::ReloadShaders() {
    const bool mainLoaded = m_MainShader.Load(
        "assets/shaders/triangle.vert",
        "assets/shaders/triangle.frag"
    );

    const bool planeLoaded = m_PlaneShader.Load(
        "assets/shaders/plane.vert",
        "assets/shaders/plane.frag"
    );

    const bool skyboxLoaded = m_SkyboxShader.Load(
        "assets/shaders/skybox.vert",
        "assets/shaders/skybox.frag"
    );

    const bool groundLoaded = m_GroundShader.Load(
        "assets/shaders/ground.vert",
        "assets/shaders/ground.frag"
    );

    if (mainLoaded && planeLoaded && skyboxLoaded && groundLoaded) {
        std::cout << "[Renderer] Shaders reloaded successfully!" << std::endl;
    }else {
        std::cerr << "[Renderer] One or more shaders failed to reload." << std::endl;
    }
}

// --- 新增：调用 LightGizmo 的 Draw 方法 ---
void Renderer::DrawLightGizmo(
    const cy::Matrix4f& proj,
    const cy::Matrix4f& view,
    const cy::Vec3f& lightWorldPos,
    float scale)
{
    // LightGizmo::Draw 已经处理了禁用深度测试和开启混合，所以直接调用即可
    m_LightGizmo.Draw(proj, view, lightWorldPos, scale);
}

void Renderer::SetDiffuseTexture(GLuint textureID) {
	m_DiffuseTexture = textureID;
}
