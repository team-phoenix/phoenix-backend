import qbs 1.0

QtApplication {
    id: testApp

    Depends {
        name: "Qt"
        submodules: ["core", "multimedia", "gui", "test"]
    }

    files: {

        var headers = ["../backend/src/*.h", "../backend/src/*.hpp", "../externals/verdigris/*.h"]

        var sources = ["../backend/src/*.cpp", "../backend/cpp/src/logging.cpp", "cpp/src/*.cpp", "*.cpp", "snes_test_roms/*.qrc", "../externals/sdl_gamecontrollerdb/*.qrc"]

        return headers.concat(sources)
    }

    cpp.includePaths: {
        var dirs = []
        dirs.push("../backend/src")
        dirs.push("../backend/src/input")
        dirs.push("../backend/src/audio")

        dirs.push("cpp/include/integration")
        dirs.push("cpp/include/unit")

        var sdlPaths = [""]

        if (qbs.targetOS.contains("windows")) {
            sdlPaths.push("C:/msys64/mingw64/include/SDL2")
        }

        dirs.concat(sdlPaths)
        return dirs
    }

    cpp.cxxLanguageVersion: "c++14"
    cpp.dynamicLibraries: ["mingw32", "SDL2main", "SDL2"]
    cpp.staticLibraries: ["samplerate"]
    cpp.libraryPaths: {
        var paths = []

        if (qbs.targetOS.contains("windows")) {
            paths.push("C:/msys64/mingw64/lib/")
        }

        return paths
    }
}
