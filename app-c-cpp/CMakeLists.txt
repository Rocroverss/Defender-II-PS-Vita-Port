cmake_minimum_required(VERSION 3.14)

if (NOT DEFINED ENV{VITASDK} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(FATAL_ERROR "Please define VITASDK env var to point to your SDK path or set CMAKE_TOOLCHAIN_FILE.")
endif ()

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "Toolchain file")
endif ()

project(defender2_vita C CXX)

if (DEFINED ENV{VITASDK})
  set(VITASDK "$ENV{VITASDK}")
endif ()

include("${VITASDK}/share/vita.cmake" REQUIRED)

# Vita application configuration
set(VITA_APP_NAME "Defender II Vita Port")
set(VITA_TITLEID  "DDEF00001")
set(VITA_VPKNAME  "defender2_vita")
set(VITA_VERSION  "01.00")
set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12")

# Optional: used for "send" and "reboot" targets
set(PSVITAIP "192.168.1.1" CACHE STRING "PSVita IP (for FTP access)")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wl,-q -mfloat-abi=softfp -std=gnu11 -Wno-deprecated -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-q -mfloat-abi=softfp -std=gnu++17 -fno-rtti -Wno-write-strings -Wno-psabi")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O1 -g3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -g3")
else ()
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O3 -g0 -ffast-math")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g0 -ffast-math")
endif ()

set(PROJECT_SOURCES
  src/main.cpp
  src/engine/abstract_game.cpp
  src/engine/bmp_scaler.cpp
  src/engine/coordinate_mapper.cpp
  src/engine/font_renderer.cpp
  src/engine/sprite_atlas_cache.cpp
  src/engine/screen.cpp
  src/engine/time_utils.cpp
  src/game/game.cpp
  src/game/achv_mng.cpp
  src/game/help_overlay.cpp
  src/game/item_cost.cpp
  src/game/item_param.cpp
  src/game/save.cpp
  src/game/skill_data.cpp
  src/game/sounds.cpp
  src/game/bow_data.cpp
  src/game/audio_manager.cpp
  src/game/umeng_helper.cpp
  src/game/xp_data.cpp
  src/scenes/cover_scene.cpp
  src/scenes/loading_scene.cpp
  src/scenes/main_game_scene.cpp
  src/scenes/maingame/arrow.cpp
  src/scenes/maingame/bow.cpp
  src/scenes/maingame/formation.cpp
  src/scenes/maingame/magic.cpp
  src/scenes/maingame/mission.cpp
  src/scenes/maingame/monster.cpp
  src/scenes/maingame/panel.cpp
  src/scenes/maingame/wall.cpp
  src/scenes/maingame/wall_defender.cpp
  src/scenes/online_data_scene.cpp
  src/scenes/research/bow_button.cpp
  src/scenes/research/description.cpp
  src/scenes/research/equip_button.cpp
  src/scenes/research/equip_zone.cpp
  src/scenes/research/item_zone.cpp
  src/scenes/research/research_button.cpp
  src/scenes/research/research_render.cpp
  src/scenes/research/update_area.cpp
  src/scenes/research_scene.cpp
  src/scenes/shop_scene.cpp
  src/scenes/scene.cpp
  src/scenes/stats_scene.cpp
  src/scenes/solid_scene.cpp
  src/scenes/image_scene.cpp
  src/engine/texture_cache.cpp
)

add_executable(${CMAKE_PROJECT_NAME}
  ${PROJECT_SOURCES}
)

target_include_directories(${CMAKE_PROJECT_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${CMAKE_PROJECT_NAME}
  vitaGL
  vitashark

  png
  jpeg
  z

  SceShaccCgExt
  taihen_stub
  SceShaccCg_stub
  SceGxm_stub
  SceDisplay_stub
  SceCtrl_stub
  SceRtc_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceCommonDialog_stub
  SceIme_stub
  ScePower_stub
  SceSysmodule_stub
  SceKernelDmacMgr_stub
  ScePgf_stub
  SceTouch_stub
  SceLibKernel_stub
  mathneon
  stdc++
  m
  c
)

find_library(SCE_AUDIO_LIB
  NAMES SceAudioOut_stub SceAudioOut_stub_weak SceAudio_stub SceAudio_stub_weak
  PATHS "${VITASDK}/arm-vita-eabi/lib"
  NO_DEFAULT_PATH
)
if (SCE_AUDIO_LIB)
  target_link_libraries(${CMAKE_PROJECT_NAME} ${SCE_AUDIO_LIB})
else ()
  message(FATAL_ERROR "No Vita audio stub library found. Expected one of: SceAudioOut_stub, SceAudio_stub.")
endif ()

find_library(OGG_LIB
  NAMES ogg
  PATHS "${VITASDK}/arm-vita-eabi/lib"
  NO_DEFAULT_PATH
)
find_library(VORBIS_LIB
  NAMES vorbis
  PATHS "${VITASDK}/arm-vita-eabi/lib"
  NO_DEFAULT_PATH
)
find_library(VORBISFILE_LIB
  NAMES vorbisfile
  PATHS "${VITASDK}/arm-vita-eabi/lib"
  NO_DEFAULT_PATH
)
if (OGG_LIB AND VORBIS_LIB AND VORBISFILE_LIB)
  target_link_libraries(${CMAKE_PROJECT_NAME} ${VORBISFILE_LIB} ${VORBIS_LIB} ${OGG_LIB})
else ()
  message(STATUS "ogg/vorbis libraries not found in VitaSDK; OGG audio decode may be unavailable.")
endif ()

find_library(SCE_SHARED_FB_LIB
  NAMES SceSharedFb_stub SceSharedFb_stub_weak
  PATHS "${VITASDK}/arm-vita-eabi/lib"
  NO_DEFAULT_PATH
)
if (SCE_SHARED_FB_LIB)
  target_link_libraries(${CMAKE_PROJECT_NAME} ${SCE_SHARED_FB_LIB})
else ()
  message(STATUS "SceSharedFb stub library not found in VitaSDK; skipping explicit SharedFb link.")
endif ()

find_library(FREETYPE_LIB
  NAMES freetype
  PATHS "${VITASDK}/arm-vita-eabi/lib"
  NO_DEFAULT_PATH
)
if (FREETYPE_LIB)
  find_library(BZIP2_LIB
    NAMES bz2 bzip2
    PATHS "${VITASDK}/arm-vita-eabi/lib"
    NO_DEFAULT_PATH
  )
  if (BZIP2_LIB)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${FREETYPE_LIB} ${BZIP2_LIB})
  else ()
    message(STATUS "bzip2 path lookup failed; falling back to linker names bz2/bzip2.")
    target_link_libraries(${CMAKE_PROJECT_NAME} ${FREETYPE_LIB} bz2 bzip2)
  endif ()
  if (EXISTS "${VITASDK}/arm-vita-eabi/include/freetype2")
    target_include_directories(${CMAKE_PROJECT_NAME}
      PRIVATE
        "${VITASDK}/arm-vita-eabi/include/freetype2"
    )
  endif ()
else ()
  message(STATUS "freetype not found in VitaSDK; TTF/OTF label rendering will be disabled.")
endif ()

vita_create_self(eboot.bin ${CMAKE_PROJECT_NAME} UNSAFE NOASLR)

set(VITA_PACKAGE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(LIVEAREA_DIR "${VITA_PACKAGE_ROOT}/livearea")
file(GLOB_RECURSE ASSET_FILES
  CONFIGURE_DEPENDS
  RELATIVE "${VITA_PACKAGE_ROOT}"
  LIST_DIRECTORIES false
  "${VITA_PACKAGE_ROOT}/assets/*"
)

# Keep audio packaging explicit so newly added files under assets/audio are
# always considered by VPK generation.
file(GLOB_RECURSE AUDIO_ASSET_FILES
  CONFIGURE_DEPENDS
  RELATIVE "${VITA_PACKAGE_ROOT}"
  LIST_DIRECTORIES false
  "${VITA_PACKAGE_ROOT}/assets/audio/*"
)
list(APPEND ASSET_FILES ${AUDIO_ASSET_FILES})
list(REMOVE_DUPLICATES ASSET_FILES)

set(_AUDIO_ASSET_FILES ${ASSET_FILES})
list(FILTER _AUDIO_ASSET_FILES INCLUDE REGEX "^assets/audio/")
list(LENGTH _AUDIO_ASSET_FILES AUDIO_ASSET_COUNT)
message(STATUS "Audio assets queued for VPK: ${AUDIO_ASSET_COUNT}")

set(ASSET_FILE_ARGS)
foreach(ASSET_REL_PATH IN LISTS ASSET_FILES)
  list(APPEND ASSET_FILE_ARGS "${VITA_PACKAGE_ROOT}/${ASSET_REL_PATH}" "${ASSET_REL_PATH}")
endforeach()

set(LIVEAREA_FILE_ARGS)
foreach(LIVEAREA_FILE_NAME
    IN ITEMS
      icon0.png
      pic0.png
      startup.png
      bg0.png
      template.xml)
  set(LIVEAREA_SOURCE_PATH "${LIVEAREA_DIR}/${LIVEAREA_FILE_NAME}")
  if (EXISTS "${LIVEAREA_SOURCE_PATH}")
    if (LIVEAREA_FILE_NAME STREQUAL "icon0.png")
      list(APPEND LIVEAREA_FILE_ARGS "${LIVEAREA_SOURCE_PATH}" "sce_sys/icon0.png")
    elseif (LIVEAREA_FILE_NAME STREQUAL "pic0.png")
      list(APPEND LIVEAREA_FILE_ARGS "${LIVEAREA_SOURCE_PATH}" "sce_sys/pic0.png")
    else ()
      list(APPEND LIVEAREA_FILE_ARGS "${LIVEAREA_SOURCE_PATH}" "sce_sys/livearea/contents/${LIVEAREA_FILE_NAME}")
    endif ()
  endif ()
endforeach()

set(VPK_FILE_ARGS ${LIVEAREA_FILE_ARGS} ${ASSET_FILE_ARGS})

if (VPK_FILE_ARGS)
  vita_create_vpk(${VITA_VPKNAME}.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME    ${VITA_APP_NAME}
    FILE    ${VPK_FILE_ARGS}
  )
else ()
  vita_create_vpk(${VITA_VPKNAME}.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME    ${VITA_APP_NAME}
  )
endif ()

add_custom_target(send
  COMMAND echo destroy | nc ${PSVITAIP} 1338
  COMMAND echo destroy | nc ${PSVITAIP} 1338
  COMMAND curl -T eboot.bin ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/
  COMMAND curl -T ${VITA_PACKAGE_ROOT}/assets/audio/cover_bgm.wav ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/assets/audio/cover_bgm.wav
  COMMAND curl -T ${VITA_PACKAGE_ROOT}/assets/audio/game_bgm.wav ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/assets/audio/game_bgm.wav
  COMMAND curl -T ${VITA_PACKAGE_ROOT}/assets/audio/boss.wav ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/assets/audio/boss.wav
  COMMAND echo launch ${VITA_TITLEID} | nc ${PSVITAIP} 1338
  DEPENDS ${VITA_VPKNAME}.vpk-vpk
)

add_custom_target(reboot
  COMMAND echo reboot | nc ${PSVITAIP} 1338
)
