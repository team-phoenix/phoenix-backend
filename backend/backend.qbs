import qbs 1.0
import qbs.File
import "utilities.qbs" as CommonPaths

QtApplication {
    id: app;
    name: "backend"

    property stringList commonPaths: {
        var paths = [];

        if ( qbs.targetOS.contains( "linux" ) ) {
            paths.push( "/usr/lib/x86_64-linux-gnu/" )
        }

        return paths;
    }

    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "test"] }

    cpp.dynamicLibraries: [ "SDL2" ]

    cpp.includePaths: {

        var sdlPath = "/usr/include/SDL2/"

        var dependencyPaths = [
                sdlPath,
                ]

        var projectPaths = [
            "cpp/src"
            , "cpp/include"
            , "cpp/include/audio"
            , "cpp/include/input"
            , "cpp/src/audio/"
            , "cpp/src/input/"
        ]

        return projectPaths.concat( dependencyPaths )
    }

    files: {

        var headers = [
                    "cpp/include/*.h",
                    "cpp/include/audio/*.h",
                    "cpp/include/input/*.h",
                ]

        var sources = [
                    "cpp/src/*cpp",
                    "cpp/src/audio/*.cpp",
                    "cpp/src/input/*.cpp",
                ]

        return headers.concat( sources ).concat( commonPaths )
    }
}
