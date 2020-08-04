project('memdump', ['cpp'],
  version: '0.1.0',
  license: ['MIT'],
  meson_version: '>=0.41.0',
  default_options: [
    'werror=true',
    'warning_level=1',
    'cpp_std=gnu++11',
  ]
)

cpp = meson.get_compiler('cpp')

warning_flags = [
  '-Wredundant-decls',
  '-Wwrite-strings',
  '-Wformat',
  '-Wformat-nonliteral',
  '-Wformat-security',
  '-Winit-self',
  '-Waddress',
  '-Wno-multichar',
  '-Wvla',
  '-Wpointer-arith',
  '-Wno-sign-compare',
]

foreach extra_arg : warning_flags
  if cpp.has_argument (extra_arg)
    add_project_arguments([extra_arg], language : 'cpp')
  endif
endforeach

subdir('src')
