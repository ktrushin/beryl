configure_file(
  input: 'version.hpp.in',
  output: 'version.hpp',
  configuration: {
    'version': run_command(get_version, '--project').stdout().strip()
  }
)
