"""Package tested build artifacts, rejecting tags that disagree with app versions."""
import hashlib
import pathlib
import re
import shutil
import sys
import zipfile

tag = sys.argv[1]
version = re.search(r"project\(\s*vocabulary_trainer\s+VERSION\s+([0-9.]+)", pathlib.Path("CMakeLists.txt").read_text()).group(1)
android = re.search(r"versionName '([^']+)'", pathlib.Path("android/app/build.gradle").read_text()).group(1)
if tag != f"v{version}" or android != version:
    raise SystemExit("Release tag, CMake version and Android versionName must match")
output = pathlib.Path("artifacts/release")
output.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(output / f"vocabulary-{tag}-web.zip", "w", zipfile.ZIP_DEFLATED) as archive:
    for path in pathlib.Path("artifacts/web").rglob("*"):
        if path.is_file():
            archive.write(path, path.relative_to("artifacts/web"))
    archive.write("README.md")
apk = list(pathlib.Path("artifacts/android").glob("*.apk"))
if len(apk) != 1:
    raise SystemExit("Expected exactly one tested Android APK")
shutil.copyfile(apk[0], output / f"vocabulary-{tag}-Android-debug.apk")
files = sorted(output.glob(f"vocabulary-{tag}-*"))
(output / "SHA256SUMS.txt").write_text("".join(
    f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n" for path in files), encoding="utf-8")
