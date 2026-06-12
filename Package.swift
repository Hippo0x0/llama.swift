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
                "https://github.com/Hippo0x0/llama.cpp/releases/download/llama-mtmd-ios-macos-jinja-chat-20260612/llama-mtmd-ios-macos-xcframework-jinja-chat-20260612.zip",
            checksum: "2eea9d2c4f64eb0037fea889f76b38d00421f54c0a21f113b27d540bfcae41d2"
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
