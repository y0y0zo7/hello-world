# OpenGL ES 3.0 coursework

Cross-platform C++ graphics exercises for CSD2101 Android Programming. GitHub Actions continuously builds the projects for Android, Windows, Linux, and WebGL, then deploys the browser versions to GitHub Pages.

## Projects

| Project | Browser demo |
| --- | --- |
| Chapter 2 - OpenGL Primitives | [Launch](https://y0y0zo7.github.io/hello-world/Chapter2/2-OpenGLES3_Primitives-Modern/) |
| Assignment 1 - Android Programming Quiz | [Launch](https://y0y0zo7.github.io/hello-world/AndroidProgrammingQuiz-CrossPlatformCI/) |
| Assignment 2 - Interactive 3D Fan | [Launch](https://y0y0zo7.github.io/hello-world/android-programming-2/) |

Assignment 2 source, controls, architecture, verification, and exact build instructions are documented in [`android-programming-2/README.md`](android-programming-2/README.md).

## Automation

- `Android` builds every APK; Assignment 2 also runs Android Lint with warnings treated as errors.
- `Desktop` compiles every project on Ubuntu and Windows and runs the Assignment 2 interaction test on both.
- `GitHub Pages` compiles all WebGL versions and deploys the checked-in gallery page plus the generated WebAssembly bundles.

Generated builds, local SDK settings, course references, assignment PDFs, and sample executables are intentionally excluded from version control.
