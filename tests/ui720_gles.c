// Compile the actual SGSR body under GLES2, including the no-highp branch.
#include "gfx_sgsr.h"
#include "gfx_upscale.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <assert.h>
#include <stdio.h>
static GLuint shader(GLenum type, const char *source) {
  GLuint s=glCreateShader(type);glShaderSource(s,1,&source,NULL);glCompileShader(s);
  GLint ok;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
  if(!ok) {char log[2048];glGetShaderInfoLog(s,sizeof log,NULL,log);puts(log);}
  assert(ok);return s;
}
int main(void) {
  EGLDisplay d=eglGetDisplay(EGL_DEFAULT_DISPLAY);EGLint n;EGLConfig c;
  const EGLint ca[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_NONE};
  const EGLint sa[]={EGL_WIDTH,16,EGL_HEIGHT,16,EGL_NONE},xa[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};
  assert(eglInitialize(d,NULL,NULL)&&eglChooseConfig(d,ca,&c,1,&n)&&n);
  EGLSurface surface=eglCreatePbufferSurface(d,c,sa);
  EGLContext context=eglCreateContext(d,c,EGL_NO_CONTEXT,xa);
  assert(eglMakeCurrent(d,surface,surface,context));
  for(int kind=0;kind<2;kind++) for(int low=0;low<2;low++) {
    const char *parts[]={low?"#define NV_SGSR_MEDIUMP\n":"",
      "precision mediump float;\n#if defined(GL_FRAGMENT_PRECISION_HIGH) && !defined(NV_SGSR_MEDIUMP)\n"
      "varying highp vec2 vUv; uniform highp vec2 uPar;\n#else\n"
      "varying mediump vec2 vUv; uniform mediump vec2 uPar;\n#endif\n"
      "uniform sampler2D uTex; uniform float uFoco;\n",
      kind?NV_SHARP_BILINEAR_SHADER:NV_SGSR_SHADER};
    GLuint f=glCreateShader(GL_FRAGMENT_SHADER);glShaderSource(f,3,parts,NULL);glCompileShader(f);
    GLint ok;glGetShaderiv(f,GL_COMPILE_STATUS,&ok);
    if(!ok) {char log[2048];glGetShaderInfoLog(f,sizeof log,NULL,log);puts(log);}assert(ok);
    const char *vs=low?"attribute vec2 p; varying mediump vec2 vUv; void main(){vUv=p;gl_Position=vec4(p,0.,1.);}":
      "attribute vec2 p; varying highp vec2 vUv; void main(){vUv=p;gl_Position=vec4(p,0.,1.);}";
    GLuint p=glCreateProgram();glAttachShader(p,shader(GL_VERTEX_SHADER,vs));glAttachShader(p,f);
    glLinkProgram(p);glGetProgramiv(p,GL_LINK_STATUS,&ok);assert(ok);
    printf("PASS GLES2 %s %s compile/link (%s)\n",kind?"Sharp-Bilinear":"SGSR",low?"mediump":"highp",glGetString(GL_RENDERER));
  }
  return 0;
}
