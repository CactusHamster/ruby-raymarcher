#define GLFW_INCLUDE_NONE
// Include GL extension headers.
#define GLFW_INCLUDE_GLEXT
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <ruby.h>
#include "extconf.h"

VALUE CLASS_WINDOW;
VALUE MOD_GL;
VALUE MOD_GLFW;
/*
VALUE say_hi () {
    printf("%s", "hiiii\n");
    return RB_INT2NUM(0);
}

VALUE rb_create_window (VALUE width, VALUE height, VALUE title) {
    if (!RB_INTEGER_TYPE_P(width)) rb_raise(rb_eArgError, "Argument for `width` must be an integer.");
    if (!RB_INTEGER_TYPE_P(height)) rb_raise(rb_eArgError, "Argument for `height` must be an integer.");
    int width_int = NUM2INT(width);
    int height_int = NUM2INT(height);
    if (!RB_TYPE_P(title, T_STRING)) rb_raise(rb_eArgError, "Argument for `title` must be a string.");
    const char *title_str = StringValueCStr(title);
    GLFWwindow *window_temp = glfwCreateWindow(width_int, height_int, title_str, NULL, NULL);
    if (window_temp == NULL) rb_raise(rb_eTypeError, "Failed to create window.");
    window = window_temp;
    return ;
}
VALUE rb_destroy_window () {
    glfwDestroyWindow(window);
    window = NULL;
}
*/

void init_glfw_key_constants (VALUE module) {
    VALUE hash = rb_hash_new();
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
        const int scancode = glfwGetKeyScancode(key);
        if (scancode == -1) continue;
        const char* keyname = glfwGetKeyName(key, scancode);
        if (keyname == NULL) continue;
        VALUE symbol = ID2SYM(rb_intern(keyname));
        rb_hash_aset(hash, symbol, INT2FIX(key));
    }
    rb_define_const(module, "KEYS", hash);
}
static void error_callback(int code, const char* description) {
    printf("[ERR]: %i %s", code, description);
}
/// Initialize GLFW context.
static VALUE rb_glfw_init () {
    glfwSetErrorCallback(error_callback);
    int success = glfwInit();
    init_glfw_key_constants(MOD_GLFW);
    return success == GLFW_TRUE ? Qtrue : Qfalse;
}
/// Destroy GLFW context.
static VALUE rb_glfw_destroy () {
    glfwTerminate();
    return Qnil;
}
/// Poll for keys on the current window.
static VALUE rb_glfw_poll_events () {
    glfwPollEvents();
    return Qnil;
}
/// Sleep the thread until an event.
static VALUE rb_glfw_wait_for_events () {
    glfwWaitEvents();
    return Qnil;
}
static VALUE rb_glfw_get_scancode (VALUE key) {
    return INT2NUM(glfwGetKeyScancode(NUM2INT(key)));
}
/// Callback for GLFW key events.
static void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods) {
    VALUE rb_window = (VALUE) glfwGetWindowUserPointer(window);
    VALUE rb_key_event;
    switch (action) {
        case GLFW_PRESS:
            rb_key_event = ID2SYM(rb_intern("key_press"));
            break;
        case GLFW_RELEASE:
            rb_key_event = ID2SYM(rb_intern("key_release"));
            break;
        default:
            rb_key_event = ID2SYM(rb_intern("key_unknown"));
            break;
    }
    rb_funcall(rb_window, rb_key_event, 1, INT2NUM(key));
}



// CLass representing a GLFW window.
typedef struct {
    GLFWwindow* window;
} WindowData;
/// The size of the Ruby Window.
static size_t rb_window_size (const void* data) {
    return sizeof(WindowData);
}
/// Retrieve the WindowData struct from a Ruby Window object.
static GLFWwindow* window_from_self (VALUE rb_window) {
    WindowData* window = (WindowData*) DATA_PTR(rb_window);
    return window->window;
}
/// Deallocates Ruby Window when no longer in use.
static void rb_window_free (void* data) {
    free(data);
}
/// Ruby type for a Ruby Window.
static const rb_data_type_t rb_window_type = {
    .wrap_struct_name = "GLFWWindow",
    .function = {
        .dmark = NULL,
        .dfree = rb_window_free,
        .dsize = rb_window_size
    },
    .data = NULL,
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};
// Allocate a WindowData for a Ruby Window.
static VALUE rb_window_allocate (VALUE self) {
    // Allocate some memory for a WindowData struct.
    WindowData* windowdata = (WindowData *) malloc(sizeof(WindowData));
    return TypedData_Wrap_Struct(self, &rb_window_type, windowdata);
}
/// Initialize a WindowData and return a Ruby Window as a wrapper for it.
static VALUE rb_window_initialize (VALUE self, VALUE width, VALUE height, VALUE title) {
    WindowData* window;
    TypedData_Get_Struct(self, WindowData, &rb_window_type, window);
    int width_int = NUM2INT(width);
    int height_int = NUM2INT(height);
    const char *title_str = StringValueCStr(title);
    window->window = NULL;
    window->window = glfwCreateWindow(width_int, height_int, title_str, NULL, NULL);
    if (window->window == NULL) {
        free(window);
        rb_raise(rb_eRuntimeError, "Failed to create GLFW window.");
        return Qnil;
    }
    return self;
}
static VALUE rb_window_make_current (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    glfwMakeContextCurrent(window);
    return Qnil;
}
static VALUE rb_window_should_close (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    return glfwWindowShouldClose(window) == GLFW_TRUE ? Qtrue : Qfalse;
}
static VALUE rb_window_start_keyevents (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    glfwSetKeyCallback(window, key_callback);
    return Qnil;
}
static VALUE rb_window_get_key_state (VALUE self, VALUE key) {
    Check_Type(key, T_FIXNUM);
    GLFWwindow *window = window_from_self(self);
    return INT2NUM(glfwGetKey(window, NUM2INT(key)));
}

void Init_opengl() {
    // OpenGL related code.
    MOD_GL = rb_define_module("GL");
    // GLFW related code.
    MOD_GLFW = rb_define_module("GLFW");
        rb_define_module_function(MOD_GLFW, "init", rb_glfw_init, 0);
        rb_define_module_function(MOD_GLFW, "destroy", rb_glfw_destroy, 0);
        rb_define_module_function(MOD_GLFW, "poll_events", rb_glfw_poll_events, 0);
        rb_define_module_function(MOD_GLFW, "wait_for_events", rb_glfw_wait_for_events, 0);
        rb_define_module_function(MOD_GLFW, "get_scancode", rb_glfw_get_scancode, 1);
        CLASS_WINDOW = rb_define_class_under(MOD_GLFW, "Window", rb_cObject);
            rb_define_alloc_func(CLASS_WINDOW, rb_window_allocate);
            rb_define_method(CLASS_WINDOW, "initialize", rb_window_initialize, 3);
            rb_define_method(CLASS_WINDOW, "make_current", rb_window_make_current, 0);
            rb_define_method(CLASS_WINDOW, "should_close", rb_window_should_close, 0);
            rb_define_method(CLASS_WINDOW, "start_keyevents", rb_window_start_keyevents, 0);
            rb_define_method(CLASS_WINDOW, "get_key_state", rb_window_get_key_state, 1);
    // module, ruby method name, c function, number of arguments
    //rb_define_method(MOD_GLFW, "init", rb_init_glfw, 0);
    //rb_define_method(MOD_GLFW, "window", rb_create_window, 3);
    
}
int main () { return 0; }