import qbs

QtApplication {
    id: app
    name: "app-backend"

    files: ["src/main.cpp"]

    Depends {
        name: "libbackend"
    }

    Depends {
        name: "cpp"
    }
}
