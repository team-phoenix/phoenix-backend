import qbs 1.0

Project {
    id: root
    name: "phoenix"

    SubProject {
        filePath: "backend/backend.qbs"
        Properties {
            name: "backend"
        }
    }

    SubProject {
        filePath: "frontend/frontend.qbs"
        Properties {
            name: "frontend"
        }
    }
}
