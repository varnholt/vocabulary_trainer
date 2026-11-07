# Architecture

The maintained application has three small layers:

```text
CSV text → C++ parser and session engine → WebAssembly C API
                                             ↓
                              HTML controls and browser storage
                                             ↓
                               Browser or offline Android host
```

`training.cpp` owns parsing, random selection, scoring, reversal, and completion. It has no UI or platform dependency and is tested natively on three operating systems. `web.cpp` exposes a narrow C API. JavaScript handles file selection, UTF-8 decoding, browser storage, and rendering text through `textContent` rather than HTML interpolation.

The UI has no package-manager dependency or frontend build chain. All resources are served locally, including the system font stack. The compiled output can be hosted on any static HTTP(S) server. It uses neither threads nor cross-origin isolation.

## ImGui assessment

Dear ImGui is technically suitable for a shared C++ desktop/browser port. Its [SDL3/OpenGL example](https://github.com/ocornut/imgui/tree/master/examples/example_sdl3_opengl3) includes an Emscripten path. A shared immediate-mode UI would be attractive if maintaining a native desktop renderer were the main goal.

For this trainer, the browser is the primary destination. Native HTML controls provide keyboard focus, selectable vocabulary, screen-reader semantics, flexible Unicode font fallback, responsive layouts, and mobile file selection directly. An ImGui canvas would require additional work around these browser capabilities. Adding SDL, a renderer, font assets, and a JavaScript bridge would introduce dependencies without simplifying this small interface.

The chosen approach keeps the training logic in C++/WebAssembly and uses a browser-native interface. A future ImGui desktop frontend can link `vocabulary_core` without replacing the parser or scoring logic.

## Storage and privacy

Only custom list text and the reverse preference are persisted, in one versioned localStorage entry. The current session stays in memory. Imports are validated before being added; quota or storage-access failures are visible, and users can continue with an in-memory list. Original CSV files are the backup; there is no cloud sync or export database.

Samples are shipped as static UTF-8 files. Browser imports use a strict UTF-8 decoder and reject oversized files before reading. The C++ parser also enforces its own 5 MiB limit. There is no upload endpoint, telemetry SDK, or third-party web resource.

## Android

The APK packages the tested web output through Android's asset system. [WebViewAssetLoader](https://developer.android.com/develop/ui/views/layout/webapps/load-local-content) serves it from an app-local HTTPS origin, including the correct WebAssembly MIME type. No network permission or JavaScript-to-Java bridge is needed. External links open in a separate browser. The document picker grants access only to a file chosen by the user, and AndroidX handles system back navigation.

The package is independent of native CPU architecture because the C++ core runs in the installed System WebView's WebAssembly engine. Android WebView must support WebAssembly; keeping it updated is required. Clearing app data removes localStorage. Backups and device transfer are disabled for app-local vocabulary.

CI produces debug-signed APKs for sideloading. Stable release signing and Play Store distribution are separate future work. No signing secrets are stored in the repository.

## Original desktop app

The Qt app is preserved in `legacy/qt/` and the original `v1.0` release remains available. It is not built by the new CMake project or CI. Its working-directory dependencies and input limitations remain historical behavior, not requirements of the new application.
