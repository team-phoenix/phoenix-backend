import qbs 1.0
import qbs.File

QtApplication {
    id: app;
    name: "frontend"
    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "qml", "sql"] }

    cpp.includePaths: {
        var dirs = [ "include"
                    , "include/models"
                    , "include/models/librarydb"
                    , "include/models/openvgdb"];
        return dirs;
    }

    destinationDirectory: "phoenix/" + app.name;

    Group {
     name: "Databases";
     qbs.install: true;
     qbs.installDir: "databases";
     qbs.installPrefix: app.name;

     files: [
            "externals/*.sqlite",
        ]
    }

    cpp.cxxLanguageVersion: "c++14";
    cpp.dynamicLibraries: [ "mingw32" ]

    files: {
        var headers = ["include/*.hpp"
                       , "include/*.h"
                       , "include/models/*.h"
                       , "include/models/librarydb/*.h"
                       , "include/models/openvgdb/*.h"];

        var qmlFiles = ["qml/*.qrc"
                        , "qml/icons/*.qrc"];

        var sources = ["src/*.cpp"
                       , "src/models/*.cpp"
                       , "src/models/librarydb/*.cpp"
                       , "src/models/openvgdb/*.cpp"];

        return headers.concat( sources ).concat(qmlFiles);
    }
}
