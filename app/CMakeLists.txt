add_executable(spigot-builder main.cpp "MainWidget.h" "MainWidget.cpp")
target_link_libraries(spigot-builder Qt6::Widgets Qt6::Network)
set_target_properties(spigot-builder PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)