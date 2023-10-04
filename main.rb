require_relative "ext/opengl/opengl"
include GL
include GLFW
GLFW::init()
window = GLFW::Window.new(600, 800, "window")
window.make_current()

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

camera = Camera.new()
while !window.should_close() do
    # Poll key events.
    GLFW::poll_events()

    # Calculate camera translation.
    # [ x, y ]
    
    if window.get_key_state(GLFW::KEYS[:"d"]) then
        camera.translatex(1)
    elsif window.get_key_state(GLFW::KEYS[:"a"]) then
        camera.translatex(-1)
    end
    if window.get_key_state(GLFW::KEYS[:"w"]) then
        camera.translatey(1)
    elsif window.get_key_state(GLFW::KEYS[:"s"]) then
        camera.translatey(-1)
    end
    sleep(0.2)
end