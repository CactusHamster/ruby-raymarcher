require_relative "ext/window_utils/window_utils"
require_relative "shader"
require_relative "program"
include GL
include GLFW

TARGET_FPS = 60
FRAME_TIME = 1.0 / TARGET_FPS

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
        @held_keys.add(kevent.key)
    end
    # KeyUp event.
    def on_keyup (kevent)
        @held_keys.delete(kevent.key)
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
    def process_held_keys (elapsed_time_ms)
        translate_x = case
            when @held_keys.include?(GLFW::KEYS[:"D"]) then 0.002
            when @held_keys.include?(GLFW::KEYS[:"A"]) then -0.002
            else 0
        end
        translate_y = case
            when @held_keys.include?(GLFW::KEYS[:"LEFT_SHIFT"]) then 0.002
            when @held_keys.include?(GLFW::KEYS[:"LEFT_CONTROL"]) then -0.002
            else 0
        end
        translate_z = case
            when @held_keys.include?(GLFW::KEYS[:"W"]) then 0.002
            when @held_keys.include?(GLFW::KEYS[:"S"]) then -0.002
            else 0
        end
        @position[0] += translate_x * elapsed_time_ms
        @position[1] += translate_y * elapsed_time_ms
        @position[2] += translate_z * elapsed_time_ms
        if translate_x + translate_y + translate_z != 0 then
            # puts "Position: #{@position} | Translation: #{[ translate_x, translate_y, translate_z ]} | Elapsed ms: #{time_elapsed_ms}"
            #@BUG When holding Shift and W, circle will disappear and y/z positions jump up to hundreds
            send_camera_uniform()
            render()
        end
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
        last_frame_time = Time.now
        render()
        while !@window.should_close & @running == true do
            # Time since last render was over.
            elapsed_time = Time.now - last_frame_time
            if elapsed_time < FRAME_TIME then
                sleep(FRAME_TIME - elapsed_time)
                elapsed_time = Time.now - last_frame_time
            end
            #@TODO: Calculate time between frames, move camera based on time spent between frames
            GLFW::poll_events()
            process_held_keys(elapsed_time * 1000)
            last_frame_time = Time.now
        end
        @window.destroy()
    end
    # Set initial parameters.
    def initialize
        @position = [ 0.0, 0.0, 0.0 ]
        @held_keys = Set.new()
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