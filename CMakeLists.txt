cmake_minimum_required(VERSION 3.26)
project(unsafe)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    file(CONFIGURE OUTPUT Directory.Build.props CONTENT [[
      <Project>
        <PropertyGroup>
          <UseMultiToolTask>false</UseMultiToolTask>
          <VcpkgEnabled>false</VcpkgEnabled>
        </PropertyGroup>
        <ItemDefinitionGroup>
          <ClCompile>
            <BuildStlModules>false</BuildStlModules>
            <MultiProcessorCompilation>true</MultiProcessorCompilation>
          </ClCompile>
        </ItemDefinitionGroup>
      </Project>
    ]] @ONLY)
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

if(NOT DEFINED BUILD_TESTING OR BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
