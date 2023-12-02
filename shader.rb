require_relative "ext/window_utils/window_utils"
include GL

class Shader
    attr_reader :"shader"
    def initialize (kind)
        # kind can be :"fragment" or :"vertex"
        if kind == :"fragment"
            @shader = GL.create_shader GL::FRAGMENT_SHADER
        elsif kind == :"vertex"
            @shader = GL.create_shader GL::VERTEX_SHADER
        else
            raise ArgumentError.new("Shader kind must be :\"vertex\" or :\"fragment\".")
            return nil
        end
        return nil
    end
    def set_source (source)
        GL.shader_source(@shader, source)
        return nil
    end
    def log ()
        log = GL.shader_log(@shader)
        return log
    end
    def compile ()
        success = GL.compile_shader(@shader)
        if success == false then
            log = self.log()
            raise RuntimeError.new("Failed to compile shader.\n#{log}");
        end
    end
    def delete ()
        if not GL.shader_deleted?(@shader) then
            GL.delete_shader(@shader)
        end
    end
    def self.from_string (kind, source)
        shader = self.new(kind)
        shader.set_source(source)
        return shader
    end
end