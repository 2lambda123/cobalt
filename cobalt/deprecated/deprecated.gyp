{
  'targets': [
    {
      'target_name': 'platform_delegate',
      'type': '<(component)',
      'toolsets': ['target'],

      'include_dirs': [
        '<(DEPTH)',
      ],

      'sources': [
        'platform_delegate.cc',
        'platform_delegate.h',
        # actual_target_arch is equal to target_arch, except on Windows where
        # we use target_arch=xb1 to use xb1's configuration for Steel code, and
        # use actual_target_arch=win to inlcude the correct delegate source:
        # platform_delegate_win.cc.
        'platform_delegate_<(actual_target_arch).cc',
      ],
    },
  ],
}
