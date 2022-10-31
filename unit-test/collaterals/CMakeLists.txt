

find_program(QUARTUS_PFG NAMES quartus_pfg)

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output_file_jic.rpd ${CMAKE_CURRENT_BINARY_DIR}/app1.rpd ${CMAKE_CURRENT_BINARY_DIR}/app2.rpd ${CMAKE_CURRENT_BINARY_DIR}/app3.rpd ${CMAKE_CURRENT_BINARY_DIR}/factory_update.rpd
                    COMMENT "Generating collateral files for unit-test"
                    COMMAND cp -vf ${CMAKE_CURRENT_SOURCE_DIR}/factory_p1p2p3.pfg ${CMAKE_CURRENT_BINARY_DIR}/
                    COMMAND cp -vf ${CMAKE_CURRENT_SOURCE_DIR}/bl2.hex ${CMAKE_CURRENT_BINARY_DIR}/
                    COMMAND cp -vf ${CMAKE_CURRENT_SOURCE_DIR}/fip.bin ${CMAKE_CURRENT_BINARY_DIR}/
                    COMMAND cp -vf ${CMAKE_CURRENT_SOURCE_DIR}/factory.sof ${CMAKE_CURRENT_BINARY_DIR}/
                    COMMAND ${QUARTUS_PFG} -c factory_p1p2p3.pfg
                    COMMAND rm -f factory_hps_auto.sof
                    COMMAND ${QUARTUS_PFG} -c factory.sof app1.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o start_address=0x00000 -o bitswap=OFF
                    COMMAND rm -f factory_hps_auto.sof
                    COMMAND ${QUARTUS_PFG} -c factory.sof app2.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o start_address=0x5000000 -o bitswap=OFF
                    COMMAND rm -f factory_hps_auto.sof
                    COMMAND ${QUARTUS_PFG} -c factory.sof app3.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o bitswap=OFF
                    COMMAND rm -f factory_hps_auto.sof
                    COMMAND ${QUARTUS_PFG} -c factory.sof factory_update.rpd -o hps_path=./bl2.hex -o mode=ASX4 -o bitswap=OFF
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/factory_p1p2p3.pfg ${CMAKE_CURRENT_SOURCE_DIR}/bl2.hex ${CMAKE_CURRENT_SOURCE_DIR}/fip.bin ${CMAKE_CURRENT_SOURCE_DIR}/factory.sof
)

add_custom_target(generate_collaterals
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/output_file_jic.rpd ${CMAKE_CURRENT_BINARY_DIR}/app1.rpd ${CMAKE_CURRENT_BINARY_DIR}/app2.rpd ${CMAKE_CURRENT_BINARY_DIR}/app3.rpd ${CMAKE_CURRENT_BINARY_DIR}/factory_update.rpd
)

set(COLLATERALS "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
