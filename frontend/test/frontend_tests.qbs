import qbs 1.0
import qbs.File

QtApplication {
    id: app;
    name: "frontend_tests"
    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "qml", "sql"] }

    cpp.includePaths: {
        var dirs = ["include"
                    , "../cpp/include"
                    , "../cpp/include/models"
                    , "../cpp/include/models/librarydb"
                    , "../cpp/include/models/openvgdb"];
        dirs.push( "include")
        return dirs;
    }

    cpp.cxxLanguageVersion: "c++14";
    cpp.dynamicLibraries: [ "mingw32" ]
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

    files: {
        var headers = ["include/*.hpp"
                       , "../cpp/include/*.h"
                       , "../cpp/include/models/*.h"
                       , "../cpp/include/models/librarydb/*.h"
                       , "../cpp/include/models/openvgdb/*.h"]

        var sources = ["src/*.cpp"
                       , "src/models/*.cpp"
                       , "../cpp/src/logging.cpp"
                       , "../cpp/src/models/*.cpp"
                       , "../cpp/src/models/librarydb/*.cpp"
                       , "../cpp/src/models/openvgdb/*.cpp"
                       ];

        return headers.concat( sources );
    }
}
