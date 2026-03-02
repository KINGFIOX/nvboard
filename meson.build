project('nvboard', 'cpp',
  version: '0.1.0',
  meson_version: '>=0.56.0',
  default_options: ['cpp_std=c++17', 'optimization=2'],
)

sdl2_dep = dependency('sdl2')
sdl2_image_dep = dependency('SDL2_image')
sdl2_ttf_dep = dependency('SDL2_ttf')

nvboard_sources = [
  'src/button.cpp',
  'src/component.cpp',
  'src/event.cpp',
  'src/font.cpp',
  'src/keyboard.cpp',
  'src/led.cpp',
  'src/nvboard.cpp',
  'src/render.cpp',
  'src/segs7.cpp',
  'src/switch.cpp',
  'src/term.cpp',
  'src/timer.cpp',
  'src/uart.cpp',
  'src/vga.cpp',
]

nvboard_inc = include_directories('include')

nvboard_lib = static_library('nvboard',
  nvboard_sources,
  include_directories: nvboard_inc,
  dependencies: [sdl2_dep, sdl2_image_dep, sdl2_ttf_dep],
)

# ================================================
# declare dependency for subproject which wants to link nvboard library
# ================================================
nvboard_dep = declare_dependency(
  link_with: nvboard_lib,
  include_directories: nvboard_inc,
  dependencies: [sdl2_dep, sdl2_image_dep, sdl2_ttf_dep],
)
meson.override_dependency('nvboard', nvboard_dep)

# for subproject to get nvboard source code root directory (for scripts, usr/include, etc.)
nvboard_src_root = meson.current_source_dir()
