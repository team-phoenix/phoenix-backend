import qbs 1.0

QtApplication {
	id: testApp;
	name: "test"

    Depends { name: "Qt"; submodules: ["core", "multimedia", "gui", "test"] }

    files: {

        var headers = [
                    "../backend/cpp/include/*.h",
                    "../backend/cpp/include/audio/*.h",
                    "../backend/cpp/include/input/*.h",

                    "cpp/include/integration/*.hpp",
                    "cpp/include/unit/*.hpp",

                    "cpp/src/*.cpp",
                ]

        var sources = [
                    "../backend/cpp/src/*.cpp",
                    "../backend/cpp/src/audio/*.cpp",
                    "../backend/cpp/src/input/*.cpp",
                    "cpp/src/*.cpp",
                    "*.h",
                    "*.cpp",

                    "snes_test_roms/*.qrc",
                 ]

        return headers.concat( sources )
    }

    cpp.includePaths: {
        var dirs = [];
        dirs.push( "../backend/cpp/include")
        dirs.push( "../backend/cpp/include/input" )
        dirs.push( "../backend/cpp/include/audio" )

        dirs.push( "cpp/include/integration" )
        dirs.push( "cpp/include/unit" )

        var sdlPaths = [ "" ];


        if ( qbs.targetOS.contains( "windows" ) ) {
            sdlPaths.push( "C:/msys64/mingw64/include/SDL2" );
        }

        dirs.concat( sdlPaths )
        return dirs
    }

    cpp.dynamicLibraries: [ "mingw32", "SDL2main", "SDL2" ]
    cpp.libraryPaths: {
        var paths = []

        if ( qbs.targetOS.contains( "windows" ) ) {
            paths.push( "C:/msys64/mingw64/lib/" )
        }

        return paths;
    }

}
