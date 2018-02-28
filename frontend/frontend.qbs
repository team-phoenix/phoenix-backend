import qbs

Project {
    id: frontend
    name: "frontend"

    SubProject {
        filePath: "cpp/app.qbs"
        Properties {
            name: "app-frontend"
        }
    }

    SubProject {
        filePath: "cpp/lib.qbs"
        Properties {
            name: "lib-frontend"
        }
    }

    SubProject {
        filePath: "cpp/test-frontend.qbs"
        Properties {
            name: "test-frontend"
        }
    }
}
