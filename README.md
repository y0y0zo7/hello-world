# OpenGL ES 3.0 coursework

Cross-platform C++ graphics projects for CSD2101 Android Programming. GitHub Actions builds Programming Quizzes 1, 2, and 3 for Android, Windows, and Linux, then builds and deploys the browser versions through GitHub Pages.

## Projects

| Project | Browser demo |
| --- | --- |
| Chapter 2 - OpenGL Primitives | [Launch](https://y0y0zo7.github.io/hello-world/Chapter2/2-OpenGLES3_Primitives-Modern/) |
| Assignment 1 - Android Programming Quiz | [Launch](https://y0y0zo7.github.io/hello-world/AndroidProgrammingQuiz-CrossPlatformCI/) |
| Assignment 2 - Interactive 3D Fan | [Launch](https://y0y0zo7.github.io/hello-world/Assignment-2-3DFan/) |
| Programming Quiz 3 - Phong Fan Dashboard | [Launch](https://y0y0zo7.github.io/hello-world/android-programming-quiz3/) |

Quiz 3 architecture, controls, bonus feature, screenshots, verification, and build instructions are documented in [`android-programming-quiz3/README.md`](android-programming-quiz3/README.md).

## Automation

- `Android` builds all three quiz APKs; Quizzes 2 and 3 also run Android Lint.
- `Desktop` compiles all three quizzes on Ubuntu and Windows and runs the interaction tests for Quizzes 2 and 3.
- `GitHub Pages` verifies the rubric invariants, compiles the WebGL versions, checks the deployable assets, and publishes the gallery.

Generated builds, local SDK settings, course references, assignment PDFs, and sample executables are intentionally excluded from version control.

## Submission reminder

The repository owner must add `parminder.singh@digipen.edu` as a GitHub collaborator before submitting. This account-level action cannot be represented by source code or CI.
