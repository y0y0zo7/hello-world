#!/usr/bin/env python3
"""Static Programming Quiz 3 rubric guardrails.

The compiled CTest suite covers state transitions. This script protects the
submission structure, render/input ordering, shader features, screenshots,
cross-platform CI entries, and repository hygiene.
"""

from __future__ import annotations

import hashlib
import struct
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
REPOSITORY = ROOT.parent


def read(relative_path: str) -> str:
    return (ROOT / relative_path).read_text(encoding="utf-8")


def sha256(relative_path: str) -> str:
    return hashlib.sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


failures: list[str] = []


def check(name: str, condition: bool, detail: str) -> None:
    print(f"[{'PASS' if condition else 'FAIL'}] {name}: {detail}")
    if not condition:
        failures.append(name)


required_files = [
    "Scene/Fan.cpp",
    "Scene/Fan.h",
    "Scene/Ground.cpp",
    "Scene/Ground.h",
    "Scene/Renderer.cpp",
    "Scene/Scene3D.cpp",
    "Scene/Scene3D.h",
    "Scene/SceneHUD.cpp",
    "Scene/SceneHUD.h",
    "Scene/Transform.cpp",
    "Scene/Transform.h",
    "android/app/src/main/assets/shader/FanVertex.glsl",
    "android/app/src/main/assets/shader/FanFragment.glsl",
    "android/app/src/main/assets/shader/GroundVertex.glsl",
    "android/app/src/main/assets/shader/GroundFragment.glsl",
    "android/app/src/main/assets/shader/HUDVertex.glsl",
    "android/app/src/main/assets/shader/HUDFragment.glsl",
    "tests/Quiz3Tests.cpp",
    "screenshots/desktop-highlighted.png",
    "screenshots/android-quiz3.png",
    "screenshots/web-quiz3.png",
]
check(
    "Required submission files",
    all((ROOT / path).is_file() for path in required_files),
    "shared scenes, six shaders, tests, and three platform screenshots exist",
)

documented_source_files = [
    "Scene/Fan.cpp",
    "Scene/Fan.h",
    "Scene/Ground.cpp",
    "Scene/Ground.h",
    "Scene/Model.h",
    "Scene/Platform.h",
    "Scene/Renderer.cpp",
    "Scene/Renderer.h",
    "Scene/Scene3D.cpp",
    "Scene/Scene3D.h",
    "Scene/SceneHUD.cpp",
    "Scene/SceneHUD.h",
    "Scene/ShaderHelper.h",
    "Scene/main.cpp",
    "tests/Quiz3Tests.cpp",
    "android/app/src/main/java/com/example/fan3d/MainActivity.java",
    "android/app/src/main/assets/shader/FanVertex.glsl",
    "android/app/src/main/assets/shader/FanFragment.glsl",
    "android/app/src/main/assets/shader/GroundVertex.glsl",
    "android/app/src/main/assets/shader/GroundFragment.glsl",
    "android/app/src/main/assets/shader/HUDVertex.glsl",
    "android/app/src/main/assets/shader/HUDFragment.glsl",
    "web_shell.html",
]
check(
    "Professional source documentation",
    all(
        "@author Zoey <t.zoey@digipen.edu>" in read(path)
        for path in documented_source_files
    ),
    "every student-owned source file has a Zoey Doxygen ownership block",
)

check(
    "Supplied Transform files unchanged",
    sha256("Scene/Transform.h")
    == "0A64FD5386D92AF172365ABC7557ABCCF34A70033F6BDCEE72B0D01045556734"
    and sha256("Scene/Transform.cpp")
    == "9E19E25C0094BDA66D12FC4345E09D602AD7F37088803DC10082F21A9A3D5800",
    "SHA-256 hashes match the instructor-provided files",
)

renderer = read("Scene/Renderer.cpp")
scene3d = read("Scene/Scene3D.cpp")
scenehud = read("Scene/SceneHUD.cpp")
fan_cpp = read("Scene/Fan.cpp")
fan_h = read("Scene/Fan.h")
ground_cpp = read("Scene/Ground.cpp")
fan_vertex = read("android/app/src/main/assets/shader/FanVertex.glsl")
fan_fragment = read("android/app/src/main/assets/shader/FanFragment.glsl")
ground_vertex = read("android/app/src/main/assets/shader/GroundVertex.glsl")
ground_fragment = read("android/app/src/main/assets/shader/GroundFragment.glsl")
tests = read("tests/Quiz3Tests.cpp")

check(
    "Separate Scene3D and SceneHUD",
    "new Scene3D" in renderer
    and "new SceneHUD" in renderer
    and "class Scene3D final" in read("Scene/Scene3D.h")
    and "class SceneHUD final" in read("Scene/SceneHUD.h"),
    "Renderer owns explicit perspective-space and screen-space models",
)

pipeline_tokens = [
    "glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)",
    "glEnable(GL_DEPTH_TEST)",
    "scene3D->Render()",
    "glDisable(GL_DEPTH_TEST)",
    "sceneHUD->Render()",
]
pipeline_positions = [renderer.find(token) for token in pipeline_tokens]
check(
    "Required mixed-scene render order",
    all(position >= 0 for position in pipeline_positions)
    and pipeline_positions == sorted(pipeline_positions)
    and renderer.find("glEnable(GL_DEPTH_TEST)", pipeline_positions[-1])
    > pipeline_positions[-1],
    "clear -> 3D with depth -> HUD without depth -> restore depth",
)

phong_tokens = ("ambient", "diffuse", "specular", "LIGHTPOSITION", "CAMERAPOSITION")
check(
    "Phong fan and ground",
    all(token in fan_fragment for token in phong_tokens)
    and all(token in ground_fragment for token in phong_tokens)
    and "NORMALMATRIX" in fan_vertex
    and "NORMALMATRIX" in ground_vertex
    and "inverseTranspose" in fan_cpp
    and "inverseTranspose" in ground_cpp,
    "both materials use ambient/diffuse/specular lighting and transformed normals",
)
check(
    "Procedural checkerboard",
    "mod(cell.x + cell.y, 2.0)" in ground_fragment
    and "Texture" not in ground_fragment
    and "sampler" not in ground_fragment,
    "floor colour is generated from world coordinates without a texture",
)

check(
    "Screen-space HUD",
    "TransformOrtho(" in scenehud
    and all(label in scenehud for label in ('"+SPD"', '"-SPD"', '"+ZOOM"', '"-ZOOM"'))
    and "constexpr float blockWidth = 10.0f" in scenehud
    and "constexpr float blockHeight = 20.0f" in scenehud
    and "for (int block = 0; block < 20; ++block)" in scenehud,
    "orthographic four-button HUD contains twenty 10x20 indicator blocks",
)
check(
    "Clamped HUD actions",
    "std::min(speed + 1, 20)" in scenehud
    and "std::max(speed - 1, 0)" in scenehud
    and "--pendingZoomSteps" in scenehud
    and "++pendingZoomSteps" in scenehud
    and "std::clamp(distance, 4.0f, 18.0f)" in scene3d
    and "float cameraDistance" not in read("Scene/SceneHUD.h"),
    "speed is 0-20 and Scene3D exclusively owns the 4-18 camera distance",
)

hud_first = renderer.find("sceneHUD->TouchDown(x, y)")
pick_second = renderer.find("scene3D->PickAt(x, y)")
check(
    "HUD-first input routing",
    hud_first >= 0
    and pick_second > hud_first
    and "return;" in renderer[hud_first:pick_second],
    "a HUD hit consumes input before 3D picking",
)
check(
    "High-DPI Desktop/Web input",
    "cursorToFramebuffer" in read("Scene/main.cpp")
    and "glfwGetWindowSize" in read("Scene/main.cpp")
    and "glfwGetFramebufferSize" in read("Scene/main.cpp"),
    "logical pointer coordinates are normalized to framebuffer pixels",
)
check(
    "World-space nearest-part ray picking",
    "TransformUnproject(" in scene3d
    and "farPoint - nearPoint" in scene3d
    and "PickRay(" in scene3d
    and "glm::inverse(worldMatrix)" in fan_cpp
    and "nearestDistance" in fan_cpp
    and "kPartCount = 7" in fan_h,
    "unprojected ray tests all seven oriented part boxes and retains the nearest hit",
)
check(
    "Visible picking selection",
    "kHighlightColor" in fan_cpp
    and "part == selectedPart ? kHighlightColor : color" in fan_cpp
    and "(hit == Fan::NoPart || hit == fan->GetSelectedPart())" in scene3d,
    "picked part highlights; same-part and empty-space clicks clear selection",
)

retired_quiz2_tokens = (
    "dragBoost",
    "kTapThreshold",
    "kBoostScale",
    "fanOn",
    "CycleBladeCount",
)
check(
    "Quiz 2 drag control removed",
    not any(token in fan_cpp + fan_h for token in retired_quiz2_tokens),
    "the fan no longer owns tap/drag speed state",
)

web_shell = read("web_shell.html")
check(
    "Web page describes Quiz 3 HUD controls",
    "Programming Quiz 3 - Fan Dashboard HUD" in web_shell
    and "Use the HUD buttons to adjust speed and zoom." in web_shell
    and "drag to boost speed" not in web_shell
    and "long-press" not in web_shell,
    "the Web wrapper names Quiz 3 and documents only its current controls",
)

check(
    "Documented bonus - press-and-hold repeat",
    "held.count() < 350" in scenehud
    and "sinceRepeat" not in scenehud
    and "held button repeats" in tests
    and "every rendered frame" in read("README.md"),
    "held HUD buttons become level-triggered every frame after 350 ms and are tested",
)
check(
    "Documented bonus - colour-coded pick status",
    "kPickColors" in scenehud
    and "kPickLabels" in scenehud
    and "SetPickedPart" in scenehud
    and "second pick-feedback detail" in read("README.md"),
    "HUD reports the selected part with a tested material-colour status strip",
)

desktop_cmake = read("CMakeLists.txt")
android_cmake = read("android/app/src/main/cpp/CMakeLists.txt")
check(
    "Shared cross-platform sources",
    all(
        filename in desktop_cmake and filename in android_cmake
        for filename in ("Scene3D.cpp", "SceneHUD.cpp", "Ground.cpp", "Fan.cpp")
    )
    and "PLATFORM_ANDROID" in read("Scene/Platform.h")
    and "PLATFORM_EMSCRIPTEN" in read("Scene/Platform.h"),
    "Desktop, Android, and WebGL compile the same scene implementation",
)

desktop_workflow = (REPOSITORY / ".github/workflows/desktop.yml").read_text(encoding="utf-8")
android_workflow = (REPOSITORY / ".github/workflows/android.yml").read_text(encoding="utf-8")
pages_workflow = (REPOSITORY / ".github/workflows/pages.yml").read_text(encoding="utf-8")
check(
    "CI covers Programming Quizzes 1-3",
    all(
        project in desktop_workflow and project in android_workflow
        for project in (
            "AndroidProgrammingQuiz-CrossPlatformCI/AndroidProgrammingQuiz-CrossPlatformCI",
            "Assignment-2-3DFan",
            "android-programming-quiz3",
        )
    )
    and "ubuntu-latest" in desktop_workflow
    and "windows-latest" in desktop_workflow
    and "android-programming-quiz3" in pages_workflow,
    "all three quizzes build for Linux, Windows, and Android; Quiz 3 also builds for Web",
)


def png_dimensions(path: Path) -> tuple[int, int] | None:
    try:
        data = path.read_bytes()[:24]
        if data[:8] != b"\x89PNG\r\n\x1a\n":
            return None
        return struct.unpack(">II", data[16:24])
    except OSError:
        return None


screenshot_paths = [ROOT / path for path in required_files if path.startswith("screenshots/")]
screenshot_dimensions = [png_dimensions(path) for path in screenshot_paths]
screenshot_hashes = [
    hashlib.sha256(path.read_bytes()).digest() for path in screenshot_paths if path.is_file()
]
check(
    "Submission screenshot quality",
    len(screenshot_dimensions) == 3
    and all(
        dimensions is not None and dimensions[0] >= 800 and dimensions[1] >= 600
        for dimensions in screenshot_dimensions
    )
    and len(set(screenshot_hashes)) == 3,
    "three distinct PNG captures are at least 800x600",
)

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
    tracked = [
        Path(line)
        for line in subprocess.run(
            ["git", "ls-files", "--", project_pathspec],
            cwd=ROOT,
            check=True,
            capture_output=True,
            text=True,
        ).stdout.splitlines()
        if line
    ]
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
    "gradle-project-cache",
    "tmp",
    "sample-demo",
}
leaked = [
    str(path)
    for path in tracked
    if path.suffix.lower() in forbidden_suffixes or forbidden_parts.intersection(path.parts)
]
check(
    "Repository hygiene",
    not leaked,
    "no tracked inputs, samples, build products, IDE caches, APKs, or WebAssembly"
    + (f"; leaked: {', '.join(leaked)}" if leaked else ""),
)

if failures:
    print(
        f"\n{len(failures)} verification check(s) failed: {', '.join(failures)}",
        file=sys.stderr,
    )
    raise SystemExit(1)

print("\nAll Programming Quiz 3 static rubric checks passed.")
