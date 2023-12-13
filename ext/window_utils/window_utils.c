#include <ruby.h>
#include <ruby/io/buffer.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "window_utils.h"
#include <GL/gl.h>
#include <GL/glext.h>
#define GLFW_INCLUDE_GLEXT
#include "extconf.h"

// Debug print
#ifdef DEBUG
#define PRINT_DEBUG(fmt, ...) \
    fprintf(stderr, fmt, __VA_ARGS__)
#else
#define PRINT_DEBUG(fmt, ...) \
    do { } while (0)
#endif

// Global types.
VALUE CLASS_KEY_EVENT = Qnil;
VALUE CLASS_BUFFER = Qnil;
int GLFW_INITIALIZED = FALSE;

/*
    Start of GLFW constant init.
*/
#define KEY_COUNT 122
#define KEYNAME_LENGTH 13
const char key_names[KEY_COUNT][KEYNAME_LENGTH] = {"UNKNOWN","SPACE","APOSTROPHE","COMMA","MINUS","PERIOD","SLASH","0","1","2","3","4","5","6","7","8","9","SEMICOLON","EQUAL","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","LEFT_BRACKET","BACKSLASH","RIGHT_BRACKET","GRAVE_ACCENT","WORLD_1","WORLD_2","ESCAPE","ENTER","TAB","BACKSPACE","INSERT","DELETE","RIGHT","LEFT","DOWN","UP","PAGE_UP","PAGE_DOWN","HOME","END","CAPS_LOCK","SCROLL_LOCK","NUM_LOCK","PRINT_SCREEN","PAUSE","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12","F13","F14","F15","F16","F17","F18","F19","F20","F21","F22","F23","F24","F25","KP_0","KP_1","KP_2","KP_3","KP_4","KP_5","KP_6","KP_7","KP_8","KP_9","KP_DECIMAL","KP_DIVIDE","KP_MULTIPLY","KP_SUBTRACT","KP_ADD","KP_ENTER","KP_EQUAL","LEFT_SHIFT","LEFT_CONTROL","LEFT_ALT","LEFT_SUPER","RIGHT_SHIFT","RIGHT_CONTROL","RIGHT_ALT","RIGHT_SUPER","MENU","LAST"};
const int key_values[KEY_COUNT] = {-1,32,39,44,45,46,47,48,49,50,51,52,53,54,55,56,57,59,61,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,96,161,162,256,257,258,259,260,261,262,263,264,265,266,267,268,269,280,281,282,283,284,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,306,307,308,309,310,311,312,313,314,320,321,322,323,324,325,326,327,328,329,330,331,332,333,334,335,336,340,341,342,343,344,345,346,347,348,348};
VALUE define_glfw_keys_under (VALUE module) {
    VALUE hash = rb_hash_new_capa(KEY_COUNT);
    for (int i = 0; i < KEY_COUNT; i++) {
        VALUE symbol = ID2SYM(rb_intern(key_names[i]));
        VALUE value = INT2NUM(key_values[i]);
        rb_hash_aset(hash, symbol, value);
    }
    rb_define_const(module, "KEYS", hash);
}
#define MOD_COUNT 6
#define MODNAME_LENGTH 9
const char mod_names[MOD_COUNT][MODNAME_LENGTH] = {"SHIFT","CONTROL","ALT","SUPER","CAPS_LOCK","NUM_LOCK"};
const int mod_values[MOD_COUNT] = {0x0001,0x0002,0x0004,0x0008,0x0010,0x0020};
VALUE define_glfw_key_mods_under (VALUE module) {
    VALUE hash = rb_hash_new_capa(MOD_COUNT);
    for (int i = 0; i < MOD_COUNT; i++) {
        VALUE symbol = ID2SYM(rb_intern(mod_names[i]));
        VALUE value = INT2NUM(mod_values[i]);
        rb_hash_aset(hash, symbol, value);
    }
    rb_define_const(module, "KEYMODS", hash);
}
/*
    End of GLFW constant init.
*/
/*
    Start of KeyEvent.
*/
typedef struct {
    int key;
    int scancode;
    int action;
    int mods;
} KeyEventData;
size_t rb_keyevent_size (const void* ptr) { return sizeof(KeyEventData); }
void rb_keyevent_free (void* ptr) { xfree(ptr); }
const rb_data_type_t rb_keyevent_type = {
    .wrap_struct_name = "RB_GLFW_KeyEvent",
    .function = {
        .dfree = rb_keyevent_free,
        .dsize = rb_keyevent_size,
        .dmark = NULL
    },
    .data = NULL,
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};
VALUE rb_keyevent_allocate (VALUE self) {
    KeyEventData* kevent;
    return TypedData_Make_Struct(self, KeyEventData, &rb_keyevent_type, kevent);
}
VALUE rb_keyevent_initialize (VALUE self, VALUE key, VALUE scancode, VALUE action, VALUE mods) {
    KeyEventData* kevent;
    TypedData_Get_Struct(self, KeyEventData, &rb_keyevent_type, kevent);
    //@TODO: check types (teehee :3c)
    kevent->key = NUM2INT(key);
    kevent->scancode = NUM2INT(scancode);
    kevent->action = NUM2INT(action);
    kevent->mods = NUM2INT(mods);
    return self;
}
// dont try this at home bleee :3
VALUE rb_keyevent_get_key (VALUE self) { return INT2NUM(((KeyEventData*) DATA_PTR(self))->key); }
VALUE rb_keyevent_get_scancode (VALUE self) { return INT2NUM(((KeyEventData*) DATA_PTR(self))->scancode); }
VALUE rb_keyevent_get_action (VALUE self) { return INT2NUM(((KeyEventData*) DATA_PTR(self))->action); }
VALUE rb_keyevent_get_mods (VALUE self) { return INT2NUM(((KeyEventData*) DATA_PTR(self))->mods); }
VALUE init_keyevent_under (VALUE module) {
    VALUE key_event_class = rb_define_class_under(module, "KeyEvent", rb_cObject);
    rb_define_alloc_func(key_event_class, rb_keyevent_allocate);
    rb_define_method(key_event_class, "initialize", rb_keyevent_initialize, 4);
    rb_define_attr(key_event_class, "key", 1, 0);
    rb_define_attr(key_event_class, "scancode", 1, 0);
    rb_define_attr(key_event_class, "action", 1, 0);
    rb_define_attr(key_event_class, "mods", 1, 0);
    rb_define_method(key_event_class, "key", rb_keyevent_get_key, 0);
    rb_define_method(key_event_class, "scancode", rb_keyevent_get_scancode, 0);
    rb_define_method(key_event_class, "action", rb_keyevent_get_action, 0);
    rb_define_method(key_event_class, "mods", rb_keyevent_get_mods, 0);
    return key_event_class;
}
/*
    End of KeyEvent.
*/
/*
    Start of Window.
*/
/// CLass representing a GLFW window.
typedef struct {
    GLFWwindow* window;
    VALUE observers;
    unsigned int destroyed;
} WindowData;
/// The size of the Ruby Window.
size_t rb_window_size (const void* data) {
    return sizeof(WindowData);
}
/// Retrieve the WindowData struct from a Ruby Window object.
GLFWwindow* window_from_self (VALUE rb_window) {
    WindowData* window = (WindowData*) DATA_PTR(rb_window);
    return window->window;
}
/// Deallocates Ruby Window when no longer in use.
void rb_window_free (void* ptr) { 
    WindowData* win_data = (WindowData*) ptr;
    if (win_data->destroyed == FALSE) glfwDestroyWindow(win_data->window);
    free(win_data->window);
    xfree(ptr);
}
/// Ensure the observers aren't killed.
void rb_window_mark (void* ptr) {
    WindowData* win_data = (WindowData*) ptr;
    rb_gc_mark_movable(win_data->observers);
}
void rb_window_compact (void *ptr) {
    WindowData* win_data = (WindowData*) ptr;
    win_data->observers = rb_gc_location(win_data->observers);
}
/// Ruby type for a WindowData.
const rb_data_type_t rb_window_type = {
    .wrap_struct_name = "RB_GLFW_Window",
    .function = {
        .dmark = rb_window_mark,
        .dfree = rb_window_free,
        .dsize = rb_window_size
    },
    .data = NULL,
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};
/// Allocate a WindowData for a Ruby Window.
VALUE rb_window_allocate (VALUE self) {
    // WindowData* windowdata = (WindowData*) malloc(sizeof(WindowData));
    WindowData* windowdata;
    VALUE obj = TypedData_Make_Struct(self, WindowData, &rb_window_type, windowdata);
    if (windowdata == NULL) {
        free(windowdata);
        rb_raise(rb_eNoMemError, "Failed to allocate internal WindowData.");
        return Qnil;
    }
    return obj;
}
/// Initialize a WindowData and return a Ruby Window as a wrapper for it.
VALUE rb_window_initialize (VALUE self, VALUE width, VALUE height, VALUE title) {
    WindowData* window_data;
    TypedData_Get_Struct(self, WindowData, &rb_window_type, window_data);
    //@TODO: Check types.
    int width_int = NUM2INT(width);
    int height_int = NUM2INT(height);
    const char *title_str = StringValueCStr(title);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    window_data->window = glfwCreateWindow(width_int, height_int, title_str, NULL, NULL);
    if (window_data->window == NULL) {
        rb_raise(rb_eRuntimeError, "Failed to create GLFW window. Did you forget to init GLFW?");
        return Qnil;
    }
    window_data->destroyed = FALSE;
    window_data->observers = rb_ary_new();
    glfwSetWindowUserPointer(window_data->window, (void*)self);
    return self;
}

/// Focus rendering and events on the Ruby Window.
VALUE rb_window_make_current (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    glfwMakeContextCurrent(window);
    return Qnil;
}
// Destroy the Window.
VALUE rb_window_destroy (VALUE self) {
    WindowData* window_data;
    TypedData_Get_Struct(self, WindowData, &rb_window_type, window_data);
    window_data->destroyed = TRUE;
    glfwDestroyWindow(window_data->window);
    return Qnil;
}
/// Boolean; True if window has been requested to close.
VALUE rb_window_should_close (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    return glfwWindowShouldClose(window) == GLFW_TRUE ? Qtrue : Qfalse;
}
/// Return the size of the window frame.
VALUE rb_window_get_framebuffer_size (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    VALUE result[] = { INT2NUM(width), INT2NUM(height) };
    VALUE result_ary = rb_ary_new4(2, result);
    return result_ary;
}
/// Add an event listener object.
VALUE rb_window_add_observer (VALUE self, VALUE observer) {
    WindowData* windowdata;
    TypedData_Get_Struct(self, WindowData, &rb_window_type, windowdata);
    rb_ary_push(windowdata->observers, observer);
    return Qnil;
}
/// Delete an event listener object.
VALUE rb_window_del_observer (VALUE self, VALUE observer) {
    WindowData* windowdata;
    TypedData_Get_Struct(self, WindowData, &rb_window_type, windowdata);
    VALUE result = rb_ary_delete(windowdata->observers, observer);
    return result;
}
/// Emit an event to compatible listeners.
VALUE rb_window_emit (VALUE self, VALUE event, VALUE data) {
    if (!RB_TYPE_P(event, T_STRING)) rb_raise(rb_eArgError, "Argument for 'event' must be a string.");
    WindowData* windowdata;
    TypedData_Get_Struct(self, WindowData, &rb_window_type, windowdata);
    long len = RARRAY_LEN(windowdata->observers);
    ID event_intern = rb_intern(RSTRING_PTR(rb_str_concat(rb_str_new("on_", 3), event)));
    for (long i = 0; i < len; i++) {
        VALUE observer = rb_ary_entry(windowdata->observers, i);
        if (rb_respond_to(observer, event_intern)) {
            rb_funcall(observer, event_intern, 1, data);
        }
    }
    return Qnil;
}
/// Callback for GLFW key events.
void rb_window_key_callback (GLFWwindow* window, int key, int scancode, int action, int mods) {
    VALUE rubywin = (VALUE) glfwGetWindowUserPointer(window);
    WindowData* windowdata;
    TypedData_Get_Struct(rubywin, WindowData, &rb_window_type, windowdata);
    char* event_name_cstring;
    if (action == GLFW_PRESS) event_name_cstring = "keydown";
    else if (action == GLFW_RELEASE) event_name_cstring = "keyup";
    else if (action == GLFW_REPEAT) event_name_cstring = "keyrepeat";
    else event_name_cstring = "keyunknown";
    VALUE event_name = rb_str_new_cstr(event_name_cstring);
    VALUE kevent = rb_funcall(CLASS_KEY_EVENT, rb_intern("new"), 4, INT2NUM(key), INT2NUM(scancode), INT2NUM(action), INT2NUM(mods));
    rb_window_emit(rubywin, event_name, kevent);
}

/// Callback for framebuffer resize.
void rb_window_framebuffer_resize_callback (GLFWwindow* window, int width, int height) {
    VALUE rubywin = (VALUE) glfwGetWindowUserPointer(window);
    WindowData* windowdata;
    TypedData_Get_Struct(rubywin, WindowData, &rb_window_type, windowdata);
    VALUE event_name = rb_str_new_cstr("resized");
    VALUE data = rb_ary_new2(2);
    rb_ary_store(data, 0, INT2NUM(width));
    rb_ary_store(data, 1, INT2NUM(height));
    rb_window_emit(rubywin, event_name, data);
}

/// Begin listening for framebuffer size changes.
VALUE rb_window_start_framebuffer_size_events (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    glfwSetFramebufferSizeCallback(window, rb_window_framebuffer_resize_callback);
    return Qnil;
}
/// Begin listening for key presses.
VALUE rb_window_start_key_events (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    glfwSetKeyCallback(window, rb_window_key_callback);
    return Qnil;
}
VALUE rb_window_get_key_state (VALUE self, VALUE rb_key) {
    Check_Type(rb_key, T_FIXNUM);
    GLFWwindow *window = window_from_self(self);
    return INT2NUM(glfwGetKey(window, NUM2INT(rb_key)));
}
VALUE rb_window_key_pressed (VALUE self, VALUE rb_key) {
    Check_Type(rb_key, T_FIXNUM);
    GLFWwindow *window = window_from_self(self);
    int key = NUM2INT(rb_key);
    int state = glfwGetKey(window, key);
    return state == GLFW_PRESS ? Qtrue : Qfalse;
}
VALUE rb_window_swap_buffers (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    glfwSwapBuffers(window);
    return Qnil;
}
VALUE rb_window_cursor_mode (VALUE self, VALUE rb_mode) {
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    int mode = NUM2INT(rb_mode);
    if (mode != GLFW_CURSOR_NORMAL && mode != GLFW_CURSOR_HIDDEN && mode != GLFW_CURSOR_DISABLED) {
        rb_raise(rb_eArgError, "Unrecognized integer given for mouse mode.");
        return Qnil;
    }
    GLFWwindow *window = window_from_self(self);
    glfwSetInputMode(window, GLFW_CURSOR, mode);
    return Qnil;
}

void rb_window_mouse_position_callback (GLFWwindow* window, double x, double y) {
    VALUE rubywin = (VALUE) glfwGetWindowUserPointer(window);
    VALUE ary = rb_ary_new2(2);
    rb_ary_store(ary, 0, DBL2NUM(x));
    rb_ary_store(ary, 1, DBL2NUM(y));
    rb_window_emit(rubywin, rb_str_new_cstr("mouse_move"), ary);
}

VALUE rb_window_start_mouse_position_events (VALUE self) {
    GLFWwindow *window = window_from_self(self);
    glfwSetCursorPosCallback(window, rb_window_mouse_position_callback);
    return Qnil;
}
VALUE rb_window_get_cursor_position (VALUE self) {
    double x, y;
    GLFWwindow *window = window_from_self(self);
    glfwGetCursorPos(window, &x, &y);
    VALUE ary = rb_ary_new2(2);
    rb_ary_store(ary, 0, DBL2NUM(x));
    rb_ary_store(ary, 1, DBL2NUM(y));
    return ary;
}

void init_window_under (VALUE module) {
    VALUE window_class = rb_define_class_under(module, "Window", rb_cObject);
    rb_define_alloc_func(window_class, rb_window_allocate);
    rb_define_method(window_class, "initialize", rb_window_initialize, 3);
    rb_define_method(window_class, "make_current", rb_window_make_current, 0);
    rb_define_method(window_class, "should_close", rb_window_should_close, 0);
    rb_define_method(window_class, "framebuffer_size", rb_window_get_framebuffer_size, 0);
    rb_define_method(window_class, "start_key_events", rb_window_start_key_events, 0);
    rb_define_method(window_class, "start_framebuffer_size_events", rb_window_start_framebuffer_size_events, 0);
    rb_define_method(window_class, "get_key_state", rb_window_get_key_state, 1);
    rb_define_method(window_class, "key_pressed?", rb_window_key_pressed, 1);
    rb_define_method(window_class, "destroy", rb_window_destroy, 0);
    rb_define_method(window_class, "add_observer", rb_window_add_observer, 1);
    rb_define_method(window_class, "del_observer", rb_window_del_observer, 1);
    rb_define_method(window_class, "emit", rb_window_emit, 2);
    rb_define_method(window_class, "swap_buffers", rb_window_swap_buffers, 0);
    rb_define_method(window_class, "cursor_mode", rb_window_cursor_mode, 1);
    rb_define_method(window_class, "start_mouse_position_events", rb_window_start_mouse_position_events, 0);
    rb_define_method(window_class, "get_mouse_position", rb_window_get_cursor_position, 0);
}
/*
    End of Window.
*/
/*
    Start of General GLFW.
*/
/// Destroy GLFW context.
VALUE rb_glfw_destroy () {
    glfwTerminate();
    GLFW_INITIALIZED = FALSE;
    return Qnil;
}
/// GLFW is initialized?
VALUE rb_glfw_is_initialized () {
    return GLFW_INITIALIZED == TRUE ? Qtrue : Qfalse;
}
/// Poll for keys on the current window.
VALUE rb_glfw_poll_events () {
    glfwPollEvents();
    return Qnil;
}
/// Sleep the thread until an event.
VALUE rb_glfw_wait_for_events () {
    glfwWaitEvents();
    return Qnil;
}
/// Get the scancode of a key.
VALUE rb_glfw_get_scancode (VALUE self, VALUE key) {
    return INT2NUM(glfwGetKeyScancode(NUM2INT(key)));
}
void error_callback(int code, const char* description) { printf("[ERR]: %i %s", code, description); }
/// Initialize GLFW context.
VALUE rb_glfw_init () {
    glfwSetErrorCallback(error_callback);
    int success = glfwInit();
    GLFW_INITIALIZED = success == GLFW_TRUE ? TRUE : FALSE;
    return success == GLFW_TRUE ? Qtrue : Qfalse;
}



/// Initialize the Ruby GLFW module.
VALUE init_general_glfw () {
    VALUE glfw_module = rb_define_module("GLFW");
    // Key event modes
    rb_define_const(glfw_module, "KEY_DOWN", INT2NUM(GLFW_PRESS));
    rb_define_const(glfw_module, "KEY_UP", INT2NUM(GLFW_RELEASE));
    rb_define_const(glfw_module, "KEY_REPEAT", INT2NUM(GLFW_REPEAT));
    // Cursor modes
    rb_define_const(glfw_module, "CURSOR_NORMAL", INT2NUM(GLFW_CURSOR_NORMAL));
    rb_define_const(glfw_module, "CURSOR_HIDE", INT2NUM(GLFW_CURSOR_HIDDEN));
    rb_define_const(glfw_module, "CURSOR_DISABLE", INT2NUM(GLFW_CURSOR_DISABLED));

    rb_define_module_function(glfw_module, "init", rb_glfw_init, 0);
    rb_define_module_function(glfw_module, "destroy", rb_glfw_destroy, 0);
    rb_define_module_function(glfw_module, "poll_events", rb_glfw_poll_events, 0);
    rb_define_module_function(glfw_module, "wait_for_events", rb_glfw_wait_for_events, 0);
    rb_define_module_function(glfw_module, "get_scancode", rb_glfw_get_scancode, 1);
    rb_define_module_function(glfw_module, "is_initialized?", rb_glfw_is_initialized, 0);
    return glfw_module;
}
/*
    End of General GLFW.
*/
/*
    Start of OpenGL.
*/
VALUE MOD_GL;
// Macro for a quick OpenGL error check.
#define CHECKGL() rb_gl_check_error()
// Macros to easily define OpenGL functions.
#define DEF_GLPROC(name, result, ...) result (*name)(__VA_ARGS__)
#define GLPROC(name, result, ...) name = (result (*)(__VA_ARGS__)) glfwGetProcAddress(#name)
#define GLPROC_CHECKED(name, result, ...) \
GLPROC(name, result, __VA_ARGS__); \
if (name == NULL) { \
    printf("Failed to load "#name".\n"); \
    CHECKGL();\
}
// OpenGL error check.
VALUE rb_gl_check_error (void) {
    GLenum errorCode;
    const char *error;
    while (true) {
        errorCode = glGetError();
        if (errorCode == GL_NO_ERROR) break;
        switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:               
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
        printf("error: %i\n", errorCode);
        rb_raise(rb_eRuntimeError, error);
    }
    return INT2NUM(errorCode);
}
// Define OpenGL functions loaded when needed.
// Shader:
DEF_GLPROC(glCreateShader, GLuint, GLenum);
DEF_GLPROC(glShaderSource, void, GLuint, GLsizei, const GLchar**, const GLint*);
DEF_GLPROC(glGetShaderiv, void, GLuint, GLenum, GLint*);
DEF_GLPROC(glGetShaderInfoLog, void, GLuint, GLsizei, GLsizei*, GLchar*);
DEF_GLPROC(glCompileShader, void, GLuint);
DEF_GLPROC(glDeleteShader, void, GLuint);
// Program:
DEF_GLPROC(glCreateProgram, GLuint, void);
DEF_GLPROC(glAttachShader, void, GLuint, GLuint);
DEF_GLPROC(glGetProgramiv, void, GLuint, GLenum, GLint*);
DEF_GLPROC(glGetProgramInfoLog, void, GLuint, GLsizei, GLsizei*, GLchar*);
DEF_GLPROC(glLinkProgram, void, GLuint);
DEF_GLPROC(glUseProgram, void, GLuint);
DEF_GLPROC(glDeleteProgram, void, GLuint);
DEF_GLPROC(glGetUniformLocation, GLint, GLuint, const GLchar*);

/*
    use gl::{self, types::{GLenum, GLint, GLuint}, COMPILE_STATUS, INFO_LOG_LENGTH, GetShaderInfoLog, AttachShader, GetProgramiv, LINK_STATUS};
    use gl::{STATIC_DRAW, DrawElements};
*/
VALUE rb_gl_set_clear_color (VALUE self, VALUE rb_r, VALUE rb_g, VALUE rb_b) {
    GLclampf r = (GLclampf) NUM2DBL(rb_r);
    GLclampf g = (GLclampf) NUM2DBL(rb_g);
    GLclampf b = (GLclampf) NUM2DBL(rb_b);
    glClearColor(r, g, b, 1.0);
    return Qnil;
}
VALUE rb_gl_clear (VALUE self) {
    glClear(GL_COLOR_BUFFER_BIT);
    return Qnil;
}
VALUE rb_gl_viewport (VALUE self, VALUE rb_width, VALUE rb_height) {
    int width, height;
    width = NUM2INT(rb_width);
    height = NUM2INT(rb_height);
    glViewport(0, 0, width, height);
    return Qnil;
}

// Shader functions.
// Create an OpenGL shader.
VALUE rb_gl_create_shader (VALUE self, VALUE shader_type) {
    GLenum kind = NUM2INT(shader_type);
    if (glCreateShader == NULL) { GLPROC_CHECKED(glCreateShader, GLuint, GLenum); }
    GLuint shader = glCreateShader(kind);
    CHECKGL();
    return INT2NUM(shader);
    return Qnil;
}
// Load a source string into an OpenGL shader.
VALUE rb_gl_shader_source (VALUE self, VALUE rb_shader, VALUE source_string) {
    GLuint shader = NUM2INT(rb_shader);
    if (glShaderSource == NULL) { GLPROC_CHECKED(glShaderSource, void, GLuint, GLsizei, const GLchar**, const GLint*); }
    GLsizei source_length = (GLsizei) NUM2SIZET(rb_str_length(source_string));
    const char* csource_str = StringValueCStr(source_string);
    glShaderSource(shader, 1, &csource_str, &source_length);
    CHECKGL();
    PRINT_DEBUG("Using shader source\n%s\n\n", csource_str);
    return Qnil;
}
// Print the error log from an OpenGL shader.
VALUE rb_gl_get_shader_log (VALUE self, VALUE rb_shader) {
    GLuint shader = NUM2INT(rb_shader);
    if (glGetShaderiv == NULL) { GLPROC_CHECKED(glGetShaderiv, void, GLuint, GLenum, GLint*); }
    if (glGetShaderInfoLog == NULL) { GLPROC_CHECKED(glGetShaderInfoLog, void, GLuint, GLsizei, GLsizei*, GLchar*); }
    GLsizei log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length == 0) return rb_str_new("", 0);
    GLchar log[log_length];
    GLsizei written;
    glGetShaderInfoLog(shader, log_length, &written, log);
    VALUE rb_log = rb_str_new(log, written);
    return rb_log;
}
// Compile an OpenGL shader.
VALUE rb_gl_compile_shader (VALUE self, VALUE rb_shader) {
    GLuint shader = NUM2INT(rb_shader);
    if (glCompileShader == NULL) { GLPROC_CHECKED(glCompileShader, void, GLuint); }
    if (glGetShaderiv == NULL) { GLPROC_CHECKED(glGetShaderiv, void, GLuint, GLenum, GLint*); }
    glCompileShader(shader);
    CHECKGL();
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    CHECKGL();
    return success == GL_TRUE ? Qtrue : Qfalse;
}
// Check if OpenGL shader is deleted.
VALUE rb_gl_shader_is_deleted (VALUE self, VALUE rb_shader) {
    GLuint shader = NUM2INT(rb_shader);
    if (glGetShaderiv == NULL) { GLPROC_CHECKED(glGetShaderiv, void, GLuint, GLenum, GLint*); }
    GLint is_deleted;
    glGetShaderiv(shader,  GL_DELETE_STATUS, &is_deleted);
    return is_deleted == GL_TRUE ? Qtrue : Qfalse;
}
// Delete an OpenGL shader.
VALUE rb_gl_delete_shader (VALUE self, VALUE rb_shader) {
    GLuint shader = NUM2INT(rb_shader);
    if (glDeleteShader == NULL) { GLPROC_CHECKED(glDeleteShader, void, GLuint); }
    glDeleteShader(shader);
    CHECKGL();
    return Qnil;
}

// Program functions.
// Create an OpenGL program.
VALUE rb_gl_create_program (VALUE self) {
    if (glCreateProgram == NULL) { GLPROC_CHECKED(glCreateProgram, GLuint, void); }
    GLuint program = glCreateProgram();
    CHECKGL();
    return INT2NUM(program);
}
// Attach a shader to an OpenGL program.
VALUE rb_gl_attach_shader (VALUE self, VALUE rb_program, VALUE rb_shader) {
    GLuint program = NUM2INT(rb_program);
    GLuint shader = NUM2INT(rb_shader);
    if (glAttachShader == NULL) { GLPROC_CHECKED(glAttachShader, void, GLuint, GLuint); }
    glAttachShader(program, shader);
    CHECKGL();
    return Qnil;
}
VALUE rb_gl_get_program_log (VALUE self, VALUE rb_program) {
    if (glGetProgramiv == NULL) { GLPROC_CHECKED(glGetProgramiv, void, GLuint, GLenum, GLint*); }
    if (glGetProgramInfoLog == NULL) { GLPROC_CHECKED(glGetProgramInfoLog, void, GLuint, GLsizei, GLsizei*, GLchar*); }
    GLsizei program_log_length;
    GLuint program = NUM2INT(rb_program);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &program_log_length);
    CHECKGL();
    if (program_log_length == 0) return rb_str_new("", 0);
    GLchar program_log[program_log_length];
    GLsizei written;
    glGetProgramInfoLog(program, program_log_length, &written, program_log);
    CHECKGL();
    VALUE rb_log = rb_str_new(program_log, written);
    return rb_log;
    return rb_str_new("", 0);
}
VALUE rb_gl_link_program (VALUE self, VALUE rb_program) {
    if (glLinkProgram == NULL) { GLPROC_CHECKED(glLinkProgram, void, GLuint); }
    if (glGetProgramiv == NULL) { GLPROC_CHECKED(glGetProgramiv, void, GLuint, GLenum, GLint*); }
    GLuint program = NUM2INT(rb_program);
    glLinkProgram(program);
    CHECKGL();
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    CHECKGL();
    return success == GL_TRUE ? Qtrue : Qfalse;
}
VALUE rb_gl_use_program (VALUE self, VALUE rb_program) {
    if (glUseProgram == NULL) { GLPROC_CHECKED(glUseProgram, void, GLuint); }
    GLuint program = NUM2INT(rb_program);
    PRINT_DEBUG("Using program %i.\n\n", program);
    glUseProgram(program);
    CHECKGL();
    return Qnil;
}
VALUE rb_gl_program_is_deleted (VALUE self, VALUE rb_program) {
    GLuint program = NUM2INT(rb_program);
    if (glGetProgramiv == NULL) { GLPROC_CHECKED(glGetProgramiv, void, GLuint, GLenum, GLint*); }
    GLint is_deleted;
    glGetProgramiv(program,  GL_DELETE_STATUS, &is_deleted);
    CHECKGL();
    return is_deleted == GL_TRUE ? Qtrue : Qfalse;
}
VALUE rb_gl_delete_program (VALUE self, VALUE rb_program) {
    if (glDeleteProgram == NULL) { GLPROC_CHECKED(glDeleteProgram, void, GLuint); }
    GLuint program = NUM2INT(rb_program);
    glDeleteProgram(program);
    CHECKGL();
    return Qnil;
}
VALUE rb_gl_get_uniform_location (VALUE self, VALUE rb_program, VALUE rb_name) {
    if (glGetUniformLocation == NULL) { GLPROC_CHECKED(glGetUniformLocation, GLint, GLuint, const GLchar*); }
    GLuint program = NUM2INT(rb_program);
    const char* cname = rb_string_value_cstr(&rb_name);
    GLint location = glGetUniformLocation(program, cname);
    CHECKGL();
    PRINT_DEBUG("Found uniform `%s` at location %i in program %i.\n\n", cname, location, program);
    return INT2NUM(location);
}

// Object functions.
// Allocate a Vertex Array Object.
DEF_GLPROC(glGenVertexArrays, void, GLsizei, GLuint*);
VALUE rb_gl_alloc_vertex_array_object () {
    if (glGenVertexArrays == NULL) { GLPROC_CHECKED(glGenVertexArrays, void, GLsizei, GLuint*); }
    GLuint vao;
    glGenVertexArrays(1, &vao);
    PRINT_DEBUG("Generated Vertex Array %i.\n\n", vao);
    return INT2NUM(vao);
}
// Bind a Vertex Array Object to the current context.
DEF_GLPROC(glBindVertexArray, void, GLuint);
VALUE rb_gl_bind_vertex_array_object (VALUE self, VALUE rb_vao) {
    if (glBindVertexArray == NULL) { GLPROC_CHECKED(glBindVertexArray, void, GLuint); }
    GLuint vao = NUM2INT(rb_vao);
    glBindVertexArray(vao);
    PRINT_DEBUG("Bound Vertex Array %i.\n\n", vao);
    return Qnil;
}
// Unbind current Vertex Array Object from the current context.
VALUE rb_gl_unbind_vertex_array_object () {
    if (glBindVertexArray == NULL) { GLPROC_CHECKED(glBindVertexArray, void, GLuint); }
    PRINT_DEBUG("Unbound Vertex Array.\n\n");
    glBindVertexArray(GL_ZERO);
}
// Allocate a Buffer.
DEF_GLPROC(glGenBuffers, void, GLsizei, GLuint*);
VALUE rb_gl_alloc_buffer_object (VALUE self) {
    //@TODO: Make a Ruby class for Buffer usage
    if (glGenBuffers == NULL) { GLPROC_CHECKED(glGenBuffers, void, GLsizei, GLuint*); }
    GLuint vbo;
    glGenBuffers(1, &vbo);
    PRINT_DEBUG("Allocated Buffer %i.\n\n", vbo);
    return INT2NUM(vbo);
}
// Bind a Buffer to the current context and target.
DEF_GLPROC(glBindBuffer, void, GLenum, GLuint);
VALUE rb_gl_bind_buffer_object (VALUE self, VALUE rb_target, VALUE rb_vbo) {
    //@TODO: Make all NUM2INT for GLuints to NUM2UINT
    //@TODO: Add enums for buffer targets
    if (glBindBuffer == NULL) { GLPROC_CHECKED(glBindBuffer, void, GLenum, GLuint); }
    GLuint vbo = NUM2UINT(rb_vbo);
    GLenum target = NUM2UINT(rb_target);
    glBindBuffer(target, vbo);
    PRINT_DEBUG("Bound Buffer %i to target %i.\n\n", vbo, target);
    return Qnil;
}
// Unbind current Buffer from the current context.
VALUE rb_gl_unbind_buffer_object (VALUE self, VALUE rb_target) {
    if (glBindBuffer == NULL) { GLPROC_CHECKED(glBindBuffer, void, GLenum, GLuint); }
    GLenum target = NUM2UINT(rb_target);
    glBindBuffer(target, GL_ZERO);
    PRINT_DEBUG("Unbound buffer from target %i.\n\n", target);
    return Qnil;
}
// Load data into the current Buffer.
DEF_GLPROC(glBufferData, void, GLenum, GLsizeiptr, const void*, GLenum);
VALUE rb_gl_set_buffer_data (VALUE self, VALUE rb_target, VALUE rb_data, VALUE rb_usage) {
    if (glBufferData == NULL) { GLPROC_CHECKED(glBufferData, void, GLenum, GLsizeiptr, const void*, GLenum); }
    long length;
    unsigned char* bytes;
    if (RB_TYPE_P(rb_data, T_STRING)) {
        length = RSTRING_LEN(rb_data);
        bytes = StringValuePtr(rb_data);
    } else {
        if (CLASS_BUFFER == Qnil) CLASS_BUFFER = rb_path2class("IO::Buffer");
        if (rb_obj_is_kind_of(rb_data, CLASS_BUFFER)) {
            size_t tlength;
            rb_io_buffer_get_bytes(rb_data, (void**) &bytes, (size_t*) &tlength);
            length = (long) tlength;
        } else {
            rb_raise(rb_eException, "%s%s", "Data must be of class String or IO::Buffer.", RB_TYPE_P(rb_data, T_ARRAY) ? " Did you pack your data?" : "");
            return Qnil;
        }
    }
    GLenum target = NUM2UINT(rb_target);
    GLenum usage = NUM2UINT(rb_usage);
    glBufferData(target, (GLsizeiptr) length, (const void*) bytes, usage);
    CHECKGL();
    #ifdef DEBUG
    printf("Buffering %ld bytes to target %i for usage %i.\n", length, target, usage);
    for (long i = 0; i < length; i++) printf("%02x ", bytes[i]);
    printf("\n\n");
    #endif
    return Qnil;
}
// Create a Vertex Attribute Array.
DEF_GLPROC(glEnableVertexAttribArray, void, GLuint);
VALUE rb_gl_enable_vertex_attribute_array (VALUE self, VALUE rb_index) {
    if (glEnableVertexAttribArray == NULL) { GLPROC_CHECKED(glEnableVertexAttribArray, void, GLuint); }
    GLuint index = NUM2UINT(rb_index);
    PRINT_DEBUG("Created Vertex Attribute Array at index %i.\n\n", index);
    glEnableVertexAttribArray(index);
    CHECKGL();
    return Qnil;
}
// Load data into Vertex Attribute Array.
DEF_GLPROC(glVertexAttribPointer, void, GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
VALUE rb_gl_set_vertex_attribute_pointer (VALUE self, VALUE rb_index, VALUE rb_component_count, VALUE rb_kind, VALUE rb_normalize, VALUE rb_start_ptr) {
    if (glVertexAttribPointer == NULL) { GLPROC_CHECKED(glVertexAttribPointer, void, GLuint, GLint, GLenum, GLboolean, GLsizei, const void*); }
    // Index of attribute to be edited
    // Component numbers per individual vertex (can be 1-4 components)
    // Type of component used (like GL_FLOAT for float values)
    // normalize or dont normalize
    // bytes between individual attributes
    // pointer to start of the attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    //@TODO: Make safer, infer stride size from type provided, less args?
    GLuint index = NUM2UINT(rb_index);
    GLint component_count = NUM2INT(rb_component_count);
    GLenum kind = NUM2UINT(rb_kind);
    GLboolean normalize = RB_TEST(rb_normalize) ? GL_TRUE : GL_FALSE;
    GLsizei stride;
    if (kind == GL_FLOAT) stride = component_count * sizeof(float);
    else if (kind == GL_INT || kind == GL_UNSIGNED_INT) stride = component_count * sizeof(float);
    else {
        stride = 0;
        rb_raise(rb_eRuntimeError, "Kind must be `GL::Float, GL::UnsignedInteger or GL::Integer`.");
        return Qnil;
    }
    void* start_ptr = (void*) NUM2SIZET(rb_start_ptr);

    #ifdef DEBUG
    char* kind_str;
    if (kind == GL_FLOAT) kind_str = "GL_FLOAT";
    else if (kind == GL_INT) kind_str = "GL_INT";
    else if (kind == GL_UNSIGNED_INT) kind_str = "GL_UNSIGNED_INT";
    PRINT_DEBUG("Vertex Attribute Pointer at index %i has %i %s components per vertex (making each vertex stride %i bytes of space), and %s normalized.\nIt starts at location %p.\n\n", index, component_count, kind_str, stride, normalize == GL_TRUE ? "is" : "is not", start_ptr);
    #endif
    
    glVertexAttribPointer(
        index,
        component_count,
        kind,
        normalize,
        stride,
        start_ptr
    );

    CHECKGL();
    return Qnil;
}
// Draw vertices.
VALUE rb_gl_draw_elements (VALUE self, VALUE rb_mode, VALUE rb_count, VALUE rb_type, VALUE rb_indices_ptr) {
    GLenum mode = NUM2UINT(rb_mode);
    GLsizei count = NUM2INT(rb_count);
    GLenum type = NUM2UINT(rb_type);
    const void* indices = (void*) NUM2SIZET(rb_indices_ptr);
    glDrawElements(mode, count, type, indices);
    CHECKGL();
    PRINT_DEBUG("Drawing %i vertices of type %i starting at %p in mode %i.\n\n", count, type, indices, mode);
    return Qnil;
}


/*
    glUniform1f
    glUniform2f
    glUniform3f
    glUniform4f
    same as f for i, ui, fv, iv, uiv
    then we have matrices 2fv, 3fv, 4fv, 2x3, 3x2, 2x4, 4x2, 3x4, 4x3
*/
#define NUM2TYPE(rbtype, rb_value) NUM2##rbtype(rb_value)
#define STRINGIFY(x) #x

#define DEF_RB_GL_UNIFORM(count, typeletter, ctype, rbtype_uppercase) \
DEF_GLPROC(glUniform##count##typeletter##v, void, GLint, GLsizei, const GL##ctype*); \
VALUE rb_gl_set_uniform##count##typeletter (VALUE self, VALUE rb_location, VALUE rb_args) { \
    Check_Type(rb_args, T_ARRAY); \
    int argc = (int) rb_array_len(rb_args); \
    if (argc != count) { \
        rb_raise(rb_eRuntimeError, "%i arguments passed, " #count "expected.", argc); \
        return Qnil; \
    }; \
    if (glUniform##count##typeletter##v == NULL) { GLPROC_CHECKED(glUniform##count##typeletter##v, void, GLint, GLsizei, const GL##ctype*); } \
    PRINT_DEBUG("Updating uniform" #count #typeletter " at location %i with data [ ", NUM2INT(rb_location)); \
    GL##ctype values[count]; \
    for (int i = 0; i < count; i++) { \
        VALUE rb_value = rb_ary_entry(rb_args, i); \
        values[i] = (GL##ctype) NUM2TYPE(rbtype_uppercase, rb_value); \
        PRINT_DEBUG("%f%s", values[i], i == count - 1 ? "" : ", "); \
    }; \
    PRINT_DEBUG(" ].\n\n"); \
    GLint location = NUM2INT(rb_location); \
    glUniform##count##typeletter##v(location, 1, values); \
    CHECKGL(); \
    return Qnil; \
}
#define DEF_RB_GL_UNIFORM_FULL(typeletter, ctype, rbtype_uppercase) \
    DEF_RB_GL_UNIFORM(1, typeletter, ctype, rbtype_uppercase); \
    DEF_RB_GL_UNIFORM(2, typeletter, ctype, rbtype_uppercase); \
    DEF_RB_GL_UNIFORM(3, typeletter, ctype, rbtype_uppercase); \
    DEF_RB_GL_UNIFORM(4, typeletter, ctype, rbtype_uppercase);
#define EXPOSE_RB_GL_UNIFORM(module, count, typeletter) rb_define_module_function(module, "set_uniform" #count #typeletter, rb_gl_set_uniform##count##typeletter, 2);
#define EXPOSE_RB_GL_UNIFORM_FULL(module, typeletter) \
    EXPOSE_RB_GL_UNIFORM(module, 1, typeletter); \
    EXPOSE_RB_GL_UNIFORM(module, 2, typeletter); \
    EXPOSE_RB_GL_UNIFORM(module, 3, typeletter); \
    EXPOSE_RB_GL_UNIFORM(module, 4, typeletter);

DEF_RB_GL_UNIFORM_FULL(i, int, INT);
DEF_RB_GL_UNIFORM_FULL(ui, uint, UINT);
DEF_RB_GL_UNIFORM_FULL(f, float, DBL);
// Initialize important OpenGL constants.
void init_opengl_constants (VALUE module) {
    // Shaders
    rb_define_const(module, "FRAGMENT_SHADER", INT2NUM(GL_FRAGMENT_SHADER));
    rb_define_const(module, "VERTEX_SHADER", INT2NUM(GL_VERTEX_SHADER));
    // Error checking
    rb_define_const(module, "COMPILE_STATUS", INT2NUM(GL_COMPILE_STATUS));
    rb_define_const(module, "LINK_STATUS", INT2NUM(GL_LINK_STATUS));
    rb_define_const(module, "INFO_LOG_LENGTH", INT2NUM(GL_INFO_LOG_LENGTH));
    // Buffer usage
    rb_define_const(module, "STATIC_DRAW", INT2NUM(GL_STATIC_DRAW));
    // Buffer types
    VALUE MOD_BUFFERTYPE = rb_define_module_under(module, "BufferType");
    rb_define_const(MOD_BUFFERTYPE, "Array", SIZET2NUM(GL_ARRAY_BUFFER));
    rb_define_const(MOD_BUFFERTYPE, "ElementArray", SIZET2NUM(GL_ELEMENT_ARRAY_BUFFER));
    // Type sizes
    VALUE MOD_SIZES = rb_define_module_under(module, "SizeOf");
    rb_define_const(MOD_SIZES, "Float", SIZET2NUM(sizeof(float)));
    rb_define_const(MOD_SIZES, "Integer", SIZET2NUM(sizeof(int)));
    rb_define_const(MOD_SIZES, "UnsignedInteger", SIZET2NUM(sizeof(int)));
    // Types
    rb_define_const(module, "Float", UINT2NUM(GL_FLOAT));
    rb_define_const(module, "Integer", UINT2NUM(GL_INT));
    rb_define_const(module, "UnsignedInteger", UINT2NUM(GL_UNSIGNED_INT));
    // Drwing modes
    rb_define_const(module, "Triangles", UINT2NUM(GL_TRIANGLES));
}
void check_gl_version (void) {
    GLFWwindow* window = glfwGetCurrentContext();
    const char* version = glGetString(GL_VERSION);
}
/*
    End of OpenGL.
*/

// Initialize everything important in this file.
void Init_window_utils (void) {
    // GLFW module.
    VALUE glfw_module = init_general_glfw();
    define_glfw_keys_under(glfw_module);
    define_glfw_key_mods_under(glfw_module);
    CLASS_KEY_EVENT = init_keyevent_under(glfw_module);
    init_window_under(glfw_module);
    // OpenGL module.
    MOD_GL = rb_define_module("GL");
    init_opengl_constants(MOD_GL);
    rb_define_module_function(MOD_GL, "set_clear_color", rb_gl_set_clear_color, 3);
    rb_define_module_function(MOD_GL, "clear", rb_gl_clear, 0);
    rb_define_module_function(MOD_GL, "viewport", rb_gl_viewport, 2);
    // shaders:
    rb_define_module_function(MOD_GL, "shader_source", rb_gl_shader_source, 2);
    rb_define_module_function(MOD_GL, "create_shader", rb_gl_create_shader, 1);
    rb_define_module_function(MOD_GL, "shader_log", rb_gl_get_shader_log, 1);
    rb_define_module_function(MOD_GL, "compile_shader", rb_gl_compile_shader, 1);
    rb_define_module_function(MOD_GL, "shader_deleted?", rb_gl_shader_is_deleted, 1);
    rb_define_module_function(MOD_GL, "delete_shader", rb_gl_delete_shader, 1);
    // programs:
    rb_define_module_function(MOD_GL, "create_program", rb_gl_create_program, 0);
    rb_define_module_function(MOD_GL, "attach_shader", rb_gl_attach_shader, 2);
    rb_define_module_function(MOD_GL, "program_log", rb_gl_get_program_log, 1);
    rb_define_module_function(MOD_GL, "link_program", rb_gl_link_program, 1);
    rb_define_module_function(MOD_GL, "use_program", rb_gl_use_program, 1);
    rb_define_module_function(MOD_GL, "program_deleted?", rb_gl_program_is_deleted, 1);
    rb_define_module_function(MOD_GL, "delete_program", rb_gl_delete_program, 1);
    rb_define_module_function(MOD_GL, "get_uniform_location", rb_gl_get_uniform_location, 2);
    // Buffers/Arrays:
    rb_define_module_function(MOD_GL, "alloc_vertex_array_object", rb_gl_alloc_vertex_array_object, 0);
    rb_define_module_function(MOD_GL, "bind_vertex_array_object", rb_gl_bind_vertex_array_object, 1);
    rb_define_module_function(MOD_GL, "unbind_vertex_array_object", rb_gl_unbind_vertex_array_object, 0);
    rb_define_module_function(MOD_GL, "alloc_buffer_object", rb_gl_alloc_buffer_object, 0);
    rb_define_module_function(MOD_GL, "bind_buffer_object", rb_gl_bind_buffer_object, 2);
    rb_define_module_function(MOD_GL, "unbind_buffer_object", rb_gl_unbind_buffer_object, 1);
    rb_define_module_function(MOD_GL, "set_buffer_data", rb_gl_set_buffer_data, 3);
    rb_define_module_function(MOD_GL, "enable_vertex_attribute_array", rb_gl_enable_vertex_attribute_array, 1);
    rb_define_module_function(MOD_GL, "set_vertex_attribute_pointer", rb_gl_set_vertex_attribute_pointer, 5);
    rb_define_module_function(MOD_GL, "draw_elements", rb_gl_draw_elements, 4);
    // Uniforms:
    EXPOSE_RB_GL_UNIFORM_FULL(MOD_GL, f);
    EXPOSE_RB_GL_UNIFORM_FULL(MOD_GL, i);
    EXPOSE_RB_GL_UNIFORM_FULL(MOD_GL, ui);
}
int main () { return 0; }