cmake_minimum_required(VERSION 2.8)
project(iomcontroller)

add_executable(iomcontroller iomcontroller.c shm_iom_in.c )
add_executable(iomserial iomserial.c shm_iom_in.c shm_iom_out.c)
add_executable(iomdataserver iomdataserver.c)

target_link_libraries(iomcontroller pthread rt)
target_link_libraries(iomserial pthread rt)
target_link_libraries(iomdataserver)

install(TARGETS iomcontroller DESTINATION bin)
install(TARGETS iomserial DESTINATION bin)
install(TARGETS iomdataserver DESTINATION bin)
