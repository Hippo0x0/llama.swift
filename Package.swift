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
            path: "../llama.cpp/build-apple/llama.xcframework"
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
