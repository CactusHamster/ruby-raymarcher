require_relative "ext/window_utils/window_utils"
require_relative "shader"
include GL

class Program
    attr_reader :"program"
    def initialize
        @program = GL.create_program()
    end
    def attach (shader)
        if shader.is_a?(Shader)
            GL::attach_shader(@program, shader.shader)
        else
            GL::attach_shader(@program, shader)
        end
    end
    def log
        return GL::program_log(@program)
    end
    def link ()
        success = GL::link_program(@program)
        raise "Failed to link program.\n\n#{self.log}" unless success == true
        return nil
    end
    def use ()
        GL::use_program(@program)
        return nil
    end
    def delete ()
        if not GL.program_deleted?(@program) then
            GL::delete_program(@program)
        end
        return nil
    end
    def uniform_location (name)
        location = GL::get_uniform_location(name)
        if location == 0 then
            raise RuntimeError.new("Failed to find uniform in program.")
        end
        return location
    end
    def self.from_sources (vertex_shader_string, fragment_shader_string)
        vert = Shader.new(:"vertex")
        frag = Shader.new(:"fragment")
        vert.set_source(vertex_shader_string)
        frag.set_source(fragment_shader_string)
        vert.compile()
        frag.compile()
        program = self.new()
        program.attach(vert)
        program.attach(frag)
        program.link()
        vert.delete()
        frag.delete()
        return program
    end
end