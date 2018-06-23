{
  'targets': [
    {
      'target_name': 'rtmidi',
      'include_dirs': ["<!(node -e \"require('nan')\")"],
      'sources': [
        'main.cpp',
        'deps/rtmidi-3.0.0/*.cpp',
      ],
      'include_dirs': [
        'deps/rtmidi-3.0.0/*.h',
      ],
      'defines': [
      ],
    },
  ],
}
