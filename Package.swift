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
                "https://github.com/Hippo0x0/llama.cpp/releases/download/llama-mtmd-ios-macos-thinking-budget-20260606/llama-mtmd-ios-macos-xcframework-thinking-budget-20260606.zip",
            checksum: "f69b1f96334874cb0f9b450c67687810c142327cb81692577c711c1b54a84a49"
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
