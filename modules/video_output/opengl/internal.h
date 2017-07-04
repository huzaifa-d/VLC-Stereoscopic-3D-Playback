/*****************************************************************************
 * opengl_internal.h: OpenGL internal header
 *****************************************************************************
 * Copyright (C) 2016 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef VLC_OPENGL_INTERNAL_H
#define VLC_OPENGL_INTERNAL_H

#include "vout_helper.h"

#if defined(USE_OPENGL_ES2)
#   define GLSL_VERSION "100"
#   define PRECISION "precision highp float;"
#   define VLCGL_PICTURE_MAX 128
#   define glClientActiveTexture(x)
#   define VLCGL_HAS_PBO /* PBO present as an OpenGlES 2 extension */
#else
#   define GLSL_VERSION "120"
#   define VLCGL_PICTURE_MAX 128
#   ifdef GL_VERSION_2_0
#       define VLCGL_HAS_PBO
#   endif
#   ifdef GL_VERSION_4_4
#       define VLCGL_HAS_MAP_PERSISTENT
#   endif
#   define PRECISION ""
#   if defined(__APPLE__)
#       define GL_TEXTURE_RECTANGLE 0x84F5
#   endif
#endif

#if defined(USE_OPENGL_ES2) || defined(__APPLE__)
#   define PFNGLGETPROGRAMIVPROC             typeof(glGetProgramiv)*
#   define PFNGLGETPROGRAMINFOLOGPROC        typeof(glGetProgramInfoLog)*
#   define PFNGLGETSHADERIVPROC              typeof(glGetShaderiv)*
#   define PFNGLGETSHADERINFOLOGPROC         typeof(glGetShaderInfoLog)*
#   define PFNGLGETUNIFORMLOCATIONPROC       typeof(glGetUniformLocation)*
#   define PFNGLGETATTRIBLOCATIONPROC        typeof(glGetAttribLocation)*
#   define PFNGLVERTEXATTRIBPOINTERPROC      typeof(glVertexAttribPointer)*
#   define PFNGLENABLEVERTEXATTRIBARRAYPROC  typeof(glEnableVertexAttribArray)*
#   define PFNGLUNIFORMMATRIX4FVPROC         typeof(glUniformMatrix4fv)*
#   define PFNGLUNIFORM4FVPROC               typeof(glUniform4fv)*
#   define PFNGLUNIFORM4FPROC                typeof(glUniform4f)*
#   define PFNGLUNIFORM2FPROC                typeof(glUniform2f)*
#   define PFNGLUNIFORM1IPROC                typeof(glUniform1i)*
#   define PFNGLCREATESHADERPROC             typeof(glCreateShader)*
#   define PFNGLSHADERSOURCEPROC             typeof(glShaderSource)*
#   define PFNGLCOMPILESHADERPROC            typeof(glCompileShader)*
#   define PFNGLDELETESHADERPROC             typeof(glDeleteShader)*
#   define PFNGLCREATEPROGRAMPROC            typeof(glCreateProgram)*
#   define PFNGLLINKPROGRAMPROC              typeof(glLinkProgram)*
#   define PFNGLUSEPROGRAMPROC               typeof(glUseProgram)*
#   define PFNGLDELETEPROGRAMPROC            typeof(glDeleteProgram)*
#   define PFNGLATTACHSHADERPROC             typeof(glAttachShader)*
#   define PFNGLGENBUFFERSPROC               typeof(glGenBuffers)*
#   define PFNGLBINDBUFFERPROC               typeof(glBindBuffer)*
#   define PFNGLBUFFERDATAPROC               typeof(glBufferData)*
#   ifdef VLCGL_HAS_PBO
#    define PFNGLBUFFERSUBDATAPROC           typeof(glBufferSubData)*
#   endif
#   ifdef VLCGL_HAS_MAP_PERSISTENT
#    define PFNGLBUFFERSTORAGEPROC           typeof(glBufferStorage)*
#    define PFNGLMAPBUFFERRANGEPROC          typeof(glMapBufferRange)*
#    define PFNGLFLUSHMAPPEDBUFFERRANGEPROC  typeof(glFlushMappedBufferRange)*
#    define PFNGLUNMAPBUFFERPROC             typeof(glUnmapBuffer)*
#    define PFNGLFENCESYNCPROC               typeof(glFenceSync)*
#    define PFNGLDELETESYNCPROC              typeof(glDeleteSync)*
#    define PFNGLCLIENTWAITSYNCPROC          typeof(glClientWaitSync)*
#   endif
#   define PFNGLDELETEBUFFERSPROC            typeof(glDeleteBuffers)*
#if defined(__APPLE__)
#   import <CoreFoundation/CoreFoundation.h>
#endif
#endif

/**
 * Structure containing function pointers to shaders commands
 */
typedef struct {
    /* Shader variables commands*/
    PFNGLGETUNIFORMLOCATIONPROC      GetUniformLocation;
    PFNGLGETATTRIBLOCATIONPROC       GetAttribLocation;
    PFNGLVERTEXATTRIBPOINTERPROC     VertexAttribPointer;
    PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;

    PFNGLUNIFORMMATRIX4FVPROC   UniformMatrix4fv;
    PFNGLUNIFORM4FVPROC         Uniform4fv;
    PFNGLUNIFORM4FPROC          Uniform4f;
    PFNGLUNIFORM2FPROC          Uniform2f;
    PFNGLUNIFORM1IPROC          Uniform1i;

    /* Shader command */
    PFNGLCREATESHADERPROC CreateShader;
    PFNGLSHADERSOURCEPROC ShaderSource;
    PFNGLCOMPILESHADERPROC CompileShader;
    PFNGLDELETESHADERPROC   DeleteShader;

    PFNGLCREATEPROGRAMPROC CreateProgram;
    PFNGLLINKPROGRAMPROC   LinkProgram;
    PFNGLUSEPROGRAMPROC    UseProgram;
    PFNGLDELETEPROGRAMPROC DeleteProgram;

    PFNGLATTACHSHADERPROC  AttachShader;

    /* Shader log commands */
    PFNGLGETPROGRAMIVPROC  GetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
    PFNGLGETSHADERIVPROC   GetShaderiv;
    PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;

    PFNGLGENBUFFERSPROC    GenBuffers;
    PFNGLBINDBUFFERPROC    BindBuffer;
    PFNGLBUFFERDATAPROC    BufferData;
#ifdef VLCGL_HAS_PBO
    PFNGLBUFFERSUBDATAPROC          BufferSubData;
#endif
#ifdef VLCGL_HAS_MAP_PERSISTENT
    PFNGLBUFFERSTORAGEPROC          BufferStorage;
    PFNGLMAPBUFFERRANGEPROC         MapBufferRange;
    PFNGLFLUSHMAPPEDBUFFERRANGEPROC FlushMappedBufferRange;
    PFNGLUNMAPBUFFERPROC            UnmapBuffer;
    PFNGLFENCESYNCPROC              FenceSync;
    PFNGLDELETESYNCPROC             DeleteSync;
    PFNGLCLIENTWAITSYNCPROC         ClientWaitSync;
#endif
    PFNGLDELETEBUFFERSPROC DeleteBuffers;

#if defined(_WIN32)
    PFNGLACTIVETEXTUREPROC  ActiveTexture;
    PFNGLCLIENTACTIVETEXTUREPROC  ClientActiveTexture;
#   undef glClientActiveTexture
#   undef glActiveTexture
#   define glActiveTexture tc->api->ActiveTexture
#   define glClientActiveTexture tc->api->ClientActiveTexture
#endif

} opengl_shaders_api_t;

typedef struct opengl_tex_converter_t opengl_tex_converter_t;

/*
 * Callback to initialize an opengl_tex_converter_t struct
 *
 * The implementation should initialize every members of the struct in regards
 * of the video format.
 *
 * \param fmt video format, fmt->i_chroma can be modified in order to match a
 * shader
 * \param fc OpenGL tex converter that needs to be filled on success
 * \return VLC_SUCCESS or a VLC error
 */
typedef int (*opengl_tex_converter_init_cb)(opengl_tex_converter_t *fc);

/*
 * Structure that is filled by an opengl_tex_converter_init_cb function
 */
struct opengl_tex_converter_t
{
    /* Pointer to object gl, set by the caller of the init cb */
    vlc_gl_t *gl;
    /* Function pointer to shaders commands, set by the caller of the init cb */
    const opengl_shaders_api_t *api;
    /* Available gl extensions (from GL_EXTENSIONS) */
    const char *glexts;

    /* Can only be changed from the module open function */
    video_format_t fmt;

    /* Fragment shader, must be set from the module open function. It will be
     * deleted by the caller. */
    GLuint fshader;

    /* Number of textures, cannot be 0 */
    unsigned tex_count;

    /* Texture mapping (usually: GL_TEXTURE_2D), cannot be 0 */
    GLenum tex_target;

    /* Set to true if textures are generated from pf_update() */
    bool handle_texs_gen;

    struct opengl_tex_cfg {
        /* Texture scale factor, cannot be 0 */
        vlc_rational_t w;
        vlc_rational_t h;

        /* The following is used and filled by the opengl_fragment_shader_init
         * function. */
        GLint  internal;
        GLenum format;
        GLenum type;
    } texs[PICTURE_PLANE_MAX];

    /* The following is used and filled by the opengl_fragment_shader_init
     * function. */
    struct {
        GLint Texture[PICTURE_PLANE_MAX];
        GLint TexSize[PICTURE_PLANE_MAX]; /* for GL_TEXTURE_RECTANGLE */
        GLint Coefficients;
        GLint FillColor;
    } uloc;
    bool yuv_color;
    GLfloat yuv_coefficients[16];

    /* Private context */
    void *priv;

    /*
     * Callback to allocate data for bound textures
     *
     * This function pointer can be NULL. Software converters should call
     * glTexImage2D() to allocate textures data (it will be deallocated by the
     * caller when calling glDeleteTextures()). Won't be called if
     * handle_texs_gen is true.
     *
     * \param fc OpenGL tex converter
     * \param textures array of textures to bind (one per plane)
     * \param tex_width array of tex width (one per plane)
     * \param tex_height array of tex height (one per plane)
     * \return VLC_SUCCESS or a VLC error
     */
    int (*pf_allocate_textures)(const opengl_tex_converter_t *tc, GLuint *textures,
                                const GLsizei *tex_width, const GLsizei *tex_height);

    /*
     * Callback to allocate a picture pool
     *
     * This function pointer *can* be NULL. If NULL, A generic pool with
     * pictures allocated from the video_format_t will be used.
     *
     * \param fc OpenGL tex converter
     * \param requested_count number of pictures to allocate
     * \return the picture pool or NULL in case of error
     */
    picture_pool_t *(*pf_get_pool)(const opengl_tex_converter_t *fc,
                                   unsigned requested_count);

    /*
     * Callback to update a picture
     *
     * This function pointer cannot be NULL. The implementation should upload
     * every planes of the picture.
     *
     * \param fc OpenGL tex converter
     * \param textures array of textures to bind (one per plane)
     * \param tex_width array of tex width (one per plane)
     * \param tex_height array of tex height (one per plane)
     * \param pic picture to update
     * \param plane_offset offsets of each picture planes to read data from
     * (one per plane, can be NULL)
     * \return VLC_SUCCESS or a VLC error
     */
    int (*pf_update)(const opengl_tex_converter_t *fc, GLuint *textures,
                     const GLsizei *tex_width, const GLsizei *tex_height,
                     picture_t *pic, const size_t *plane_offset);

    /*
     * Callback to fetch locations of uniform or attributes variables
     *
     * This function pointer cannot be NULL. This callback is called one time
     * after init.
     *
     * \param fc OpenGL tex converter
     * \param program linked program that will be used by this tex converter
     * \return VLC_SUCCESS or a VLC error
     */
    int (*pf_fetch_locations)(opengl_tex_converter_t *fc, GLuint program);

    /*
     * Callback to prepare the fragment shader
     *
     * This function pointer cannot be NULL. This callback can be used to
     * specify values of uniform variables.
     *
     * \param fc OpenGL tex converter
     * \param tex_width array of tex width (one per plane)
     * \param tex_height array of tex height (one per plane)
     * \param alpha alpha value, used only for RGBA fragment shader
     */
    void (*pf_prepare_shader)(const opengl_tex_converter_t *fc,
                              const GLsizei *tex_width, const GLsizei *tex_height,
                              float alpha);

    /*
     * Callback to release the private context
     *
     * This function pointer can be NULL.
     * \param fc OpenGL tex converter
     */
    void (*pf_release)(const opengl_tex_converter_t *fc);
};

/*
 * Generate a fragment shader
 *
 * This utility function can be used by hw opengl tex converters that need a
 * generic fragment shader. It will compile a fragment shader generated from
 * the chroma and the tex target. This will initialize all elements of the
 * opengl_tex_converter_t struct except for priv, pf_allocate_texture,
 * pf_get_pool, pf_update, and pf_release.
 *
 * \param tc OpenGL tex converter
 * \param tex_target GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE
 * \param chroma chroma used to generate the fragment shader
 * \param if not COLOR_SPACE_UNDEF, YUV planes will be converted to RGB
 * according to the color space
 * \return the compiled fragment shader or 0 in case of error
 */
GLuint
opengl_fragment_shader_init(opengl_tex_converter_t *tc, GLenum tex_target,
                            vlc_fourcc_t chroma, video_color_space_t yuv_space);

int
opengl_tex_converter_subpictures_init(opengl_tex_converter_t *);

int
opengl_tex_converter_generic_init(opengl_tex_converter_t *);

#ifdef __ANDROID__
int
opengl_tex_converter_anop_init(opengl_tex_converter_t *);
#endif

#ifdef VLCGL_CONV_CVPX
int
opengl_tex_converter_cvpx_init(opengl_tex_converter_t *tc);
#endif

#ifdef VLCGL_CONV_VA
int
opengl_tex_converter_vaapi_init(opengl_tex_converter_t *tc);
#endif

#endif /* include-guard */
