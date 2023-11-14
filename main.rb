require_relative "ext/opengl/opengl"
require_relative "ext/glfw/glfw"
include GL
include GLFW

game_running = true
win = nil

class Camera
    def initialize
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

class EventHandler
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
        if translate_x != 0 then
            camera.translatex(translate_x)
        end
        if translate_y != 0 then
            camera.translatey(translate_y)
        end
    end
    def keyup (kevent)
        if kevent.key == GLFW::KEYS[:"ESCAPE"] then
            game_running = false
        end
    end
    def resized (size)
        p "Resized window to #{size[0]}x#{size[1]}"
    end
end


# Initialize GLFW.
GLFW::init()
# Create main window.
win = GLFW::Window.new(600, 800, "game")
# Initialize the window.
win.start_framebuffer_size_events()
win.start_key_events()
win.make_current()
# Listen for key and resize events.
event_handler = EventHandler.new()
win.add_observer(event_handler)
# Create a camera.
camera = Camera.new()
# Main loop.
while !win.should_close() & game_running do
    GLFW::wait_for_events()
end
win.destroy()