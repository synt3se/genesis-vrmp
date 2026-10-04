project('genesis', 'cpp',
  version : '0.1.0',
  meson_version : '>=1.3.0',
  default_options : ['cpp_std=c++20', 'warning_level=3', 'buildtype=debugoptimized'])

cpp = meson.get_compiler('cpp')

# Флаги детерминизма нужны всем целям, а не только ядру: inline-код из core/include
# компилируется с флагами той цели, которая его включает
if cpp.get_argument_syntax() == 'msvc'
  add_project_arguments('/fp:precise', language : 'cpp')
else
  add_project_arguments('-ffp-contract=off', '-fno-fast-math', language : 'cpp')
endif

core_inc = include_directories('core/include')
# is_system: предупреждения из чужих заголовков не смешиваются с нашими
third_inc = include_directories('third_party', is_system : true)
app_inc = include_directories('app')

# Без SDL3 собираются только ядро и тесты
sdl3_dep = dependency('sdl3', version : '>=3.4.0', required : get_option('gui'))

if sdl3_dep.found()
  imgui_inc = include_directories('third_party/imgui', 'third_party/imgui/backends', is_system : true)
  imgui_lib = static_library('imgui',
    files('third_party/imgui/imgui.cpp', 'third_party/imgui/imgui_demo.cpp',
          'third_party/imgui/imgui_draw.cpp', 'third_party/imgui/imgui_tables.cpp',
          'third_party/imgui/imgui_widgets.cpp',
          'third_party/imgui/backends/imgui_impl_sdl3.cpp',
          'third_party/imgui/backends/imgui_impl_sdlrenderer3.cpp'),
    include_directories : imgui_inc,
    dependencies : sdl3_dep,
    override_options : ['warning_level=0'])
  imgui_dep = declare_dependency(link_with : imgui_lib, include_directories : imgui_inc)

  executable('genesis',
    files('app/main.cpp', 'app/app.cpp', 'app/test_world.cpp',
          'app/render/camera.cpp', 'app/render/map_view.cpp', 'app/render/terrain_color.cpp',
          'app/ui/imgui_layer.cpp', 'app/ui/sidebar.cpp'),
    include_directories : [core_inc, third_inc, app_inc],
    dependencies : [sdl3_dep, imgui_dep])

  # Камере нужны только типы SDL; без линковки тестам не нужна SDL3.dll
  sdl3_headers = sdl3_dep.partial_dependency(compile_args : true, includes : true)
  app_tests = executable('app_tests',
    files('tests/main.cpp', 'tests/app/camera_test.cpp', 'app/render/camera.cpp'),
    include_directories : [core_inc, third_inc, app_inc],
    dependencies : sdl3_headers)
  test('app', app_tests)
endif
