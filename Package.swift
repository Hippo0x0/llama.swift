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
                "https://github.com/Hippo0x0/llama.cpp/releases/download/copytain-ios-vision-20261001/llama-copytain-ios-vision-20261001.zip",
            checksum: "477bdc54390242b8f9fff877688d823ad38b0068807f66ada44494e25d33a3bf"
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
