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
        GL::delete_program(@program) unless GL.program_deleted?(@program)
        return nil
    end
    def uniform_location (name)
        location = GL::get_uniform_location(@program, name)
        raise RuntimeError.new("Failed to find uniform `#{name}`.") if location < 0
        return location
    end
    def resolve_uniform_locator (name_or_location)
        location = case
            when name_or_location.is_a?(Integer) then name_or_location
            when name_or_location.is_a?(String) then uniform_location(name_or_location)
            else raise RuntimeError.new("Invalid uniform location/name.")
        end
        return location
    end
    def set_float_uniform (name, *values)
        location = resolve_uniform_locator(name)
        raise "Invalid uniform length #{values.length}." if values.length > 4 or values.length < 1
        GL::set_uniform1f(location, values) if values.length == 1
        GL::set_uniform2f(location, values) if values.length == 2
        GL::set_uniform3f(location, values) if values.length == 3
        GL::set_uniform4f(location, values) if values.length == 4
    end
    def set_int_uniform (name, *values)
        location = resolve_uniform_locator(name)
        raise "Invalid uniform length #{values.length}." if values.length > 4 or values.length < 1
        GL::set_uniform1i(location, values) if values.length == 1
        GL::set_uniform2i(location, values) if values.length == 2
        GL::set_uniform3i(location, values) if values.length == 3
        GL::set_uniform4i(location, values) if values.length == 4
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