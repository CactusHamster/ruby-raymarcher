#include <ruby.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glfw.h"
VALUE CLASS_KEY_EVENT;

#define KEY_COUNT 122
#define KEYNAME_LENGTH 13
const char key_names[KEY_COUNT][KEYNAME_LENGTH] = {"UNKNOWN","SPACE","APOSTROPHE","COMMA","MINUS","PERIOD","SLASH","0","1","2","3","4","5","6","7","8","9","SEMICOLON","EQUAL","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","LEFT_BRACKET","BACKSLASH","RIGHT_BRACKET","GRAVE_ACCENT","WORLD_1","WORLD_2","ESCAPE","ENTER","TAB","BACKSPACE","INSERT","DELETE","RIGHT","LEFT","DOWN","UP","PAGE_UP","PAGE_DOWN","HOME","END","CAPS_LOCK","SCROLL_LOCK","NUM_LOCK","PRINT_SCREEN","PAUSE","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12","F13","F14","F15","F16","F17","F18","F19","F20","F21","F22","F23","F24","F25","KP_0","KP_1","KP_2","KP_3","KP_4","KP_5","KP_6","KP_7","KP_8","KP_9","KP_DECIMAL","KP_DIVIDE","KP_MULTIPLY","KP_SUBTRACT","KP_ADD","KP_ENTER","KP_EQUAL","LEFT_SHIFT","LEFT_CONTROL","LEFT_ALT","LEFT_SUPER","RIGHT_SHIFT","RIGHT_CONTROL","RIGHT_ALT","RIGHT_SUPER","MENU","LAST"};
const int key_values[KEY_COUNT] = {-1,32,39,44,45,46,47,48,49,50,51,52,53,54,55,56,57,59,61,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,96,161,162,256,257,258,259,260,261,262,263,264,265,266,267,268,269,280,281,282,283,284,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,306,307,308,309,310,311,312,313,314,320,321,322,323,324,325,326,327,328,329,330,331,332,333,334,335,336,340,341,342,343,344,345,346,347,348,348};

#define MOD_COUNT 6
#define MODNAME_LENGTH 9
const char mod_names[MOD_COUNT][MODNAME_LENGTH] = {"SHIFT","CONTROL","ALT","SUPER","CAPS_LOCK","NUM_LOCK"};
const int mod_values[MOD_COUNT] = {0x0001,0x0002,0x0004,0x0008,0x0010,0x0020};

VALUE define_glfw_keys_under (VALUE module) {
    VALUE hash = rb_hash_new_capa(KEY_COUNT);
    for (int i = 0; i < KEY_COUNT; i++) {
        VALUE symbol = ID2SYM(rb_intern(key_names[i]));
        VALUE value = INT2NUM(key_values[i]);
        rb_hash_aset(hash, symbol, value);
    }
    rb_define_const(module, "KEYS", hash);
}
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
    printf("Checking size of window.\n");
    return sizeof(WindowData);
}
/// Retrieve the WindowData struct from a Ruby Window object.
GLFWwindow* window_from_self (VALUE rb_window) {
    WindowData* window = (WindowData*) DATA_PTR(rb_window);
    return window->window;
}
/// Deallocates Ruby Window when no longer in use.
void rb_window_free (void* ptr) { 
    printf("Freeing   WindowData at 0x%p.\n", ptr);
    WindowData* win_data = (WindowData*) ptr;
    if (win_data->destroyed == FALSE) glfwDestroyWindow(win_data->window);
    free(win_data->window);
    xfree(ptr);
}
/// Ensure the observers aren't killed.
void rb_window_mark (void* ptr) {
    printf("WindowData at 0x%p was marked.\n", ptr);
    WindowData* win_data = (WindowData*) ptr;
    rb_gc_mark_movable(win_data->observers);
}
void rb_window_compact (void *ptr) {
    printf("WindowData at 0x%p was compacted.\n", ptr);
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
    printf("Allocated WindowData at 0x%p.\n", windowdata);
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
    window_data->window = glfwCreateWindow(width_int, height_int, title_str, NULL, NULL);
    if (window_data->window == NULL) {
        printf("Window is NULL.\n");
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
    ID event_intern = rb_intern(RSTRING_PTR(event));
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
VALUE rb_window_get_key_state (VALUE self, VALUE key) {
    Check_Type(key, T_FIXNUM);
    GLFWwindow *window = window_from_self(self);
    return INT2NUM(glfwGetKey(window, NUM2INT(key)));
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
    rb_define_method(window_class, "destroy", rb_window_destroy, 0);
    rb_define_method(window_class, "add_observer", rb_window_add_observer, 1);
    rb_define_method(window_class, "del_observer", rb_window_del_observer, 1);
    rb_define_method(window_class, "emit", rb_window_emit, 2);
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
    return Qnil;
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
VALUE rb_glfw_get_scancode (VALUE key) {
    return INT2NUM(glfwGetKeyScancode(NUM2INT(key)));
}
void init_glfw_key_constants (VALUE module) {
    VALUE hash = rb_hash_new();
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
        const int scancode = glfwGetKeyScancode(key);
        if (scancode == -1) continue;
        const char* keyname = glfwGetKeyName(key, scancode);
        if (keyname == NULL) {
            printf("%i | %i\n", key, scancode);
            continue;
        };
        VALUE symbol = ID2SYM(rb_intern(keyname));
        rb_hash_aset(hash, symbol, INT2FIX(key));
    }
    rb_define_const(module, "KEYS", hash);
}
void error_callback(int code, const char* description) { printf("[ERR]: %i %s", code, description); }
/// Initialize GLFW context.
VALUE rb_glfw_init (void) {
    glfwSetErrorCallback(error_callback);
    int success = glfwInit();
    return success == GLFW_TRUE ? Qtrue : Qfalse;
}
/// Initialize the Ruby GLFW module.
VALUE init_general_glfw () {
    VALUE glfw_module = rb_define_module("GLFW");
    rb_define_const(glfw_module, "KEYDOWN", GLFW_PRESS);
    rb_define_const(glfw_module, "KEYUP", GLFW_RELEASE);
    rb_define_const(glfw_module, "KEYREPEAT", GLFW_REPEAT);
    rb_define_module_function(glfw_module, "init", rb_glfw_init, 0);
    rb_define_module_function(glfw_module, "destroy", rb_glfw_destroy, 0);
    rb_define_module_function(glfw_module, "poll_events", rb_glfw_poll_events, 0);
    rb_define_module_function(glfw_module, "wait_for_events", rb_glfw_wait_for_events, 0);
    rb_define_module_function(glfw_module, "get_scancode", rb_glfw_get_scancode, 1);
    return glfw_module;
}
/*
    End of General GLFW.
*/

// Initialize everything important in this file.
void Init_glfw (void) {
    rb_require("set");
    // GLFW related code.
    VALUE glfw_module = init_general_glfw();
    define_glfw_keys_under(glfw_module);
    define_glfw_key_mods_under(glfw_module);
    CLASS_KEY_EVENT = init_keyevent_under(glfw_module);
    init_window_under(glfw_module);
}
int main () { return 0; }