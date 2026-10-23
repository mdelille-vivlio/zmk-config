if(CONFIG_ZMK_DISPLAY AND CONFIG_SOFLE_DISPLAY_STATUS)
  target_sources(app PRIVATE src/status_screen.c src/util.c src/behavior_boot_screen.c)

  if(NOT CONFIG_ZMK_SPLIT OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    target_sources(app PRIVATE src/status.c src/keynames.c)
  else()
    target_sources(app PRIVATE src/peripheral.c)
  endif()
endif()
