import qbs 1.0
import qbs.File

QtApplication {
    id: app
    name: "frontend"

    Depends {
        name: "libfrontend"
    }

    files: ["app/main.cpp", "qml/*.qrc"]

    destinationDirectory: "phoenix/" + app.name

    Group {
        name: "Databases"
        qbs.install: true
        qbs.installDir: "databases"
        qbs.installPrefix: app.name
        files: ["externals/*.sqlite"]
    }
}
