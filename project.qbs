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
        filePath: "frontend/test/frontend_tests.qbs"
        Properties {
            name: "frontend_tests"
        }
    }

    SubProject {
        filePath: "test/backend_tests.qbs"
        Properties {
            name: "backend_tests"
        }
    }

}
