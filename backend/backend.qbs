import qbs 1.0
import qbs.File
import "utilities.qbs" as CommonPaths

QtApplication {
    id: app
    name: "backend"

    property stringList commonPaths: {
        var paths = []

        if (qbs.targetOS.contains("linux")) {
            paths.push("/usr/lib/x86_64-linux-gnu/")
        } else if (qbs.targetOS.contains("windows")) {
            paths.push("C:/msys64/mingw64/include")
        }

        return paths
    }

    Depends {
        name: "Qt"
        submodules: ["core", "multimedia", "gui"]
    }

    cpp.includePaths: {
        var dirs = []
        dirs.push("src")

        if (qbs.targetOS.contains("windows")) {
            dirs.push("C:/msys64/mingw64/include/SDL2")
        }

        return dirs
    }

    cpp.cxxLanguageVersion: "c++14"
    cpp.dynamicLibraries: ["mingw32", "SDL2main", "SDL2"]
    cpp.libraryPaths: {
        var paths = []

        if (qbs.targetOS.contains("windows")) {
            paths.push("C:/msys64/mingw64/lib/")
        }

        return paths
    }

    files: {

        var headers = ["src/*.hpp", "src/*.h"]

        var sources = ["src/*.cpp", "src/main/*.cpp", "src/logging.cpp", "../externals/sdl_gamecontrollerdb/*.qrc"]

        return headers.concat(sources).concat(commonPaths)
    }
}
