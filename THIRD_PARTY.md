# Third-party components

The CC0 dedication applies to this project's original code and vocabulary lists. It does not replace upstream licenses.

| Component                                                                                                                                     | Use                                               | License                                                |
| --------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------- | ------------------------------------------------------ |
| [AndroidX WebKit](https://android.googlesource.com/platform/frameworks/support/+/androidx-main/webkit/)                                       | App-local asset loading                           | Apache-2.0                                             |
| [AndroidX Activity](https://android.googlesource.com/platform/frameworks/support/+/androidx-main/activity/) and transitive AndroidX libraries | Android lifecycle and back navigation             | Apache-2.0                                             |
| [Gradle wrapper](https://github.com/gradle/gradle)                                                                                            | Checked-in Android build launcher and wrapper JAR | Apache-2.0                                             |
| [Emscripten](https://github.com/emscripten-core/emscripten/blob/main/LICENSE) and LLVM/libc++ runtime portions                                | WebAssembly toolchain/runtime                     | MIT and Apache-2.0 with LLVM exceptions, as applicable |
| [Qt](https://www.qt.io/licensing/)                                                                                                            | Historical desktop app only                       | Qt's applicable commercial or open-source terms        |

License texts are included in [`licenses/`](licenses/) and copied into the web output and APK assets. Gradle and Android dependency source information is available from the linked upstream projects and their published Maven artifacts. The Gradle distribution is downloaded at build time rather than vendored. The browser interface uses system fonts and no third-party scripts.
