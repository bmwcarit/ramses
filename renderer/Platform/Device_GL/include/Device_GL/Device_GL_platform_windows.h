//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEVICE_GL_PLATFORM_WINDOWS_H
#define RAMSES_DEVICE_GL_PLATFORM_WINDOWS_H

// Required workaround for WGL, because it defines all extension procs
// and leaves no possibility to check if an extension is available in the headers
// or not
#define GL_NV_draw_buffers 1
#define PFNGLDRAWBUFFERSNVPROC PFNGLDRAWBUFFERSPROC

#define LOAD_API_PROC(CONTEXT, TYPE, NAME)        \
    NAME##Native = reinterpret_cast<TYPE>(CONTEXT.getProcAddress(#NAME)); \
    if(0 == NAME##Native)                               \
    {                                                   \
        LOG_DEBUG(CONTEXT_RENDERER, "--->  loading address of proc "#NAME" failed!");  \
    }

#define DECLARE_API_PROC(TYPE, NAME) extern TYPE NAME##Native;
#define DECLARE_EXTENSION_PROC(TYPE, NAME) extern TYPE NAME;

#define DEFINE_API_PROC(TYPE, NAME) TYPE NAME##Native = 0;
#define DEFINE_EXTENSION_PROC(TYPE, NAME) TYPE NAME = 0;

#define glGetStringi(...)               glGetStringiNative(__VA_ARGS__)
#define glCreateProgram(...)            glCreateProgramNative(__VA_ARGS__)
#define glDeleteProgram(...)            glDeleteProgramNative(__VA_ARGS__)
#define glUseProgram(...)               glUseProgramNative(__VA_ARGS__)
#define glProgramBinary(...)            glProgramBinaryNative(__VA_ARGS__)
#define glGetProgramBinary(...)         glGetProgramBinaryNative(__VA_ARGS__)
#define glAttachShader(...)             glAttachShaderNative(__VA_ARGS__)
#define glDetachShader(...)             glDetachShaderNative(__VA_ARGS__)
#define glLinkProgram(...)              glLinkProgramNative(__VA_ARGS__)
#define glGetProgramiv(...)             glGetProgramivNative(__VA_ARGS__)
#define glGetShaderInfoLog(...)         glGetShaderInfoLogNative(__VA_ARGS__)
#define glGetUniformLocation(...)       glGetUniformLocationNative(__VA_ARGS__)
#define glUniform1i(...)                glUniform1iNative(__VA_ARGS__)
#define glUniform1iv(...)               glUniform1ivNative(__VA_ARGS__)
#define glUniform2iv(...)               glUniform2ivNative(__VA_ARGS__)
#define glUniform3iv(...)               glUniform3ivNative(__VA_ARGS__)
#define glUniform4iv(...)               glUniform4ivNative(__VA_ARGS__)
#define glUniform1f(...)                glUniform1fNative(__VA_ARGS__)
#define glUniform1fv(...)               glUniform1fvNative(__VA_ARGS__)
#define glUniform2fv(...)               glUniform2fvNative(__VA_ARGS__)
#define glUniform3fv(...)               glUniform3fvNative(__VA_ARGS__)
#define glUniform4fv(...)               glUniform4fvNative(__VA_ARGS__)
#define glUniformMatrix2fv(...)         glUniformMatrix2fvNative(__VA_ARGS__)
#define glUniformMatrix3fv(...)         glUniformMatrix3fvNative(__VA_ARGS__)
#define glUniformMatrix4fv(...)         glUniformMatrix4fvNative(__VA_ARGS__)
#define glGetAttribLocation(...)        glGetAttribLocationNative(__VA_ARGS__)
#define glVertexAttrib1f(...)           glVertexAttrib1fNative(__VA_ARGS__)
#define glVertexAttrib1fv(...)          glVertexAttrib1fvNative(__VA_ARGS__)
#define glVertexAttrib2fv(...)          glVertexAttrib2fvNative(__VA_ARGS__)
#define glVertexAttrib3fv(...)          glVertexAttrib3fvNative(__VA_ARGS__)
#define glVertexAttrib4fv(...)          glVertexAttrib4fvNative(__VA_ARGS__)
#define glEnableVertexAttribArray(...)  glEnableVertexAttribArrayNative(__VA_ARGS__)
#define glBindAttribLocation(...)       glBindAttribLocationNative(__VA_ARGS__)
#define glGetActiveUniform(...)         glGetActiveUniformNative(__VA_ARGS__)
#define glGetActiveAttrib(...)          glGetActiveAttribNative(__VA_ARGS__)
#define glGetProgramInfoLog(...)        glGetProgramInfoLogNative(__VA_ARGS__)
#define glCreateShader(...)             glCreateShaderNative(__VA_ARGS__)
#define glDeleteShader(...)             glDeleteShaderNative(__VA_ARGS__)
#define glShaderSource(...)             glShaderSourceNative(__VA_ARGS__)
#define glCompileShader(...)            glCompileShaderNative(__VA_ARGS__)
#define glGetShaderiv(...)              glGetShaderivNative(__VA_ARGS__)
#define glGenVertexArrays(...)          glGenVertexArraysNative(__VA_ARGS__)
#define glBindVertexArray(...)          glBindVertexArrayNative(__VA_ARGS__)
#define glDeleteVertexArrays(...)       glDeleteVertexArraysNative(__VA_ARGS__)
#define glGenBuffers(...)               glGenBuffersNative(__VA_ARGS__)
#define glBindBuffer(...)               glBindBufferNative(__VA_ARGS__)
#define glBufferData(...)               glBufferDataNative(__VA_ARGS__)
#define glVertexAttribPointer(...)      glVertexAttribPointerNative(__VA_ARGS__)
#define glGenFramebuffers(...)          glGenFramebuffersNative(__VA_ARGS__)
#define glBindFramebuffer(...)          glBindFramebufferNative(__VA_ARGS__)
#define glBlitFramebuffer(...)          glBlitFramebufferNative(__VA_ARGS__)
#define glDeleteFramebuffers(...)       glDeleteFramebuffersNative(__VA_ARGS__)
#define glFramebufferTexture2D(...)     glFramebufferTexture2DNative(__VA_ARGS__)
#define glDeleteBuffers(...)            glDeleteBuffersNative(__VA_ARGS__)
#define glCheckFramebufferStatus(...)   glCheckFramebufferStatusNative(__VA_ARGS__)
#define glGenRenderbuffers(...)         glGenRenderbuffersNative(__VA_ARGS__)
#define glBindRenderbuffer(...)         glBindRenderbufferNative(__VA_ARGS__)
#define glRenderbufferStorage(...)      glRenderbufferStorageNative(__VA_ARGS__)
#define glRenderbufferStorageMultisample(...)      glRenderbufferStorageMultisampleNative(__VA_ARGS__)
#define glFramebufferRenderbuffer(...)  glFramebufferRenderbufferNative(__VA_ARGS__)
#define glDrawBuffers(...)              glDrawBuffersNative(__VA_ARGS__)
#define glDeleteRenderbuffers(...)      glDeleteRenderbuffersNative(__VA_ARGS__)
#define glActiveTexture(...)            glActiveTextureNative(__VA_ARGS__)
#define glBlendEquation(...)            glBlendEquationNative(__VA_ARGS__)
#define glBlendEquationSeparate(...)    glBlendEquationSeparateNative(__VA_ARGS__)
#define glBlendFuncSeparate(...)        glBlendFuncSeparateNative(__VA_ARGS__)
#define glBlendColor(...)               glBlendColorNative(__VA_ARGS__)
#define glGenerateMipmap(...)           glGenerateMipmapNative(__VA_ARGS__)
#define glCompressedTexImage2D(...)     glCompressedTexImage2DNative(__VA_ARGS__)
#define glGenSamplers(...)              glGenSamplersNative(__VA_ARGS__)
#define glDeleteSamplers(...)           glDeleteSamplersNative(__VA_ARGS__)
#define glBindSampler(...)              glBindSamplerNative(__VA_ARGS__)
#define glSamplerParameteri(...)        glSamplerParameteriNative(__VA_ARGS__)
#define glShaderBinary(...)             glShaderBinaryNative(__VA_ARGS__)
#define glClearDepthf(...)              glClearDepthfNative(__VA_ARGS__)
#define glDrawElementsInstanced(...)    glDrawElementsInstancedNative(__VA_ARGS__)
#define glDrawArraysInstanced(...)      glDrawArraysInstancedNative(__VA_ARGS__)
#define glVertexAttribDivisor(...)      glVertexAttribDivisorNative(__VA_ARGS__)
#define glTexStorage2D(...)             glTexStorage2DNative(__VA_ARGS__)
#define glTexStorage2DMultisample(...)  glTexStorage2DMultisampleNative(__VA_ARGS__)
#define glTexStorage3D(...)             glTexStorage3DNative(__VA_ARGS__)
#define glTexSubImage3D(...)            glTexSubImage3DNative(__VA_ARGS__)
#define glCompressedTexSubImage2D(...)  glCompressedTexSubImage2DNative(__VA_ARGS__)
#define glCompressedTexSubImage3D(...)  glCompressedTexSubImage3DNative(__VA_ARGS__)
#define glGetInternalformativ(...)      glGetInternalformativNative(__VA_ARGS__)

#define DECLARE_ALL_API_PROCS                                                                   \
DECLARE_API_PROC(PFNGLGETSTRINGIPROC, glGetStringi);                                            \
DECLARE_API_PROC(PFNGLCREATEPROGRAMPROC, glCreateProgram);                                      \
DECLARE_API_PROC(PFNGLDELETEPROGRAMPROC, glDeleteProgram);                                      \
DECLARE_API_PROC(PFNGLUSEPROGRAMPROC, glUseProgram);                                            \
DECLARE_API_PROC(PFNGLPROGRAMBINARYPROC, glProgramBinary);                                      \
DECLARE_API_PROC(PFNGLGETPROGRAMBINARYPROC, glGetProgramBinary);                                \
DECLARE_API_PROC(PFNGLATTACHSHADERPROC, glAttachShader);                                        \
DECLARE_API_PROC(PFNGLDETACHSHADERPROC, glDetachShader);                                        \
DECLARE_API_PROC(PFNGLLINKPROGRAMPROC, glLinkProgram);                                          \
DECLARE_API_PROC(PFNGLGETPROGRAMIVPROC, glGetProgramiv);                                        \
DECLARE_API_PROC(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);                                \
DECLARE_API_PROC(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);                            \
DECLARE_API_PROC(PFNGLUNIFORM1IPROC, glUniform1i);                                              \
DECLARE_API_PROC(PFNGLUNIFORM1IVPROC, glUniform1iv);                                            \
DECLARE_API_PROC(PFNGLUNIFORM2IVPROC, glUniform2iv);                                            \
DECLARE_API_PROC(PFNGLUNIFORM3IVPROC, glUniform3iv);                                            \
DECLARE_API_PROC(PFNGLUNIFORM4IVPROC, glUniform4iv);                                            \
DECLARE_API_PROC(PFNGLUNIFORM1FPROC, glUniform1f);                                              \
DECLARE_API_PROC(PFNGLUNIFORM1FVPROC, glUniform1fv);                                            \
DECLARE_API_PROC(PFNGLUNIFORM2FVPROC, glUniform2fv);                                            \
DECLARE_API_PROC(PFNGLUNIFORM3FVPROC, glUniform3fv);                                            \
DECLARE_API_PROC(PFNGLUNIFORM4FVPROC, glUniform4fv);                                            \
DECLARE_API_PROC(PFNGLUNIFORMMATRIX2FVPROC, glUniformMatrix2fv);                                \
DECLARE_API_PROC(PFNGLUNIFORMMATRIX3FVPROC, glUniformMatrix3fv);                                \
DECLARE_API_PROC(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);                                \
DECLARE_API_PROC(PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation);                              \
DECLARE_API_PROC(PFNGLVERTEXATTRIB1FPROC, glVertexAttrib1f);                                    \
DECLARE_API_PROC(PFNGLVERTEXATTRIB1FVPROC, glVertexAttrib1fv);                                  \
DECLARE_API_PROC(PFNGLVERTEXATTRIB2FVPROC, glVertexAttrib2fv);                                  \
DECLARE_API_PROC(PFNGLVERTEXATTRIB3FVPROC, glVertexAttrib3fv);                                  \
DECLARE_API_PROC(PFNGLVERTEXATTRIB4FVPROC, glVertexAttrib4fv);                                  \
DECLARE_API_PROC(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);                  \
DECLARE_API_PROC(PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);                            \
DECLARE_API_PROC(PFNGLGETACTIVEUNIFORMPROC, glGetActiveUniform);                                \
DECLARE_API_PROC(PFNGLGETACTIVEATTRIBPROC, glGetActiveAttrib);                                  \
DECLARE_API_PROC(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);                              \
DECLARE_API_PROC(PFNGLCREATESHADERPROC, glCreateShader);                                        \
DECLARE_API_PROC(PFNGLDELETESHADERPROC, glDeleteShader);                                        \
DECLARE_API_PROC(PFNGLSHADERSOURCEPROC, glShaderSource);                                        \
DECLARE_API_PROC(PFNGLCOMPILESHADERPROC, glCompileShader);                                      \
DECLARE_API_PROC(PFNGLGETSHADERIVPROC, glGetShaderiv);                                          \
DECLARE_API_PROC(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);                                  \
DECLARE_API_PROC(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);                                  \
DECLARE_API_PROC(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);                            \
DECLARE_API_PROC(PFNGLGENBUFFERSPROC, glGenBuffers);                                            \
DECLARE_API_PROC(PFNGLBINDBUFFERPROC, glBindBuffer);                                            \
DECLARE_API_PROC(PFNGLBUFFERDATAPROC, glBufferData);                                            \
DECLARE_API_PROC(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);                          \
DECLARE_API_PROC(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);                                  \
DECLARE_API_PROC(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);                                  \
DECLARE_API_PROC(PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer);                                  \
DECLARE_API_PROC(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);                            \
DECLARE_API_PROC(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);                        \
DECLARE_API_PROC(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);                                      \
DECLARE_API_PROC(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);                    \
DECLARE_API_PROC(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);                                \
DECLARE_API_PROC(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer);                                \
DECLARE_API_PROC(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage);                          \
DECLARE_API_PROC(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample);    \
DECLARE_API_PROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer);                  \
DECLARE_API_PROC(PFNGLDRAWBUFFERSPROC, glDrawBuffers);                                          \
DECLARE_API_PROC(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);                          \
DECLARE_API_PROC(PFNGLACTIVETEXTUREPROC, glActiveTexture);                                      \
DECLARE_API_PROC(PFNGLBLENDEQUATIONPROC, glBlendEquation);                                      \
DECLARE_API_PROC(PFNGLBLENDEQUATIONSEPARATEPROC, glBlendEquationSeparate);                      \
DECLARE_API_PROC(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate);                              \
DECLARE_API_PROC(PFNGLBLENDCOLORPROC, glBlendColor);                              \
DECLARE_API_PROC(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap);                                    \
DECLARE_API_PROC(PFNGLCLEARDEPTHFPROC, glClearDepthf);                                          \
DECLARE_API_PROC(PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D);                        \
DECLARE_API_PROC(PFNGLGENSAMPLERSPROC, glGenSamplers);                                          \
DECLARE_API_PROC(PFNGLDELETESAMPLERSPROC, glDeleteSamplers);                                    \
DECLARE_API_PROC(PFNGLBINDSAMPLERPROC, glBindSampler);                                          \
DECLARE_API_PROC(PFNGLSAMPLERPARAMETERIPROC, glSamplerParameteri);                              \
DECLARE_API_PROC(PFNGLSHADERBINARYPROC, glShaderBinary);                                        \
DECLARE_API_PROC(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced);                      \
DECLARE_API_PROC(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced);                          \
DECLARE_API_PROC(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor);                          \
DECLARE_API_PROC(PFNGLTEXSTORAGE2DPROC, glTexStorage2D);                                        \
DECLARE_API_PROC(PFNGLTEXSTORAGE2DMULTISAMPLEPROC, glTexStorage2DMultisample);                  \
DECLARE_API_PROC(PFNGLTEXSTORAGE3DPROC, glTexStorage3D);                                        \
DECLARE_API_PROC(PFNGLTEXSUBIMAGE3DPROC, glTexSubImage3D);                                      \
DECLARE_API_PROC(PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, glCompressedTexSubImage2D);                  \
DECLARE_API_PROC(PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC, glCompressedTexSubImage3D);                  \
DECLARE_API_PROC(PFNGLGETINTERNALFORMATIVPROC, glGetInternalformativ);                          \

#define LOAD_ALL_API_PROCS(CONTEXT)                                                               \
LOAD_API_PROC(CONTEXT, PFNGLGETSTRINGIPROC, glGetStringi);                                        \
LOAD_API_PROC(CONTEXT, PFNGLCREATEPROGRAMPROC, glCreateProgram);                                  \
LOAD_API_PROC(CONTEXT, PFNGLDELETEPROGRAMPROC, glDeleteProgram);                                  \
LOAD_API_PROC(CONTEXT, PFNGLUSEPROGRAMPROC, glUseProgram);                                        \
LOAD_API_PROC(CONTEXT, PFNGLPROGRAMBINARYPROC, glProgramBinary);                                  \
LOAD_API_PROC(CONTEXT, PFNGLGETPROGRAMBINARYPROC, glGetProgramBinary);                            \
LOAD_API_PROC(CONTEXT, PFNGLATTACHSHADERPROC, glAttachShader);                                    \
LOAD_API_PROC(CONTEXT, PFNGLDETACHSHADERPROC, glDetachShader);                                    \
LOAD_API_PROC(CONTEXT, PFNGLLINKPROGRAMPROC, glLinkProgram);                                      \
LOAD_API_PROC(CONTEXT, PFNGLGETPROGRAMIVPROC, glGetProgramiv);                                    \
LOAD_API_PROC(CONTEXT, PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);                            \
LOAD_API_PROC(CONTEXT, PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM1IPROC, glUniform1i);                                          \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM1IVPROC, glUniform1iv);                                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM2IVPROC, glUniform2iv);                                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM3IVPROC, glUniform3iv);                                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM4IVPROC, glUniform4iv);                                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM1FPROC, glUniform1f);                                          \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM1FVPROC, glUniform1fv);                                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM2FVPROC, glUniform2fv);                                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM3FVPROC, glUniform3fv);                                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORM4FVPROC, glUniform4fv);                                        \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORMMATRIX2FVPROC, glUniformMatrix2fv);                            \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORMMATRIX3FVPROC, glUniformMatrix3fv);                            \
LOAD_API_PROC(CONTEXT, PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);                            \
LOAD_API_PROC(CONTEXT, PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation);                          \
LOAD_API_PROC(CONTEXT, PFNGLVERTEXATTRIB1FPROC, glVertexAttrib1f);                                \
LOAD_API_PROC(CONTEXT, PFNGLVERTEXATTRIB1FVPROC, glVertexAttrib1fv);                              \
LOAD_API_PROC(CONTEXT, PFNGLVERTEXATTRIB2FVPROC, glVertexAttrib2fv);                              \
LOAD_API_PROC(CONTEXT, PFNGLVERTEXATTRIB3FVPROC, glVertexAttrib3fv);                              \
LOAD_API_PROC(CONTEXT, PFNGLVERTEXATTRIB4FVPROC, glVertexAttrib4fv);                              \
LOAD_API_PROC(CONTEXT, PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);              \
LOAD_API_PROC(CONTEXT, PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);                        \
LOAD_API_PROC(CONTEXT, PFNGLGETACTIVEUNIFORMPROC, glGetActiveUniform);                            \
LOAD_API_PROC(CONTEXT, PFNGLGETACTIVEATTRIBPROC, glGetActiveAttrib);                              \
LOAD_API_PROC(CONTEXT, PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);                          \
LOAD_API_PROC(CONTEXT, PFNGLCREATESHADERPROC, glCreateShader);                                    \
LOAD_API_PROC(CONTEXT, PFNGLDELETESHADERPROC, glDeleteShader);                                    \
LOAD_API_PROC(CONTEXT, PFNGLSHADERSOURCEPROC, glShaderSource);                                    \
LOAD_API_PROC(CONTEXT, PFNGLCOMPILESHADERPROC, glCompileShader);                                  \
LOAD_API_PROC(CONTEXT, PFNGLGETSHADERIVPROC, glGetShaderiv);                                      \
LOAD_API_PROC(CONTEXT, PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);                              \
LOAD_API_PROC(CONTEXT, PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);                              \
LOAD_API_PROC(CONTEXT, PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);                        \
LOAD_API_PROC(CONTEXT, PFNGLGENBUFFERSPROC, glGenBuffers);                                        \
LOAD_API_PROC(CONTEXT, PFNGLBINDBUFFERPROC, glBindBuffer);                                        \
LOAD_API_PROC(CONTEXT, PFNGLBUFFERDATAPROC, glBufferData);                                        \
LOAD_API_PROC(CONTEXT, PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);                      \
LOAD_API_PROC(CONTEXT, PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);                              \
LOAD_API_PROC(CONTEXT, PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);                              \
LOAD_API_PROC(CONTEXT, PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer);                              \
LOAD_API_PROC(CONTEXT, PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);                        \
LOAD_API_PROC(CONTEXT, PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);                    \
LOAD_API_PROC(CONTEXT, PFNGLDELETEBUFFERSPROC, glDeleteBuffers);                                  \
LOAD_API_PROC(CONTEXT, PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);                \
LOAD_API_PROC(CONTEXT, PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);                            \
LOAD_API_PROC(CONTEXT, PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer);                            \
LOAD_API_PROC(CONTEXT, PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage);                      \
LOAD_API_PROC(CONTEXT, PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample);\
LOAD_API_PROC(CONTEXT, PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer);              \
LOAD_API_PROC(CONTEXT, PFNGLDRAWBUFFERSPROC, glDrawBuffers);                                      \
LOAD_API_PROC(CONTEXT, PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);                      \
LOAD_API_PROC(CONTEXT, PFNGLACTIVETEXTUREPROC, glActiveTexture);                                  \
LOAD_API_PROC(CONTEXT, PFNGLBLENDEQUATIONPROC, glBlendEquation);                                  \
LOAD_API_PROC(CONTEXT, PFNGLBLENDEQUATIONSEPARATEPROC, glBlendEquationSeparate);                  \
LOAD_API_PROC(CONTEXT, PFNGLBLENDCOLORPROC, glBlendColor);                                        \
LOAD_API_PROC(CONTEXT, PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate);                          \
LOAD_API_PROC(CONTEXT, PFNGLGENERATEMIPMAPPROC, glGenerateMipmap);                                \
LOAD_API_PROC(CONTEXT, PFNGLCLEARDEPTHFPROC, glClearDepthf);                                      \
LOAD_API_PROC(CONTEXT, PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D);                    \
LOAD_API_PROC(CONTEXT, PFNGLGENSAMPLERSPROC, glGenSamplers);                                      \
LOAD_API_PROC(CONTEXT, PFNGLDELETESAMPLERSPROC, glDeleteSamplers);                                \
LOAD_API_PROC(CONTEXT, PFNGLBINDSAMPLERPROC, glBindSampler);                                      \
LOAD_API_PROC(CONTEXT, PFNGLSAMPLERPARAMETERIPROC, glSamplerParameteri);                          \
LOAD_API_PROC(CONTEXT, PFNGLSHADERBINARYPROC, glShaderBinary);                                    \
LOAD_API_PROC(CONTEXT, PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced);                  \
LOAD_API_PROC(CONTEXT, PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced);                      \
LOAD_API_PROC(CONTEXT, PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor);                      \
LOAD_API_PROC(CONTEXT, PFNGLTEXSTORAGE2DPROC, glTexStorage2D);                                    \
LOAD_API_PROC(CONTEXT, PFNGLTEXSTORAGE2DMULTISAMPLEPROC, glTexStorage2DMultisample);                                    \
LOAD_API_PROC(CONTEXT, PFNGLTEXSTORAGE3DPROC, glTexStorage3D);                                    \
LOAD_API_PROC(CONTEXT, PFNGLTEXSUBIMAGE3DPROC, glTexSubImage3D);                                  \
LOAD_API_PROC(CONTEXT, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, glCompressedTexSubImage2D);              \
LOAD_API_PROC(CONTEXT, PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC, glCompressedTexSubImage3D);              \
LOAD_API_PROC(CONTEXT, PFNGLGETINTERNALFORMATIVPROC, glGetInternalformativ);                      \

//In WGL (Windows), all api procs are static and need explicit definition in a source file
#define DEFINE_ALL_API_PROCS                                                                   \
DEFINE_API_PROC(PFNGLGETSTRINGIPROC, glGetStringi);                                            \
DEFINE_API_PROC(PFNGLCREATEPROGRAMPROC, glCreateProgram);                                      \
DEFINE_API_PROC(PFNGLDELETEPROGRAMPROC, glDeleteProgram);                                      \
DEFINE_API_PROC(PFNGLUSEPROGRAMPROC, glUseProgram);                                            \
DEFINE_API_PROC(PFNGLPROGRAMBINARYPROC, glProgramBinary);                                      \
DEFINE_API_PROC(PFNGLGETPROGRAMBINARYPROC, glGetProgramBinary);                                \
DEFINE_API_PROC(PFNGLATTACHSHADERPROC, glAttachShader);                                        \
DEFINE_API_PROC(PFNGLDETACHSHADERPROC, glDetachShader);                                        \
DEFINE_API_PROC(PFNGLLINKPROGRAMPROC, glLinkProgram);                                          \
DEFINE_API_PROC(PFNGLGETPROGRAMIVPROC, glGetProgramiv);                                        \
DEFINE_API_PROC(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);                                \
DEFINE_API_PROC(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);                            \
DEFINE_API_PROC(PFNGLUNIFORM1IPROC, glUniform1i);                                              \
DEFINE_API_PROC(PFNGLUNIFORM1IVPROC, glUniform1iv);                                            \
DEFINE_API_PROC(PFNGLUNIFORM2IVPROC, glUniform2iv);                                            \
DEFINE_API_PROC(PFNGLUNIFORM3IVPROC, glUniform3iv);                                            \
DEFINE_API_PROC(PFNGLUNIFORM4IVPROC, glUniform4iv);                                            \
DEFINE_API_PROC(PFNGLUNIFORM1FPROC, glUniform1f);                                              \
DEFINE_API_PROC(PFNGLUNIFORM1FVPROC, glUniform1fv);                                            \
DEFINE_API_PROC(PFNGLUNIFORM2FVPROC, glUniform2fv);                                            \
DEFINE_API_PROC(PFNGLUNIFORM3FVPROC, glUniform3fv);                                            \
DEFINE_API_PROC(PFNGLUNIFORM4FVPROC, glUniform4fv);                                            \
DEFINE_API_PROC(PFNGLUNIFORMMATRIX2FVPROC, glUniformMatrix2fv);                                \
DEFINE_API_PROC(PFNGLUNIFORMMATRIX3FVPROC, glUniformMatrix3fv);                                \
DEFINE_API_PROC(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);                                \
DEFINE_API_PROC(PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation);                              \
DEFINE_API_PROC(PFNGLVERTEXATTRIB1FPROC, glVertexAttrib1f);                                    \
DEFINE_API_PROC(PFNGLVERTEXATTRIB1FVPROC, glVertexAttrib1fv);                                  \
DEFINE_API_PROC(PFNGLVERTEXATTRIB2FVPROC, glVertexAttrib2fv);                                  \
DEFINE_API_PROC(PFNGLVERTEXATTRIB3FVPROC, glVertexAttrib3fv);                                  \
DEFINE_API_PROC(PFNGLVERTEXATTRIB4FVPROC, glVertexAttrib4fv);                                  \
DEFINE_API_PROC(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);                  \
DEFINE_API_PROC(PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);                            \
DEFINE_API_PROC(PFNGLGETACTIVEUNIFORMPROC, glGetActiveUniform);                                \
DEFINE_API_PROC(PFNGLGETACTIVEATTRIBPROC, glGetActiveAttrib);                                  \
DEFINE_API_PROC(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);                              \
DEFINE_API_PROC(PFNGLCREATESHADERPROC, glCreateShader);                                        \
DEFINE_API_PROC(PFNGLDELETESHADERPROC, glDeleteShader);                                        \
DEFINE_API_PROC(PFNGLSHADERSOURCEPROC, glShaderSource);                                        \
DEFINE_API_PROC(PFNGLCOMPILESHADERPROC, glCompileShader);                                      \
DEFINE_API_PROC(PFNGLGETSHADERIVPROC, glGetShaderiv);                                          \
DEFINE_API_PROC(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);                                  \
DEFINE_API_PROC(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);                                  \
DEFINE_API_PROC(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);                            \
DEFINE_API_PROC(PFNGLGENBUFFERSPROC, glGenBuffers);                                            \
DEFINE_API_PROC(PFNGLBINDBUFFERPROC, glBindBuffer);                                            \
DEFINE_API_PROC(PFNGLBUFFERDATAPROC, glBufferData);                                            \
DEFINE_API_PROC(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);                          \
DEFINE_API_PROC(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);                                  \
DEFINE_API_PROC(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);                                  \
DEFINE_API_PROC(PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer);                                  \
DEFINE_API_PROC(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);                            \
DEFINE_API_PROC(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);                        \
DEFINE_API_PROC(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);                                      \
DEFINE_API_PROC(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);                    \
DEFINE_API_PROC(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);                                \
DEFINE_API_PROC(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer);                                \
DEFINE_API_PROC(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage);                          \
DEFINE_API_PROC(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample);    \
DEFINE_API_PROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer);                  \
DEFINE_API_PROC(PFNGLDRAWBUFFERSPROC, glDrawBuffers);                                          \
DEFINE_API_PROC(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);                          \
DEFINE_API_PROC(PFNGLACTIVETEXTUREPROC, glActiveTexture);                                      \
DEFINE_API_PROC(PFNGLBLENDEQUATIONPROC, glBlendEquation);                                      \
DEFINE_API_PROC(PFNGLBLENDEQUATIONSEPARATEPROC, glBlendEquationSeparate);                      \
DEFINE_API_PROC(PFNGLBLENDCOLORPROC, glBlendColor);                                            \
DEFINE_API_PROC(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate);                              \
DEFINE_API_PROC(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap);                                    \
DEFINE_API_PROC(PFNGLCLEARDEPTHFPROC, glClearDepthf);                                          \
DEFINE_API_PROC(PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D);                        \
DEFINE_API_PROC(PFNGLGENSAMPLERSPROC, glGenSamplers);                                          \
DEFINE_API_PROC(PFNGLDELETESAMPLERSPROC, glDeleteSamplers);                                    \
DEFINE_API_PROC(PFNGLBINDSAMPLERPROC, glBindSampler);                                          \
DEFINE_API_PROC(PFNGLSAMPLERPARAMETERIPROC, glSamplerParameteri);                              \
DEFINE_API_PROC(PFNGLSHADERBINARYPROC, glShaderBinary);                                        \
DEFINE_API_PROC(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced);                      \
DEFINE_API_PROC(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced);                          \
DEFINE_API_PROC(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor);                          \
DEFINE_API_PROC(PFNGLTEXSTORAGE2DPROC, glTexStorage2D);                                        \
DEFINE_API_PROC(PFNGLTEXSTORAGE2DMULTISAMPLEPROC, glTexStorage2DMultisample);                                        \
DEFINE_API_PROC(PFNGLTEXSTORAGE3DPROC, glTexStorage3D);                                        \
DEFINE_API_PROC(PFNGLTEXSUBIMAGE3DPROC, glTexSubImage3D);                                      \
DEFINE_API_PROC(PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, glCompressedTexSubImage2D);                  \
DEFINE_API_PROC(PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC, glCompressedTexSubImage3D);                  \
DEFINE_API_PROC(PFNGLGETINTERNALFORMATIVPROC, glGetInternalformativ);                          \

#endif
