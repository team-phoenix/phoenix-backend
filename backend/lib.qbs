import qbs

StaticLibrary {
    id: lib
    name: "libbackend"
    property stringList commonPaths: {
        var paths = []

        if (qbs.targetOS.contains("linux")) {
            paths.push("/usr/lib/x86_64-linux-gnu/")
        } else if (qbs.targetOS.contains("windows")) {
            paths.push("C:/msys64/mingw64/include")
        }

        return paths
    }

    property var srcDirs: ["src", "src/audio", "src/input", "src/core", "src/ipc", "src/loop", "src/utils"]

    Depends {
        name: "Qt"
        submodules: ["core", "multimedia", "gui"]
    }

    cpp.includePaths: {
        var dirs = []
        srcDirs.forEach(function (iter) {
            dirs.push(iter)
        })

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
        var files = []
        srcDirs.forEach(function (iter) {
            files.push(iter + "/*.h", iter + "/*.hpp", iter + "/*.cpp")
        })

        files.push("externals/sdl_gamecontrollerdb/*.qrc")
        return files.concat(commonPaths)
    }

    Export {
        cpp.includePaths: {
            var dirs = []
            lib.srcDirs.forEach(function (iter) {
                dirs.push(iter)
            })
            return dirs
        }
        cpp.cxxLanguageVersion: "c++14"

        Depends {
            name: "cpp"
        }
        Depends {
            name: "Qt"
            submodules: ["core", "multimedia", "gui"]
        }
    }
}
