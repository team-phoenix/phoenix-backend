import qbs

Project {
    id: backend
    name: "phoenix"

    SubProject {
        filePath: "app.qbs"
        Properties {
            name: "app-backend"
        }
    }

    SubProject {
        filePath: "lib.qbs"
        Properties {
            name: "lib-backend"
        }
    }

    SubProject {
        filePath: "test.qbs"
        Properties {
            name: "test-backend"
        }
    }
}
