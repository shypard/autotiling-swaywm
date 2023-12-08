project('autotiling', 'c', version: '1.0', license: 'MIT')
project_description  = 'Autotiling tool for swaywm using libswayipc.'

# ========================== Build Executable ===========================

deps = [
  dependency('libcjson'),
  dependency('swayipc'),
  dependency('threads'),
]

autotiling_target = executable(
  'autotiling',
  'source/autotiling.c',
  dependencies : deps,
  c_args : ['-std=c99', '-Wall', '-fanalyzer'],
  include_directories : include_directories('include'),
  )

