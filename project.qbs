import qbs 1.0

Project {
    name: "phoenix"

    SubProject {
        filePath: "backend/backend.qbs"
        Properties {
            name: "backend"
        }
    }

    SubProject {
        filePath: "test/test.qbs"
        Properties {
            name: "test"
        }
    }

}
