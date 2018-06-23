{
  'targets': [
    {
      'target_name': 'rtmidi',
      'sources': [
        'main.cpp',
        'deps/rtmidi-3.0.0/*.cpp',
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        'deps/rtmidi-3.0.0/*.h',
      ],
      'defines': [
      ],
    },
  ],
}
