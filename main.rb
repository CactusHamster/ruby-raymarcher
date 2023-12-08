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

class Context
    # Window creation.
    def init_window ()
        @window = GLFW::Window.new(600, 800, "raymarcher")
        @window.make_current()
        size = @window.framebuffer_size()
        GL::viewport size[0], size[1]
        GL::set_clear_color(0, 1, 0)
    end
    # Shader loading, compilation, and linking; and program creation
    def init_shaders ()
        # Vertex Array Object ( Pointers to vertices in the VBO )
        vao = GL::alloc_vertex_array_object()
        GL::bind_vertex_array_object(vao)

        # Vertex Buffer Object ( For vertex positions )
        vbo = GL::alloc_buffer_object()
        GL::bind_buffer_object(GL::BufferType::Array, vbo)
        GL::set_buffer_data(GL::BufferType::Array, VERTICES.flatten().pack("F*"), GL::STATIC_DRAW)

        # Element Buffer Object ( For vertex indices )
        ebo = GL::alloc_buffer_object()
        GL::bind_buffer_object(GL::BufferType::ElementArray, ebo)
        GL::set_buffer_data(GL::BufferType::ElementArray, INDICES.flatten().pack("L*"), GL::STATIC_DRAW)

        # Finish VAO stuff
        GL::enable_vertex_attribute_array(0)
        GL::set_vertex_attribute_pointer(0, 3, GL::Float, false, 0)

        GL::unbind_buffer_object(GL::BufferType::Array)
        # GL::unbind_buffer_object(GL::BufferType::ElementArray)

        # Make and use shader program
        @program = Program.from_sources(File.read("vert.glsl"), File.read("frag.glsl"))
        @program.use()
    end
    # Set uniform vec3 camera - holds camera position data.
    def send_camera_uniform ()
        @program.set_float_uniform("camera", *@position)
    end
    # Set float aspect_ratio - holds the screen's aspect ratio.
    def send_size_uniform (width, height)
        @program.set_float_uniform("aspect_ratio", width.to_f / height)
    end
    # KeyDown event.
    def on_keydown (kevent)
        translate_x = case
            when kevent.key == GLFW::KEYS[":D"] then 1
            when kevent.key == GLFW::KEYS[:"A"] then -1
            else 0
        end
        translate_y = case
            when kevent.key == GLFW::KEYS[:"W"] then 1
            when kevent.key == GLFW::KEYS[:"S"] then -1
            else 0
        end
        @position[0] += translate_x
        @position[1] += translate_y
        if translate_x + translate_y != 0 then
            send_camera_uniform()
            render()
        end
    end
    # KeyUp event.
    def on_keyup (kevent)
        @running = false if kevent.key == GLFW::KEYS[:"ESCAPE"]
    end
    # Resize event.
    def on_resized (size)
        GL::viewport(*size)
        send_size_uniform(*size)
        render()
    end
    # Draw shaders to window.
    def render ()
        GL::clear()
        GL::draw_elements(GL::Triangles, INDICES.flatten.length, GL::UnsignedInteger, 0)
        @window.swap_buffers()
    end
    # Begin main loop.
    def begin
        raise "OpenGL not initialized!" unless GLFW::is_initialized?()
        init_window
        init_shaders
        @window.start_framebuffer_size_events()
        @window.start_key_events()
        @window.add_observer(self)
        send_camera_uniform()
        send_size_uniform(*@window.framebuffer_size())
        render()
        while !@window.should_close & @running == true do
           GLFW::wait_for_events()
        end
        @window.destroy()
    end
    # Set initial parameters.
    def initialize
        @position = [ 0, 0, 0 ]
        @running = true
    end
    # Ensure everything is private.
    private :init_window, :init_shaders, :render
end

GLFW::init()
ctx = Context.new()
ctx.begin()

# Create a camera.
#camera = Camera.new()

# Listen for key and resize events.
#event_handler = EventHandler.new(win, camera)
#win.add_observer(event_handler)

# Main loop.
#game_running = true
#win.start_framebuffer_size_events()
#win.start_key_events()
#render win
#while !win.should_close() & game_running do
#    GLFW::wait_for_events()
#end
#win.destroy()