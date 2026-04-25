add_library(caerwyn-gui STATIC)

target_compile_features(caerwyn-gui PUBLIC cxx_std_23)

set_target_properties(caerwyn-gui PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    CXX_SCAN_FOR_MODULES ON
)

find_package(raylib CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(caerwyn-gui
    PUBLIC
        raylib
        glm::glm
)

target_sources(caerwyn-gui
    PUBLIC
        FILE_SET caerwyn_gui_modules
        TYPE CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            Widget.ixx
            BoxLayout.ixx
            RowLayout.ixx
            ColumnLayout.ixx
            RichTextLabel.ixx
            Viewport.ixx
            Scrollbar.ixx
            ScrollView.ixx
            Gui.ixx
)
