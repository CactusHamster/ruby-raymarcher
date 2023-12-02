require_relative "ext/window_utils/window_utils"
require_relative "shader"
require_relative "program"
include GL
include GLFW

# Vertex indices.
VERTICES = [
    [1.0, 1.0, 0.0],
    [1.0, -1.0, 0.0],
    [-1.0, -1.0, 0.0],
    [-1.0, 1.0, 0.0]
];
# Triangles from vertex indices.
INDICES = [
    [0, 1, 3],
    [1, 2, 3]
];

# Render with OpenGL.
def render (win)
    GL::clear()
    GL::draw_elements(GL::Triangles, INDICES.flatten.length, GL::UnsignedInteger, 0)
    win.swap_buffers()
end

# Storage for camera position.
class Camera
    def initialize ()
        @position = [ 0, 0 ]
        @zoom = 1
    end
    def translate(x, y)
        translatex(x)
        translatey(y)
    end
    def translatex(x)
        @position[0] = x
    end
    def translatey(y)
        @position[1] = y
    end
end

# Handler for GLFW events.
class EventHandler
    def initialize (win)
        @window = win
    end
    def keydown (kevent)
        translate_x = 0
        translate_y = 0
        if kevent.key == GLFW::KEYS[:"D"] then
            translate_x += 1
        elsif kevent.key == GLFW::KEYS[:"A"] then
            translate_x -= 1
        end
        if kevent.key == GLFW::KEYS[:"W"] then
            translate_x += 1
        elsif kevent.key == GLFW::KEYS[:"S"] then
            translate_x -= 1
        end
        # if translate_x != 0 then
        #     camera.translatex(translate_x)
        # end
        # if translate_y != 0 then
        #     camera.translatey(translate_y)
        # end
        render @window
    end
    def keyup (kevent)
        if kevent.key == GLFW::KEYS[:"ESCAPE"] then
            puts "Escape"
        end
    end
    def resized (size)
        GL::viewport(size[0], size[1])
        render @window
    end
end


# Initialize GLFW.
GLFW::init()

# Initialize main window.
win = GLFW::Window.new(600, 800, "game")
win.make_current()
size = win.framebuffer_size()
GL::viewport size[0], size[1]
GL::set_clear_color(0, 1, 0)

# Vertex Array Object ( Pointers to vertices in the VBO )
vao = GL::alloc_vertex_array_object()
GL::bind_vertex_array_object(vao)

# Vertex Buffer Object ( For vertex positions )
vbo = GL::alloc_buffer_object()
GL::bind_buffer_object(GL::BufferType::Array, vbo)
GL::set_buffer_data(GL::BufferType::Array, VERTICES.flatten().pack("F*"), GL::STATIC_DRAW) # Pack as single-precision floats.

# Element Buffer Object ( For vertex indices )
ebo = GL::alloc_buffer_object()
GL::bind_buffer_object(GL::BufferType::ElementArray, ebo)
GL::set_buffer_data(GL::BufferType::ElementArray, INDICES.flatten().pack("L*"), GL::STATIC_DRAW) # Pack as unsigned 8-bit chars.


# Finish VAO stuff
GL::enable_vertex_attribute_array(0)
GL::set_vertex_attribute_pointer(0, 3, GL::Float, false, 0)

GL::unbind_buffer_object(GL::BufferType::Array)
# GL::unbind_buffer_object(GL::BufferType::ElementArray)

program = Program.from_sources(File.read("vert.glsl"), File.read("frag.glsl"))
program.use

# Listen for key and resize events.
event_handler = EventHandler.new(win)
win.add_observer(event_handler)
# Create a camera.
camera = Camera.new()
# Main loop.
game_running = true
win.start_framebuffer_size_events()
win.start_key_events()
render win
while !win.should_close() & game_running do
    GLFW::wait_for_events()
end
win.destroy()