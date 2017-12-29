import qbs 1.0
import qbs.File

QtApplication {
    id: app;
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
        var sources = ["src/*.cpp"];

        return headers.concat( sources );
    }
}
