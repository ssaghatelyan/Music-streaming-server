cmake_minimum_required(VERSION 3.16)
project(MusicStreamClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Multimedia)

add_executable(client
    main.cpp
    Client.cpp
    LoginWindow.cpp
    MainWindow.cpp
)

target_link_libraries(client PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
)
