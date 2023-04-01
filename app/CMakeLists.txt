add_executable(spigot-builder main.cpp "MainWidget.h" "MainWidget.cpp" JdkPrepareTask.cpp JdkPrepareTask.h)
target_link_libraries(spigot-builder Qt6::Widgets Qt6::Network LibArchive::LibArchive)
set_target_properties(spigot-builder PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
add_custom_command(TARGET spigot-builder
                   POST_BUILD
                   COMMAND cmake ARGS -D target_exe=spigot-builder.exe -D search_path="${LibArchive_INCLUDE_DIR}/../bin" -P ${CMAKE_CURRENT_LIST_DIR}/bundle.cmake
                   COMMAND ${Qt6_DIR}/../../../bin/windeployqt.exe ${CMAKE_CURRENT_BINARY_DIR}/dist/spigot-builder.exe
                   )