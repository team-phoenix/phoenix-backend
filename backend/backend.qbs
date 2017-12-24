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
        } else if ( qbs.targetOS.contains( "windows" ) ) {
            paths.push( "C:/msys64/mingw64/include" )
        }

        return paths;

    }

    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "test"] }

    cpp.dynamicLibraries: [ "mingw32", "SDL2main", "SDL2" ]
    cpp.libraryPaths: {
        var paths = []

        if ( qbs.targetOS.contains( "windows" ) ) {
            paths.push( "C:/msys64/mingw64/lib/" )
        }

        return paths;
    }

    cpp.includePaths: {

        var sdlPath = "/usr/include/SDL2/"

        var dependencyPaths = [
                sdlPath,
                    "C:/msys64/mingw64/include",
                    "C:/msys64/mingw64/include/SDL2",
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
                    "cpp/src/main/*.cpp"
                ]

        return headers.concat( sources ).concat( commonPaths )
    }
}
