# Contributing

Small improvements to vocabulary practice, CSV compatibility, accessibility, and reliability are welcome. For larger changes, open an issue describing the user need first.

Keep training behavior in `src/training.cpp`, independent of UI and platform APIs. Put file selection and browser storage in `web/`; keep Android-specific behavior in `android/`. Treat imported names and vocabulary as plain text.

Before submitting changes, run the native checks described in the README. For changes to the browser interface or WebAssembly bridge, also rebuild the web output and run `tests/test_web.py`. For Android changes, run `assembleDebug lintDebug`. Test narrow screens and keyboard access when changing controls.

Add regression coverage for parser or scoring changes. Report the checks you ran and any platform limitations in the pull request. Use invented words in bug reports rather than private vocabulary lists.

Contributions to project code and sample lists are provided under the repository's CC0 license. Third-party dependencies keep their original licenses.
