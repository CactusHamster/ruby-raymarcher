require_relative "ext/window_utils/window_utils"
require_relative "shader"
require_relative "program"
require_relative "vector"
require_relative "player"
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
    #     MOVE_SPEED_PER_SECOND = 6
    TARGET_FPS = 60
    FRAME_TIME = 1.0 / TARGET_FPS
    GROUND_HEIGHT = 0.0
    WINDOW_WIDTH = 800
    WINDOW_HEIGHT = 800

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

    # Update uniforms.
    def send_camera_uniform
        @program.set_float_uniform("camera", *@player.position)
    end

    def send_aspect_ratio_uniform(width, height)
        @program.set_float_uniform("aspect_ratio", width.to_f / height)
    end

    def send_rotation_uniform
        @program.set_float_uniform("rotation", *@player.rotation)
    end

    def send_ground_height_uniform
        @program.set_float_uniform("ground_height", GROUND_HEIGHT)
    end

    def send_time_uniform
        @program.set_float_uniform("time", (Time.now - @time_started).to_f)
    end


    # Set all uniforms.
    def initialize_uniforms ()
        send_time_uniform()
        send_camera_uniform()
        send_rotation_uniform()
        send_aspect_ratio_uniform(*@window.framebuffer_size())
    end

    # Draw shaders to window.
    def render ()
        GL::clear()
        GL::draw_elements(GL::Triangles, INDICES.flatten.length, GL::UnsignedInteger, 0)
        @window.swap_buffers()
    end

    # True if a key is currently held down.
    def key_held? (keyname)
        return @window.key_pressed?(GLFW::KEYS[keyname])
    end

    # Resize event.
    def on_resized (size)
        GL::viewport(*size)
        send_aspect_ratio_uniform(*size)
    end

    # Keyboard events.
    def on_keydown (kevent)
        @window.cursor_mode(GLFW::CURSOR_NORMAL) if kevent.key == GLFW::KEYS[:"LEFT_ALT"]
    end
    def on_keyup (kevent)
        @window.cursor_mode(GLFW::CURSOR_DISABLE) if kevent.key == GLFW::KEYS[:"LEFT_ALT"]
    end

    # Process keyboard input.
    def process_held_keys (elapsed_time)
        # ESC to quit.
        @running = false if key_held?(:"ESCAPE")
    end

    # Process mouse input.
    def process_mouse_movement (elapsed_time, dx, dy)
        return nil
    end

    # Begin main loop.
    def begin
        # Setup.
        raise "OpenGL not initialized!" unless GLFW::is_initialized?()
        @window = GLFW::Window.new(WINDOW_WIDTH, WINDOW_HEIGHT, "raymarcher")
        @window.make_current()
        @window.cursor_mode(GLFW::CURSOR_DISABLE)
        size = @window.framebuffer_size()
        GL::viewport size[0], size[1]
        GL::set_clear_color(0, 1, 0)
        init_shaders()
        @window.start_framebuffer_size_events()
        @window.start_key_events()
        @window.add_observer(self)
        @window.add_observer(@player)
        initialize_uniforms()
        
        # Main loop.
        last_frame_time = Time.now
        last_mouse_position = [ 0.0, 0.0 ]
        first_mouse = true
        while !@window.should_close & @running == true do
            # Time since last render was over.
            elapsed_time = Time.now - last_frame_time
            if elapsed_time < FRAME_TIME then
                sleep(FRAME_TIME - elapsed_time)
                elapsed_time = Time.now - last_frame_time
            end
            GLFW::poll_events()
            mouse_position = @window.get_mouse_position()
            if first_mouse == true then
                last_mouse_position = mouse_position 
                first_mouse = false
            end
            mouse_movement = [ mouse_position[0] - last_mouse_position[0], mouse_position[1] - last_mouse_position[1] ]
            
            # Process user input and update player.
            process_held_keys(elapsed_time)
            process_mouse_movement(elapsed_time, *mouse_movement)
            @player.update(elapsed_time, mouse_movement)

            # Render the scene.
            send_time_uniform()
            render()
            
            last_frame_time = Time.now
            last_mouse_position = mouse_position
        end
        
        # Cleanup.
        @window.destroy()
    end

    # Set initial parameters.
    def initialize
        @running = true
        @time_started = Time.now()
        @player = Player.new(self)
    end

    # Ensure everything is private.
    private :init_shaders, :render
end

GLFW::init()
ctx = Context.new()
ctx.begin()