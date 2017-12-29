import qbs 1.0
import qbs.File

QtApplication {
    id: app;
    name: "frontend"
    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "qml", "sql"] }

    cpp.includePaths: {
        var dirs = [ "include", "include/models"];
        return dirs;
    }

    destinationDirectory: app.name;

    Group {
     name: "Databases";
     qbs.install: true;
     qbs.installDir: "databases";

     files: [
      "externals/*.sqlite"
     ]
    }

    cpp.cxxLanguageVersion: "c++14";
    cpp.dynamicLibraries: [ "mingw32" ]

    files: {
        var headers = ["include/*.hpp", "include/*.h", "include/models/*.h"]
        var qmlFiles = ["qml/*.qrc", "qml/icons/*.qrc"];
        var sources = ["src/*.cpp", "src/models/*.cpp"];

        return headers.concat( sources ).concat(qmlFiles);
    }
}
