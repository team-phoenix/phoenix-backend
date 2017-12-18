import qbs 1.0
import qbs.File

QtApplication {
    id: app;
    name: "frontend"
    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "qml"] }

    cpp.includePaths: {
        var dirs = [];
        dirs.push( "include")
        return dirs;
    }

    cpp.cxxLanguageVersion: "c++14";
    cpp.dynamicLibraries: [ "mingw32" ]

    files: {
        var headers = ["include/*.hpp"]
        var qmlFiles = ["qml/*.qrc"];
        var sources = ["src/*.cpp"];

        return headers.concat( sources ).concat(qmlFiles);
    }
}
