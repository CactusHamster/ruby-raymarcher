# Ruby Raymarcher
Just a silly little raymarcher I made to practice/learn Ruby!✨
## Dependencies
- GLFW3
- OpenGL
## Build
Because this project uses C for GLFW and OpenGL, some code needs to be compiled.
Running `extconf.rb` will generate a Makefile.
```bash
# Generate a Makefile
cd ./ext/window_utils
ruby extconf.rb

# Compile
make
```
## Run!
```
ruby main.rb
```
## Controls
---
| Control | Action |
|------|-----------|
| WSAD | Move |
| Mouse | Rotate |
| Space | Jump |
| Escape | Quit |
| Left Alt | Show Mouse |
