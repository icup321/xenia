test_subfe:
  #_ REGISTER_IN r10 0x00000000000103BF
  #_ REGISTER_IN r11 0x00000000000103C0

  subfe r3, r10, r11

  blr
  #_ REGISTER_OUT r10 0x00000000000103BF
  #_ REGISTER_OUT r11 0x00000000000103C0
  #_ REGISTER_OUT r3 0x1