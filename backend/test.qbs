import qbs

QtApplication {
    name: "tests-backend"
    property var integrationTestPaths: {
        var basePath = "test/src/integration/"
        return [basePath + "core/", basePath + "ipc/", basePath + "input/"]
    }

    property var unitTestPaths: {
        var basePath = "test/src/unit/"
        return [basePath + "core/", basePath + "ipc/", basePath + "input/", basePath + "audio/"]
    }

    property var srcPaths: {
        var paths = ["test/src/"].concat(integrationTestPaths).concat(
                    unitTestPaths)
        return paths
    }
    cpp.includePaths: {
        var dirs = []
        srcPaths.forEach(function (iter) {
            dirs.push(iter)
        })
        return dirs
    }

    files: {
        var files = []
        files.push("./externals/sdl_gamecontrollerdb/*.qrc")
        srcPaths.forEach(function (iter) {
            files.push(iter + "*.cpp", iter + "*.h", iter + "*.hpp")
        })
        return files
    }

    Depends {
        name: "libbackend"
    }
    Depends {
        name: "cpp"
    }
}
