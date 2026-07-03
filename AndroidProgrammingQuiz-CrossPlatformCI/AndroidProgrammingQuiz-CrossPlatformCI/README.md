# Android Programming Quiz Cross-Platform CI

Refactored OpenGL ES quiz project with a shared Model/Renderer architecture for Desktop, Android, and Web builds. The scene renders a rotating RGB triangle and an independent colored square using shader source loaded from `android/app/src/main/assets/shader`.

## Desktop

From the project root:

```bat
script_build_and_run.bat
```

This configures and builds the `GLPIFrameworkIntro` executable through CMake, copies shader assets beside the executable, and runs the desktop GLFW build.

## Android

Open the `android/` folder in Android Studio, then run:

1. Sync Gradle.
2. Clean Project.
3. Build and run on an emulator or Android device.

`MainActivity` passes the Java `AssetManager` into native code so the shared renderer can load GLSL shader assets from the APK.

## Web

Activate Emscripten first, then from the project root run one of:

```bat
build_web.bat
```

```sh
./build_web.sh
```

Serve the generated output:

```sh
cd web
python -m http.server 8080
```

Open `http://localhost:8080/`. The build preloads `android/app/src/main/assets/shader` into the Emscripten virtual filesystem at `assets/shader`.

## Project Notes

- Shared rendering code lives in `Scene/`.
- `Renderer` owns a collection of `Model` instances and is the only object called by platform entry points.
- `Triangle` and `Square` each own their own shader program, VAO, and VBOs.
- Generated folders such as `build_desktop/`, `web/`, `.gradle/`, `.idea/`, `android/app/build/`, and `android/app/.cxx/` are build output and should not be included in a submission.
