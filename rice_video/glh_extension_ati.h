#ifndef _GLH_EXT_ATI_H_
#define _GLH_EXT_ATI_H_



/*
**  ATI EXTENSIONS
*/



/*
** GL_ARB_fragment_program
**
** Support:
**   Rage 128 based     : Not Supported
**   Radeon 7xxx based  : Not Supported
**   Radeon 8xxx based  : Not Supported
**   Radeon 9xxx based  : Supported
*/

typedef GLvoid (APIENTRY *PFNGLPROGRAMSTRINGARBPROC)(GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef GLvoid (APIENTRY *PFNGLBINDPROGRAMARBPROC)(GLenum target, GLuint program);
typedef GLvoid (APIENTRY *PFNGLDELETEPROGRAMSARBPROC)(GLsizei n, const GLuint *programs);
typedef GLvoid (APIENTRY *PFNGLGENPROGRAMSARBPROC)(GLsizei n, GLuint *programs);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4FARBPROC)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4DARBPROC)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4FVARBPROC)(GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4DVARBPROC)(GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FARBPROC)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DARBPROC)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)(GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)(GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMENVPARAMETERFVARBPROC)(GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMENVPARAMETERDVARBPROC)(GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)(GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)(GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMIVARBPROC)(GLenum target, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMSTRINGARBPROC)(GLenum target, GLenum pname, GLvoid *string);
typedef GLvoid (APIENTRY *PFNGLGETVERTEXATTRIBDVARBPROC)(GLuint index, GLenum pname, GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETVERTEXATTRIBFVARBPROC)(GLuint index, GLenum pname, GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETVERTEXATTRIBIVARBPROC)(GLuint index, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *PFNGLGETVERTEXATTRIBPOINTERVARBPROC)(GLuint index, GLenum pname, GLvoid **pointer);
typedef GLboolean (APIENTRY *PFNGLISPROGRAMARBPROC)(GLuint program);

extern GLboolean GL_ARB_fragment_program_Flag;
extern PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC glBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
extern PFNGLPROGRAMENVPARAMETER4DARBPROC glProgramEnvParameter4dARB;
extern PFNGLPROGRAMENVPARAMETER4DVARBPROC glProgramEnvParameter4dvARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB;
extern PFNGLPROGRAMENVPARAMETER4FVARBPROC glProgramEnvParameter4fvARB;
extern PFNGLPROGRAMLOCALPARAMETER4DARBPROC glProgramLocalParameter4dARB;
extern PFNGLPROGRAMLOCALPARAMETER4DVARBPROC glProgramLocalParameter4dvARB;
extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB;
extern PFNGLGETPROGRAMENVPARAMETERDVARBPROC glGetProgramEnvParameterdvARB;
extern PFNGLGETPROGRAMENVPARAMETERFVARBPROC glGetProgramEnvParameterfvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC glGetProgramLocalParameterdvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC glGetProgramLocalParameterfvARB;
extern PFNGLGETPROGRAMIVARBPROC glGetProgramivARB;
extern PFNGLGETPROGRAMSTRINGARBPROC glGetProgramStringARB;
extern PFNGLISPROGRAMARBPROC glIsProgramARB;


#endif // _GLH_EXT_ATI_H_
