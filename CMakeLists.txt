
set(PLUGIN_NAME "datetime")
project(${PLUGIN_NAME})

set(CMAKE_AUTOMOC ON)
# 启用 qrc 资源文件的支持
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Sources files
set(SRCS "datetimeplugin.cpp"
    "datetimewidget.cpp"
    "weekitem.cpp"
    "weekwidget.cpp"
    "weekwidget.ui"
    "settingdialog.cpp"
    "settings.ui"
    "../../widgets/tipswidget.cpp")

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)

pkg_check_modules(DdeDockInterface REQUIRED dde-dock)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} datetime.qrc)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)

target_include_directories(${PLUGIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DdeDockInterface_INCLUDE_DIRS}
  )

  target_link_libraries(${PLUGIN_NAME} PRIVATE
  ${Qt5DBus_LIBRARIES}
  ${DtkWidget_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
)

# install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)
