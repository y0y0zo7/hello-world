#!/usr/bin/env python3
"""Static assignment guardrails that complement the compiled interaction test."""

from __future__ import annotations

import hashlib
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FAN_CPP = (ROOT / "Scene" / "Fan.cpp").read_text(encoding="utf-8")
FAN_H = (ROOT / "Scene" / "Fan.h").read_text(encoding="utf-8")

failures: list[str] = []


def check(name: str, condition: bool, detail: str) -> None:
    marker = "PASS" if condition else "FAIL"
    print(f"[{marker}] {name}: {detail}")
    if not condition:
        failures.append(name)


def sha256(relative_path: str) -> str:
    return hashlib.sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


required_files = [
    ".github/workflows/android.yml",
    ".github/workflows/desktop.yml",
    ".github/workflows/pages.yml",
    "Scene/Fan.cpp",
    "Scene/Fan.h",
    "Scene/Model.h",
    "Scene/Renderer.cpp",
    "Scene/ShaderHelper.h",
    "Scene/Transform.cpp",
    "Scene/Transform.h",
    "android/app/src/main/cpp/NativeTemplate.cpp",
    "android/app/src/main/java/com/example/fan3d/MainActivity.java",
    "android/app/src/main/assets/shader/FanVertex.glsl",
    "android/app/src/main/assets/shader/FanFragment.glsl",
    "tests/FanInteractionTests.cpp",
    "screenshots/android.png",
    "screenshots/desktop.png",
    "screenshots/web.png",
]
check(
    "Required submission files",
    all((ROOT / path).is_file() for path in required_files),
    "shared source, three platform entry points, tests, shaders, and screenshots exist",
)

check(
    "Supplied Transform files unchanged",
    sha256("Scene/Transform.h")
    == "0A64FD5386D92AF172365ABC7557ABCCF34A70033F6BDCEE72B0D01045556734"
    and sha256("Scene/Transform.cpp")
    == "9E19E25C0094BDA66D12FC4345E09D602AD7F37088803DC10082F21A9A3D5800",
    "SHA-256 hashes match the instructor-provided files",
)

check(
    "Core 1 - one indexed cube",
    "constexpr GLfloat kCubePositions[]" in FAN_CPP
    and "constexpr GLushort kCubeIndices[]" in FAN_CPP
    and "glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr)" in FAN_CPP,
    "eight shared vertices and 36 indexed elements are used",
)
check(
    "Core 2 - one VBO, IBO, and VAO",
    FAN_CPP.count("glGenVertexArrays(1, &vao)") == 1
    and FAN_CPP.count("glGenBuffers(1, &vbo)") == 1
    and FAN_CPP.count("glGenBuffers(1, &ibo)") == 1
    and FAN_CPP.count("glBufferSubData(") == 2,
    "positions and shades occupy two sub-regions in one VBO",
)

world_transform = [
    "transform.TransformTranslate(0.0f, 0.8f, -8.0f)",
    "transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f)",
]
hierarchy = [
    "transform.TransformTranslate(0.0f, -2.6f, 0.0f)",
    "transform.TransformScale(1.6f, 0.25f, 0.8f)",
    "transform.TransformTranslate(0.0f, -1.21f, 0.0f)",
    "transform.TransformScale(0.15f, 2.53f, 0.15f)",
    "transform.TransformTranslate(0.0f, 0.2f, 0.0f)",
    "const float bladeSpacing = 360.0f / static_cast<float>(bladeCount)",
    "for (int blade = 0; blade < bladeCount; ++blade)",
    "spinAngle + blade * bladeSpacing",
]
check(
    "Core 3 - hierarchical scene",
    all(token in FAN_CPP for token in world_transform + hierarchy)
    and "bladeCount = 4" in FAN_H
    and FAN_CPP.count("TransformPushMatrix()") == FAN_CPP.count("TransformPopMatrix()"),
    "the exact world transform and balanced base/pole/hub/blade branches are present",
)

check(
    "Core 4 - per-part colour and depth shading",
    all(
        token in FAN_CPP
        for token in (
            "kBaseColor",
            "kPoleColor",
            "kHubColor",
            "kBladeColors",
            'glGetUniformLocation(program, "PARTCOLOR")',
            "glUniform3fv(partColorUniform, 1, color)",
            "glEnable(GL_DEPTH_TEST)",
        )
    ),
    "seven parts share the cube while uploading the required colour uniform",
)

check(
    "Core 5 - fan animation",
    all(
        token in FAN_H + FAN_CPP
        for token in (
            "kBaseSpeed = 1.5f",
            "spinAngle += kBaseSpeed + dragBoost",
            "if (fanOn)",
        )
    ),
    "the fan starts on and advances at 1.5 degrees per frame plus drag boost",
)

check(
    "Core 6 - tap and chrono drag controls",
    all(
        token in FAN_H + FAN_CPP
        for token in (
            "kTapThreshold = 12.0f",
            "kBoostScale = 8.0f",
            "kMaxBoost = 20.0f",
            "std::chrono::steady_clock",
            "fanOn = !fanOn",
            "dragBoost = 0.0f",
            'LOGI("Fan %s"',
            'LOGI("Swipe velocity',
        )
    ),
    "tap toggling, velocity scaling, clamping, logging, and release reset are present",
)

check(
    "Stretch goal - runtime blade count",
    all(
        token in FAN_H + FAN_CPP
        for token in (
            "kLongPressMs = 600.0f",
            "void CycleBladeCount()",
            "bladeCount = bladeCount >= 5 ? 3 : bladeCount + 1",
            "360.0f / static_cast<float>(bladeCount)",
            'LOGI("Blade count %d"',
        )
    ),
    "a long press cycles 3/4/5 blades and spaces them by 360 degrees / count",
)

check(
    "Core 7 - cross-platform delivery",
    (ROOT / "CMakeLists.txt").is_file()
    and (ROOT / "android" / "app" / "build.gradle").is_file()
    and (ROOT / "build_web.sh").is_file()
    and "PLATFORM_ANDROID" in (ROOT / "Scene" / "Platform.h").read_text(encoding="utf-8")
    and "PLATFORM_EMSCRIPTEN" in (ROOT / "Scene" / "Platform.h").read_text(encoding="utf-8"),
    "Desktop, Android, and Emscripten use the same Scene implementation",
)

# Once the project is tracked, fail CI if generated or assignment-input files leak in.
try:
    repository_root = Path(
        subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            cwd=ROOT,
            check=True,
            capture_output=True,
            text=True,
        ).stdout.strip()
    )
    project_pathspec = ROOT.relative_to(repository_root).as_posix()
    tracked_result = subprocess.run(
        ["git", "ls-files", "--", project_pathspec],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    tracked = [Path(line) for line in tracked_result.stdout.splitlines() if line]
except (FileNotFoundError, subprocess.CalledProcessError):
    tracked = []

forbidden_suffixes = {".apk", ".aab", ".exe", ".pdf", ".wasm"}
forbidden_parts = {
    "build",
    "build_ci",
    "build_web",
    "web",
    "external",
    ".gradle",
    ".idea",
    ".cxx",
}
leaked = [
    str(path)
    for path in tracked
    if path.suffix.lower() in forbidden_suffixes or forbidden_parts.intersection(path.parts)
]
check(
    "Repository hygiene",
    not leaked,
    "no tracked build products, APKs, executables, PDFs, or WebAssembly outputs"
    + (f"; leaked: {', '.join(leaked)}" if leaked else ""),
)

if failures:
    print(f"\n{len(failures)} verification check(s) failed: {', '.join(failures)}", file=sys.stderr)
    raise SystemExit(1)

print("\nAll Fan3D static rubric checks passed.")
