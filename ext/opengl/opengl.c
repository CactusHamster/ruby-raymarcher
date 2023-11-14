#include <GL/gl.h>
#include <ruby.h>
#include "extconf.h"
VALUE MOD_GL;

/*
    use gl::{self, types::{GLenum, GLint, GLuint}, COMPILE_STATUS, INFO_LOG_LENGTH, GetShaderInfoLog, AttachShader, GetProgramiv, LINK_STATUS};
    use gl::{STATIC_DRAW, DrawElements};
*/
VALUE rb_gl_get_shader_info_log () {
    
}
VALUE rb_gl_attach_shader () {

}
VALUE rb_gl_get_program_iv () {

}

/// Loads the Ruby module. Makes GL real.
void Init_opengl(void) {
    MOD_GL = rb_define_module("GL");
    
    
}
int main () { return 0; }