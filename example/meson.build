topname = 'top'
nxdc_files = 'constr/top.nxdc'

verilator = find_program('verilator', required: true)
python = find_program('python3', required: true)

# 生成 auto_bind.cpp
auto_bind = custom_target('auto_bind',
  input: nxdc_files,
  output: 'auto_bind.cpp',
  command: [
    python,
    meson.project_source_root() / 'scripts' / 'auto_pin_bind.py',
    '@INPUT@',
    '@OUTPUT@',
  ],
)

# Verilog 和 C++ 源文件
vsrc_files = [
  'vsrc/top.v',
  'vsrc/led.v',
  'vsrc/seg.v',
  'vsrc/vga_ctrl.v',
  'vsrc/uart.v',
  'vsrc/ps2_keyboard.v',
]
csrc_files = [
  'csrc/main.cpp',
]

# SDL2 链接参数 (与 nvboard.mk 一致)
sdl2_config = find_program('sdl2-config', required: true)
sdl2_ldflags = run_command(sdl2_config, '--libs', check: true).stdout().strip()
ldflags = sdl2_ldflags + ' -lSDL2_image -lSDL2_ttf'

# NVBoard 库路径 (Verilator 的 make 在 obj_dir 中运行，需用 -L 指定库目录)
fs = import('fs')
nvboard_dir = fs.parent(nvboard_lib.full_path())
ldflags += ' -L' + nvboard_dir + ' -lnvboard'

# CFLAGS
usr_inc = meson.project_source_root() / 'usr' / 'include'
cflags = '-I' + usr_inc + ' -DTOP_NAME="V' + topname + '"'

# Verilator 构建可执行文件 (使用绝对路径作为 -o，因 Verilator 的 make 在 obj_dir 中运行)
top_output_path = meson.current_build_dir() / topname

top_exe = custom_target('top',
  input: [vsrc_files, csrc_files, auto_bind],
  output: topname,
  command: [
    verilator,
    '--build', '-cc',
    '-O3', '--x-assign', 'fast', '--x-initial', 'fast', '--noassert',
    '--top-module', topname,
    '@INPUT@',
    '-CFLAGS', cflags,
    '-LDFLAGS', ldflags,
    '--Mdir', '@OUTDIR@' / 'obj_dir',
    '--exe', '-o', top_output_path,
  ],
  depends: [nvboard_lib],
  build_by_default: true,
)

# 提供 run 目标: ninja -C build run 会执行 example 的 top
run_target('run',
  command: [top_exe],
  depends: top_exe,
)
