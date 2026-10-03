// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "llama.swift",
    platforms: [
        .macOS(.v13),
        .iOS(.v16),
        .tvOS(.v16),
        .watchOS(.v9),
        .visionOS(.v1),
    ],
    products: [
        .library(
            name: "LlamaSwift",
            targets: ["LlamaSwift"]
        )
    ],
    targets: [
        .binaryTarget(
            name: "llama-cpp",
            url:
                "https://github.com/Hippo0x0/llama.cpp/releases/download/llama-gemma4-tools-20261003/llama-gemma4-tools-20261003.zip",
            checksum: "1afcb0143665ca28771f3a4ba1a9a250cbafa4601eae4a1e7f69f53d484342ff"
        ),
        .target(
            name: "LlamaSwift",
            dependencies: [
                "llama-cpp",
                "LlamaSwiftReasoning",
            ],
            path: "Sources/LlamaSwift"
        ),
        .target(
            name: "LlamaSwiftReasoning",
            dependencies: ["llama-cpp"],
            path: "Sources/LlamaSwiftReasoning",
            publicHeadersPath: "include"
        ),
        .testTarget(
            name: "LlamaTests",
            dependencies: ["LlamaSwift"]
        ),
    ]
)
