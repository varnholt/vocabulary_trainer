Practice vocabulary in the browser or in the offline Android app. This version adds a C++/WebAssembly engine, a responsive interface, CSV import and paste, local list storage, reversed cards, and session progress.

- **Web:** unzip the web package and serve it with any static HTTP(S) server, or use the [hosted trainer](https://varnholt.github.io/vocabulary_trainer/).
- **Android:** the debug APK requires Android 8.0+ and an updated Android System WebView. It bundles the app and samples offline. This is a sideloadable preview, not a Play Store release; CI debug keys can change between builds, so updating may require uninstalling the previous build and losing its saved lists. Keep original CSV files.
- **Checksums:** verify downloaded packages with `SHA256SUMS.txt`.

Core regression checks run on Windows, Linux and macOS. Browser workflows run against the compiled WebAssembly app in Chromium. The Android package is built, linted, and smoke-tested in an emulator; device behavior still benefits from testing on your own phone.

The original Qt desktop implementation remains in `legacy/qt/`, and the old Windows release remains available under `v1.0`. See the README and changelog for details.
