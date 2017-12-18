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
        filePath: "frontend/cpp/frontend.qbs"
        Properties {
            name: "frontend"
        }
    }

    SubProject {
        filePath: "frontend/test/test_frontend.qbs"
        Properties {
            name: "test_frontend"
        }
    }

    SubProject {
        filePath: "test/test.qbs"
        Properties {
            name: "test"
        }
    }

}
