#!/usr/bin/env ruby
require 'mkmf'
require 'rbconfig'

def glfw_name ()
    return 'glfw3' if /mswin|msys|mingw|cygwin|bccwin|wince|emc/.match?(RbConfig::CONFIG["host_os"])
    'glfw'
end

def check_glfw ()
    unless find_header File.join("GLFW", "#{glfw_name}.h")
        raise "Header GLFW/glfw3.h not found."
    end
    if RbConfig::CONFIG['target_os'] =~ /mingw|mswin/
        puts "Please ensure lib glfw is installed. Skipping check for Windows."
        raise "Library gdi32 not found." unless find_library "gdi32", "BeginPaint"
        raise "Library opengl32 not found." unless find_library "opengl32", "glBegin"
        # with_ldflags( "-l" + glfw_name + " -lopengl32 -lgdi32" )
        $LDFLAGS += " -l" + glfw_name + " -lopengl32 " + "-lgdi32" 
    else
        raise "Library " + glfw_name + "not found." unless have_library glfw_name, "glfwInit"
    end
end

check_glfw()
create_header
result = create_makefile "glfw/glfw"