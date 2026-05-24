cmake_minimum_required(VERSION 3.5)
project(critics_viz_rviz)

set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(rviz_common REQUIRED)
find_package(rviz_rendering REQUIRED)
find_package(pluginlib REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS
  Core Widgets Charts
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include_directories(include)

add_library(critics_viz_panel SHARED
  src/critics_panel.cpp
  include/critics_viz_rviz/critics_panel.hpp
)

target_include_directories(critics_viz_panel PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Charts_INCLUDE_DIRS}
)

target_link_libraries(critics_viz_panel
  Qt5::Core
  Qt5::Widgets
  Qt5::Charts
  rviz_common::rviz_common
)

ament_target_dependencies(critics_viz_panel
  rclcpp
  nav2_msgs
  rviz_common
  rviz_rendering
  pluginlib
)

pluginlib_export_plugin_description_file(rviz_common plugin_description.xml)

install(TARGETS critics_viz_panel
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES plugin_description.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(critics_viz_panel)
ament_export_dependencies(
  rclcpp nav2_msgs rviz_common rviz_rendering pluginlib
  Qt5Core Qt5Widgets Qt5Charts
)

ament_package()
