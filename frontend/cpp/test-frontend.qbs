import qbs
import qbs.File

QtApplication {
    id: app
    name: "tests-frontend"
    Depends {
        name: "Qt"
        submodules: ["test"]
    }

    Depends {
        name: "libfrontend"
    }

    property var srcDirs: {
        var paths = ["test/src/", "test/src/import/", "test/src/ipc/", "test/src/models/", "test/src/utils/"]
        return paths
    }

    cpp.includePaths: {
        var dirs = []
        srcDirs.forEach(function (iter) {
            dirs.push(iter)
        })
        return dirs
    }

    cpp.cxxLanguageVersion: "c++14"
    cpp.dynamicLibraries: ["mingw32"]
    destinationDirectory: "phoenix/" + app.name

    files: {
        var files = []
        srcDirs.forEach(function (iter) {
            files.push(iter + "*.cpp", iter + "*.h", iter + "*.hpp")
        })
        return files
    }

    Group {
        name: "Databases"
        qbs.install: true
        qbs.installDir: "databases"
        qbs.installPrefix: app.name
        files: ["test/externals/*.sqlite"]
    }

    Group {
        name: "External Files"
        qbs.install: true
        qbs.installDir: "testFiles"
        qbs.installPrefix: app.name
        files: ["test/externals/*.nes"]
    }
}
