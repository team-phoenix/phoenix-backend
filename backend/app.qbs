import qbs

QtApplication {
    id: app
    name: "backend"

    files: ["src/main.cpp"]

    Depends {
        name: "libbackend"
    }

    Depends {
        name: "cpp"
    }
}
