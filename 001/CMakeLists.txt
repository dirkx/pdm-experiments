cmake_minimum_required(VERSION 3.13...3.27)
	
include(pico_sdk_import.cmake)
set(PICO_SDK_FETCH_FROM_GIT on)

project(pdmgenerate)
pico_sdk_init()

add_executable(pdmgenerate)

pico_generate_pio_header(pdmgenerate ${CMAKE_CURRENT_LIST_DIR}/pdmgenerate.pio)

target_sources(pdmgenerate PRIVATE pdmgenerate.c)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(pdmgenerate PRIVATE pico_stdlib hardware_pio)

# Convenience for burning
# add_custom_target(flash ALL
#   COMMAND /usr/local/bin/openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c 'adapter speed 5000' -c 'program pdmgenerate.elf reset exit'
# )

add_custom_target(flashW2 ALL
  COMMAND /usr/local/bin/openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c 'adapter speed 5000' -c 'program pdmgenerate.elf reset exit'
)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pdmgenerate)

