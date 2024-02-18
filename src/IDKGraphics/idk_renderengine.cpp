#include "idk_renderengine.hpp"
#include "render/idk_initflags.hpp"
#include "render/idk_vxgi.hpp"

#include <libidk/GL/idk_glShaderStage.hpp>
#include <libidk/idk_image.hpp>


static float delta_time = 1.0f;


void
idk::RenderEngine::init_screenquad()
{
    float quad_vertices[] = {
      -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
      -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
       1.0f, -1.0f,  0.0f,  1.0f,  0.0f,

      -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
       1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
       1.0f,  1.0f,  0.0f,  1.0f,  1.0f
    };

    // Send screen quad to GPU
    // ------------------------------------------------------------------------------------
    gl::genVertexArrays(1, &m_quad_VAO);
    gl::genBuffers(1, &m_quad_VBO);

    gl::bindVertexArray(m_quad_VAO);
    gl::bindBuffer(GL_ARRAY_BUFFER, m_quad_VBO);
    gl::bufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    gl::enableVertexAttribArray(0);

    gl::vertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 3*sizeof(float));
    gl::enableVertexAttribArray(1);
    // ------------------------------------------------------------------------------------
}

void
idk::RenderEngine::compileShaders()
{
    createProgram("vxgi-voxelize", "IDKGE/shaders/vxgi/", "voxelize.vs", "voxelize.fs");
    createProgram("vxgi-shadow",   "IDKGE/shaders/vxgi/", "shadow.vs", "shadow.fs");
    createProgram("vxgi-trace",    "IDKGE/shaders/deferred/", "background.vs", "../vxgi/trace.fs");
    createProgram("vxgi-inject",    glShaderProgram("IDKGE/shaders/vxgi/inject-radiance.comp"));
    createProgram("vxgi-propagate", glShaderProgram("IDKGE/shaders/vxgi/propagate-radiance.comp"));

    createProgram("vxgi-mipmap-1",  glShaderProgram("IDKGE/shaders/vxgi/mipmap-1.comp"));
    createProgram("vxgi-mipmap-2",  glShaderProgram("IDKGE/shaders/vxgi/mipmap-2.comp"));
    createProgram("vxgi-clear",     glShaderProgram("IDKGE/shaders/vxgi/clear.comp"));
    createProgram("vxgi-copy",      glShaderProgram("IDKGE/shaders/vxgi/copy.comp"));


    createProgram("background",     "IDKGE/shaders/deferred/", "background.vs", "background.fs");


    createProgram("gpass",     "IDKGE/shaders/deferred/", "gpass-indirect.vs", "gpass-indirect.fs");
    createProgram("dirshadow-indirect", "IDKGE/shaders/", "dirshadow-indirect.vs", "dirshadow.fs");

    createProgram("lpass",          "IDKGE/shaders/", "screenquad.vs", "deferred/lpass.fs");
    // createProgram("lpass",  glShaderProgram(glShaderStage("IDKGE/shaders/deferred/lpass.comp")));

    createProgram("dir-volumetric", "IDKGE/shaders/", "screenquad.vs", "deferred/volumetric_dirlight.fs");

    createProgram("SSR", glShaderProgram("IDKGE/shaders/post/SSR.comp"));
    createProgram("motion-blur", glShaderProgram("IDKGE/shaders/post/motion-blur.comp"));


    createProgram("bloom",          "IDKGE/shaders/", "screenquad.vs", "post/bloom.fs");
    createProgram("downsample",     "IDKGE/shaders/", "screenquad.vs", "post/downsample.fs");
    createProgram("upsample",       "IDKGE/shaders/", "screenquad.vs", "post/upsample.fs");
    createProgram("gaussian",       "IDKGE/shaders/", "screenquad.vs", "post/gaussian.fs");
    createProgram("additive",       "IDKGE/shaders/", "screenquad.vs", "post/additive.fs");
    createProgram("chromatic",      "IDKGE/shaders/", "screenquad.vs", "post/c-abberation.fs");
    createProgram("blit",           "IDKGE/shaders/", "screenquad.vs", "post/blit.fs");
    createProgram("fxaa",           "IDKGE/shaders/", "screenquad.vs", "post/fxaa.fs");
    createProgram("colorgrade",     "IDKGE/shaders/", "screenquad.vs", "post/colorgrade.fs");
    createProgram("dir_shadow",     "IDKGE/shaders/", "dirshadow.vs",  "dirshadow.fs");
    createProgram("dir_shadow-anim", "IDKGE/shaders/", "dirshadow-anim.vs",  "dirshadow.fs");

}


// void
// idk::RenderEngine::compileShaders()
// {
//     idk::glShaderStage VS_vxgi("IDKGE/shaders/vxgi/vxgi.vs");
//     idk::glShaderStage FS_vxgi("IDKGE/shaders/vxgi/vxgi.fs");
//     createProgram("vxgi", VS_vxgi, FS_vxgi);


//     idk::glShaderStage VS_screenquad("IDKGE/shaders/screenquad.vs");
//     idk::glShaderStage FS_dirshadow("IDKGE/shaders/screenquad.vs");
//     idk::glShaderStage FS_gpass_default("IDKGE/shaders/deferred/geometry-pass/default.fs");


//     createProgram("background",      "IDKGE/shaders/deferred/background.vs", "IDKGE/shaders/deferred/background.fs");
//     createProgram("gpass",           "IDKGE/shaders/deferred/geometry-pass/default.vs", FS_gpass_default);
//     createProgram("gpass-anim",      "IDKGE/shaders/deferred/geometry-pass/anim.vs",      FS_gpass_default);
//     createProgram("gpass-instanced", "IDKGE/shaders/deferred/geometry-pass/instanced.vs", "IDKGE/shaders/deferred/geometry-pass/instanced.fs");
//     createProgram("gpass-terrain",   "IDKGE/shaders/deferred/geometry-pass/terrain.vs",   "IDKGE/shaders/deferred/geometry-pass/terrain.fs");

//     createProgram("lpass", VS_screenquad, "IDKGE/shaders/deferred/lighting-pass/pbr.fs");
//     createProgram("dirvolumetrics", VS_screenquad, "IDKGE/shaders/deferred/volumetric_dirlight.fs");


//     createProgram("bloom",      VS_screenquad, "IDKGE/shaders/post/bloom.fs");
//     createProgram("downsample", VS_screenquad, "IDKGE/shaders/post/downsample.fs");
//     createProgram("upsample",   VS_screenquad, "IDKGE/shaders/post/upsample.fs");
//     createProgram("gaussian",   VS_screenquad, "IDKGE/shaders/post/gaussian.fs");
//     createProgram("additive",   VS_screenquad, "IDKGE/shaders/post/additive.fs");
//     createProgram("chromatic",  VS_screenquad, "IDKGE/shaders/post/c-abberation.fs");
//     createProgram("blit",       VS_screenquad, "IDKGE/shaders/post/blit.fs");
//     createProgram("fxaa",       VS_screenquad, "IDKGE/shaders/post/fxaa.fs");
//     createProgram("colorgrade", VS_screenquad, "IDKGE/shaders/post/colorgrade.fs");

//     createProgram("dir_shadow",      "IDKGE/shaders/dirshadow.vs",      FS_dirshadow);
//     createProgram("dir_shadow-anim", "IDKGE/shaders/dirshadow-anim.vs", FS_dirshadow);

//     // createProgram(
//     //     "solid",
//     //     glShaderStage("IDKGE/shaders/vsin_pos_only.vs"),
//     //     glShaderStage("IDKGE/shaders/solid.fs")
//     // );

// }


void
idk::RenderEngine::init_framebuffers( int w, int h )
{
    w = glm::clamp(w, 128, 4096);
    h = glm::clamp(h, 128, 4096);


    idk::glTextureConfig config = {
        .internalformat = GL_RGBA16F,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT
    };

    idk::glTextureConfig config2 = {
        .internalformat = GL_RGBA16F,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig config_highp = {
        .internalformat = GL_RGBA32F,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT
    };

    m_scratchbufs0.resize(NUM_SCRATCH_BUFFERS);
    m_scratchbufs1.resize(NUM_SCRATCH_BUFFERS);
    m_scratchbufs2.resize(NUM_SCRATCH_BUFFERS);
    m_scratchbufs3.resize(NUM_SCRATCH_BUFFERS);

    for (size_t i=0; i<NUM_SCRATCH_BUFFERS; i++)
    {
        int width  = w / pow(2, i);
        int height = h / pow(2, i);

        m_scratchbufs0[i].reset(width, height, ATTACHMENTS_PER_BUFFER);
        m_scratchbufs1[i].reset(width, height, ATTACHMENTS_PER_BUFFER);
        m_scratchbufs2[i].reset(width, height, ATTACHMENTS_PER_BUFFER);
        m_scratchbufs3[i].reset(width, height, ATTACHMENTS_PER_BUFFER);

        for (size_t j=0; j<ATTACHMENTS_PER_BUFFER; j++)
        {
            m_scratchbufs0[i].colorAttachment(j, config);
            m_scratchbufs1[i].colorAttachment(j, config);
            m_scratchbufs2[i].colorAttachment(j, config);
            m_scratchbufs3[i].colorAttachment(j, config);
        }
    }

    m_geom_buffer.reset(w, h, 4);
    m_geom_buffer.colorAttachment(0, config);
    m_geom_buffer.colorAttachment(1, config_highp);
    m_geom_buffer.colorAttachment(2, config2);
    m_geom_buffer.colorAttachment(3, config);

    m_volumetrics_buffer.reset(w/4, h/4, 1);
    m_volumetrics_buffer.colorAttachment(0, config);


    m_finalbuffer.reset(w, h, 1);
    m_finalbuffer.colorAttachment(0, config);


    static const idk::DepthAttachmentConfig depth_config = {
        .internalformat = GL_DEPTH_COMPONENT,
        .datatype       = GL_FLOAT
    };

    m_vxgi_buffer.reset(VXGI_TEXTURE_SIZE, VXGI_TEXTURE_SIZE, 1);
    m_vxgi_buffer.colorAttachment(0, config);
    m_vxgi_buffer.depthAttachment(depth_config);


    m_mainbuffer_0.reset(w, h, 2);
    m_mainbuffer_0.colorAttachment(0, config);
    m_mainbuffer_0.colorAttachment(1, config);

    m_mainbuffer_1.reset(w, h, 2);
    m_mainbuffer_1.colorAttachment(0, config);
    m_mainbuffer_1.colorAttachment(1, config);



    config =
    {
        .internalformat = GL_RGBA16F,
        .format   = GL_RGBA,
        .datatype = GL_FLOAT
    };

    gl::deleteTextures(1, &m_positionbuffer);
    gl::deleteTextures(1, &m_velocitybuffer);
    m_positionbuffer = gltools::loadTexture2D(w, h, nullptr, config);
    m_velocitybuffer = gltools::loadTexture2D(w, h, nullptr, config);

}


void
idk::RenderEngine::init_all( std::string name, int w, int h )
{
    compileShaders();

    init_screenquad();
    init_framebuffers(w, h);

    m_lightsystem.init();


    m_UBO_SyncData.init(0);
    m_UBO_SyncData.bufferData(sizeof(uint32_t), &m_SyncData_index);

    m_UBO_RenderData.init(3);




    m_UBO_camera.init();
    m_UBO_dirlights.init();

    m_UBO_camera.bind(2);
    m_UBO_dirlights.bind(5);

    m_UBO_camera.bufferData(2*sizeof(glm::mat4) + 7*sizeof(glm::vec4), nullptr);
    m_UBO_dirlights.bufferData(IDK_MAX_DIRLIGHTS * (sizeof(Dirlight) + sizeof(glm::mat4)), nullptr);



    // Compute BRDF LUT
    // -----------------------------------------------------------------------------------------
    idk::glTextureConfig config = {
        .internalformat = GL_RG16F,
        .format         = GL_RG,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_CLAMP_TO_EDGE,
        .wrap_t         = GL_CLAMP_TO_EDGE,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    static constexpr size_t LUT_TEXTURE_SIZE = 512;

    BRDF_LUT = idk::gltools::loadTexture2D(LUT_TEXTURE_SIZE, LUT_TEXTURE_SIZE, nullptr, config);
    idk::gl::bindImageTexture(0, BRDF_LUT, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);

    idk::glShaderProgram program("IDKGE/shaders/brdf-lut.comp");
    program.bind();
    program.dispatch(LUT_TEXTURE_SIZE/8, LUT_TEXTURE_SIZE/8, 1);
    idk::gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    program.unbind();
    // -----------------------------------------------------------------------------------------




    m_active_camera_id = createCamera();

    loadSkybox("IDKGE/resources/skybox/");



    vxgi_albedo = VXGI::allocateTexture(VXGI_TEXTURE_SIZE);
    vxgi_normal = VXGI::allocateTexture(VXGI_TEXTURE_SIZE);

    for (int i=0; i<6; i++)
    {
        vxgi_radiance[i] = VXGI::allocateTexture(VXGI_TEXTURE_SIZE);
        vxgi_radiance_2[i] = VXGI::allocateTexture(VXGI_TEXTURE_SIZE);
    }

}



idk::RenderEngine::RenderEngine( const std::string &name, int w, int h, int gl_major,
                                 int gl_minor, uint32_t flags )
:
    m_windowsys(name.c_str(), w, h, gl_major, gl_minor, flags)
{
    m_resolution = glm::ivec2(w, h);
    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);

    if (flags == idk::InitFlag::NONE)
    {
        init_all(name, w, h);
        return;
    }

    if (flags & idk::InitFlag::INIT_HEADLESS)
    {
        return;
    }
}


int
idk::RenderEngine::createProgram( const std::string &name, const std::string &root,
                                  const std::string &vs, const std::string &fs )
{
    int id = m_programs.create();
    m_programs.get(id).loadFileC(root, vs, fs);
    m_program_ids[name] = id;

    return id;
}


int
idk::RenderEngine::createProgram( const std::string &name, const idk::glShaderProgram &program )
{
    int id = m_programs.create(program);
    m_program_ids[name] = id;
    return id;
}


int
idk::RenderEngine::loadSkybox( const std::string &filepath )
{
    gl::enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    static const std::vector<std::string> faces
    {
        "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png"
    };

    static const std::vector<std::string> faces2
    {
        "0px.png", "0nx.png", "0py.png", "0ny.png", "0pz.png", "0nz.png"
    };

    static const glTextureConfig skybox_config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = false
    };

    static const glTextureConfig diffuse_config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = true
    };


    static const glTextureConfig specular_config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = false,
        .setmipmap      = true,
        .texbaselevel   = 0,
        .texmaxlevel    = 5
    };

    GLuint skybox   = gltools::loadCubemap(filepath, faces, skybox_config);
    GLuint diffuse  = gltools::loadCubemap(filepath + "diffuse/", faces, diffuse_config);
    GLuint specular = gltools::loadCubemap(filepath + "specular/", faces2, specular_config);

    for (GLint mip=0; mip<=5; mip++)
    {
        gltools::loadCubemapMip(filepath + "specular/", faces, specular_config, specular, mip);
    }

    skyboxes.push_back(skybox);
    skyboxes_IBL.push_back(std::make_pair(diffuse, specular));

    return skyboxes.size() - 1;
}



void
idk::RenderEngine::f_fbfb( glShaderProgram &program, glFramebuffer &in )
{
    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);

    for (size_t i=0; i < in.attachments.size(); i++)
        program.set_sampler2D("un_texture_" + std::to_string(i), in.attachments[i]);

    gl::drawArrays(GL_TRIANGLES, 0, 6);
}



void
idk::RenderEngine::tex2tex( glShaderProgram &program, glFramebuffer &in, glFramebuffer &out )
{
    out.bind();

    for (size_t i=0; i < in.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(i), in.attachments[i]);
    }

    gl::drawArrays(GL_TRIANGLES, 0, 6);
};



void
idk::RenderEngine::tex2tex( glShaderProgram &program, glFramebuffer &a, glFramebuffer &b, glFramebuffer &out )
{
    out.bind();

    size_t textureID = 0;
    for (size_t i=0; i < a.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(i), a.attachments[i]);
    }

    for (size_t i=0; i < b.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(4+i), b.attachments[i]);
    }

    gl::drawArrays(GL_TRIANGLES, 0, 6);
};



int
idk::RenderEngine::createCamera()
{
    int camera_id = m_camera_allocator.create();
    m_camera_allocator.get(camera_id).aspect(m_resolution.x, m_resolution.y);
    return camera_id;
}


int
idk::RenderEngine::loadModel( const std::string &filepath )
{
    return m_model_allocator.loadModel(filepath);
}


void
idk::RenderEngine::drawModel( int model_id, const glm::mat4 &transform )
{
    modelAllocator().pushModelDraw(model_id, transform);
    // _getRenderQueue(m_RQ).push(model_id, model_mat);
}


void
idk::RenderEngine::drawShadowCaster( int model_id, const glm::mat4 &model_mat )
{
    // m_shadow_render_queue.push(model_id, model_mat);
}


void
idk::RenderEngine::drawEnvironmental( int model, const glm::mat4 &transform )
{
    // m_vxgi_RQ.push(model, transform);
}



void
idk::RenderEngine::shadowpass_dirlights( GLuint VAO, GLuint draw_indirect_buffer, 
                                         const std::vector<idk::glDrawElementsIndirectCommand> &commands )
{
    idk::Camera &cam = getCamera();

    std::vector<idk::Dirlight> &dirlights = m_lightsystem.dirlights();
    idk::glDepthCascade &depthcascade = m_lightsystem.depthCascade();

    idk::Dirlight &light = dirlights[0];
    const glm::vec3 dir = glm::normalize(glm::vec3(light.direction));


    // Compute matrices
    // -------------------------------------------------------------------------------------
    depthcascade.computeCascadeMatrices(
        cam.getFOV(),    cam.getAspect(),  cam.nearPlane(),
        cam.farPlane(),  cam.view(),       dir
    );

    const auto &cascade_matrices = depthcascade.getCascadeMatrices();
    // -------------------------------------------------------------------------------------

    depthcascade.bind();


    auto &program = getProgram("dirshadow-indirect");
    program.bind();

    gl::bindVertexArray(VAO);

    gl::namedBufferSubData(
        draw_indirect_buffer,
        0,
        commands.size() * sizeof(idk::glDrawElementsIndirectCommand),
        reinterpret_cast<const void *>(commands.data())
    );

    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_indirect_buffer);


    for (int i=0; i<glDepthCascade::NUM_CASCADES; i++)
    {
        depthcascade.setOutputAttachment(i);
        depthcascade.clear(GL_DEPTH_BUFFER_BIT);

        program.set_mat4("un_lightspacematrix", cascade_matrices[i]);

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            commands.size(),
            sizeof(idk::glDrawElementsIndirectCommand)
        );
    }


    program.unbind();
}


void
idk::RenderEngine::update_UBO_camera()
{
    idk::Camera &camera = getCamera();

    m_UBO_camera.bind();
    m_UBO_camera.add<glm::mat4>(glm::value_ptr(camera.view()));
    m_UBO_camera.add<glm::mat4>(glm::value_ptr(camera.projection()));


    glm::vec3 pos0 = camera.position();

    static float increment = -M_PI;
    glm::vec4 pos = glm::vec4(camera.renderPosition(), increment);
    increment = fmod(increment + delta_time, 2.0f * M_PI);

    glm::vec4 b(camera.m_r_abr, camera.m_g_abr);
    glm::vec4 c(camera.m_b_abr, camera.m_abr_str, 0.0f);

    m_UBO_camera.add<glm::vec3>(glm::value_ptr(pos0));
    m_UBO_camera.add<glm::vec4>(glm::value_ptr(pos));
    m_UBO_camera.add<glm::vec4>(glm::value_ptr(camera.m_bloom_gamma));
    m_UBO_camera.add<glm::vec4>(glm::value_ptr(b));
    m_UBO_camera.add<glm::vec4>(glm::value_ptr(c));
    m_UBO_camera.add<glm::vec4>(glm::value_ptr(camera.m_exposure));
    m_UBO_camera.add<glm::vec4>(glm::value_ptr(glm::vec4(m_resolution, 0.0f, 0.0f)));

    m_UBO_camera.unbind();

    static glm::mat4 prev_v, prev_p, prev_pv;
    static glm::vec3 prev_position;

    m_RenderData.cameras[0].position = glm::vec4(pos0, 1.0f);
    m_RenderData.cameras[0].V  = camera.view();
    m_RenderData.cameras[0].P  = camera.projection();
    m_RenderData.cameras[0].PV = camera.view() * camera.projection();
    m_RenderData.cameras[0].image_size = glm::vec4(width(), height(), 0.0f, 0.0f);

    m_RenderData.cameras[0].prev_position = glm::vec4(prev_position, 1.0f);
    m_RenderData.cameras[0].prev_V  = prev_v;
    m_RenderData.cameras[0].prev_P  = prev_p;
    m_RenderData.cameras[0].prev_PV = prev_pv;

    prev_position = camera.position();
    prev_v  = camera.view();
    prev_p  = camera.projection();
    prev_pv = prev_p * prev_v;
    
}


void
idk::RenderEngine::update_UBO_dirlights()
{
    idk::Camera &cam = getCamera();

    std::vector<Dirlight>  lights(IDK_MAX_DIRLIGHTS);
    std::vector<glm::mat4> matrices(IDK_MAX_DIRLIGHTS);

    std::vector<idk::Dirlight> &dirlights = m_lightsystem.dirlights();
    idk::Dirlight &light = dirlights[0];
    const glm::vec3 dir = glm::normalize(glm::vec3(light.direction));

    const auto &cascade_matrices = m_lightsystem.depthCascade().getCascadeMatrices();

    for (int j=0; j<cascade_matrices.size(); j++)
    {
        lights[0]   = light;
    }


    m_UBO_dirlights.bind();
    m_UBO_dirlights.add(IDK_MAX_DIRLIGHTS * sizeof(Dirlight),  lights.data());
    m_UBO_dirlights.add(glDepthCascade::NUM_CASCADES * sizeof(glm::mat4), cascade_matrices.data());
    m_UBO_dirlights.unbind();
}



void
idk::RenderEngine::beginFrame()
{
    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);
    gl::viewport(0, 0, m_resolution.x, m_resolution.y);
    gl::clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    idk::Camera &camera = getCamera();

    float near     = camera.nearPlane();
    float far      = camera.farPlane();
    glm::mat4 view = camera.view();
    glm::mat4 proj = camera.projection();


}






static void MultiDrawTest( idk::glShaderProgram &program, GLuint VAO, GLuint draw_indirect_buffer, 
                           const std::vector<idk::glDrawElementsIndirectCommand> &commands )
{
    using namespace idk;

    program.bind();

    gl::bindVertexArray(VAO);
    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_indirect_buffer);

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        commands.size(),
        sizeof(idk::glDrawElementsIndirectCommand)
    );

    program.unbind();
};





void
idk::RenderEngine::endFrame( float dt )
{
    delta_time = dt;

    const auto &commands = modelAllocator().genDrawCommands();
    gl::bindVertexArray(modelAllocator().getVAO());

    gl::disable(GL_CULL_FACE);

    shadowpass_dirlights(
        modelAllocator().getVAO(),
        modelAllocator().getDrawIndirectBuffer(),
        commands
    );

    gl::enable(GL_CULL_FACE);


    update_UBO_camera();
    update_UBO_dirlights();


    uint32_t index = m_SyncData_index;
    m_SyncData_index = m_UBO_RenderData.update(m_SyncData_index, m_RenderData);
    m_UBO_SyncData.bufferSubData(0, sizeof(uint32_t), &index);


    idk::Camera &camera = getCamera();

    gl::bindImageTextures(0, 6, &vxgi_radiance_2[0]);
    gl::bindImageTextures(8, 6, &vxgi_radiance[0]);
    gl::bindImageTexture(6, vxgi_albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
    gl::bindImageTexture(7, vxgi_normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);


    // Clear volume textures
    // -----------------------------------------------------------------------------------------
    {
        auto &program = getProgram("vxgi-clear");
        program.bind();
        program.dispatch(VXGI_TEXTURE_SIZE / 4);
        gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    // -----------------------------------------------------------------------------------------


    VXGI::shadowPass(
        m_vxgi_buffer,
        camera,
        lightSystem().getDirlight(0).direction,
        getProgram("vxgi-shadow"),
        modelAllocator().getDrawIndirectBuffer(),
        commands
    );


    VXGI::renderTexture(
        m_vxgi_buffer,
        camera,
        getProgram("vxgi-voxelize"),
        modelAllocator().getDrawIndirectBuffer(),
        commands,
        m_lightsystem.depthCascade()
    );


    static int vxgi_face   = 0;
    static int vxgi_offset = 0;

    vxgi_face = (vxgi_face + 1) % 6;
    vxgi_offset = (vxgi_offset + 1) % 4;

    {
        auto &program = getProgram("vxgi-inject");
        program.bind();
        program.set_int("un_face", vxgi_face);
        program.set_int("un_offset", vxgi_offset);

        VXGI::injectRadiance(
            program,
            camera,
            m_vxgi_buffer,
            lightSystem().getDirlight(0).direction,
            lightSystem().depthCascade()
        );
    }
    VXGI::generateMipmap(getProgram("vxgi-mipmap-1"), getProgram("vxgi-mipmap-2"), vxgi_radiance_2);

    {
        auto &program = getProgram("vxgi-propagate");
        program.bind();
        program.set_int("un_face", vxgi_face);
        program.set_int("un_offset", vxgi_offset);

        for (int i=0; i<6; i++)
        {
            program.set_sampler2D("un_input_radiance[" + std::to_string(i) + "]", vxgi_radiance_2[i]);
        }
        program.dispatch(VXGI_TEXTURE_SIZE/4);
        gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    VXGI::generateMipmap(getProgram("vxgi-mipmap-1"), getProgram("vxgi-mipmap-2"), vxgi_radiance);


    m_geom_buffer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_geom_buffer.bind();

    MultiDrawTest(
        getProgram("gpass"),
        modelAllocator().getVAO(),
        modelAllocator().getDrawIndirectBuffer(),
        commands
    );


    gl::bindImageTexture(30, m_positionbuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    gl::bindImageTexture(31, m_velocitybuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    RenderStage_geometry(camera, dt, m_geom_buffer);
    RenderStage_lighting(camera, dt, m_geom_buffer,  m_mainbuffer_0);

    // {
    //     gl::bindImageTexture(0, m_mainbuffer_0.attachments[0], 0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA16F);
    //     gl::bindImageTexture(1, m_mainbuffer_1.attachments[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    //     gl::memoryBarrier(GL_ALL_BARRIER_BITS);

    //     auto &program = getProgram("motion-blur");
    //     program.bind();
    //     program.dispatch(width()/8, height()/8, 1);

    //     gl::memoryBarrier(GL_ALL_BARRIER_BITS);
    // }


    RenderStage_postprocessing(camera, m_mainbuffer_0, m_finalbuffer);

    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);
    gl::bindVertexArray(0);
}


void
idk::RenderEngine::swapWindow()
{
    SDL_GL_SwapWindow(this->getWindow());
}


void
idk::RenderEngine::resize( int w, int h )
{
    m_resolution.x = w;  m_resolution.y = h;
    init_framebuffers(w, h);
    getCamera().aspect(w, h);
}

