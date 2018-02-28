import qbs

StaticLibrary {
    id: lib
    name: "libfrontend"

    Depends {
        name: "Qt"
        submodules: ["core", "multimedia", "gui", "qml", "quick", "sql", "concurrent"]
    }

    property var srcDirs: {
        var dirs = ["include", "include/databases", "include/databases/systemdb", "include/databases/librarydb", "include/databases/openvgdb", "include/models", "include/models/gamemetadatamodel", "include/models/inputdevicesmodel", "include/models/systemlistmodel", "include/gameimporter"]
        return dirs
    }

    cpp.includePaths: srcDirs

    cpp.cxxLanguageVersion: "c++14"
    cpp.dynamicLibraries: ["mingw32"]

    files: {
        var headers = ["include/*.hpp", "include/*.h", "include/models/*.h", "include/models/gamemetadatamodel/*.h", "include/models/inputdevicesmodel/*.h", "include/models/systemlistmodel/*.h", "include/databases/*.h", "include/databases/librarydb/*.h", "include/databases/openvgdb/*.h", "include/gameimporter/*.h", "include/databases/systemdb/*.h"]

        var qmlFiles = ["qml/*.qrc", "qml/icons/*.qrc"]

        var sources = ["src/*.cpp", "src/models/*.cpp", "src/models/gamemetadatamodel/*.cpp", "src/models/systemlistmodel/*.cpp", "src/models/inputdevicesmodel/*.cpp", "src/databases/*.cpp", "src/databases/librarydb/*.cpp", "src/databases/openvgdb/*.cpp", "src/gameimporter/*.cpp", "src/databases/systemdb/*.cpp"]

        return headers.concat(sources).concat(qmlFiles)
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
            submodules: ["core", "multimedia", "gui", "qml", "quick", "sql", "concurrent"]
        }
    }
}
