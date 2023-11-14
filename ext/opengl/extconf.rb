#!/usr/bin/env ruby
require 'mkmf'

def check_GL ()
    raise "GL/gl.h not found." unless find_header "GL/gl.h"
end

check_GL()
create_header
result = create_makefile "opengl/opengl"