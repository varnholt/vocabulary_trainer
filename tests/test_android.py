"""Smoke-test the bundled WebAssembly app in a running Android emulator via CDP."""
import os
import pathlib
import subprocess
import sys
import time

from playwright.sync_api import sync_playwright, expect

sdk = os.environ.get("ANDROID_HOME", os.environ.get("ANDROID_SDK_ROOT", ""))
adb = str(pathlib.Path(sdk) / "platform-tools" / ("adb.exe" if os.name == "nt" else "adb")) if sdk else "adb"


def command(*args):
    return subprocess.check_output([adb, *args], text=True).strip()


package = "com.varnholt.vocabulary"
command("install", "-r", sys.argv[1])
command("shell", "pm", "clear", package)
command("shell", "am", "start", "-n", f"{package}/.MainActivity")
try:
    for _ in range(60):
        pid = subprocess.run([adb, "shell", "pidof", package], text=True, capture_output=True).stdout.strip()
        sockets = command("shell", "cat", "/proc/net/unix")
        if f"webview_devtools_remote_{pid}" in sockets:
            break
        time.sleep(0.5)
    else:
        raise AssertionError("The app did not start its debug WebView")
    command("forward", "tcp:9223", f"localabstract:webview_devtools_remote_{pid}")
    with sync_playwright() as playwright:
        browser = playwright.chromium.connect_over_cdp("http://127.0.0.1:9223")
        page = browser.contexts[0].pages[0]
        page.wait_for_selector('html[data-ready="true"]', timeout=30000)
        expect(page.locator(".list-start")).to_have_count(7)
        page.locator(".list-start").first.click()
        expect(page.locator("#practice")).to_be_visible()
        page.locator("#card").click()
        expect(page.locator("#known")).to_be_enabled()
        page.locator("#known").click()
        expect(page.locator("#attempts")).to_have_text("1 answer")
        command("shell", "input", "keyevent", "4")
        expect(page.locator("#library")).to_be_visible()
        page.locator("summary").click()
        page.locator("#list-name").fill("Android check")
        page.locator("#csv").fill("hej;hello")
        page.locator("#add-paste").click()
        page.locator(".list-start").last.click()
        expect(page.locator("#card-text")).to_have_text("hej")
        page.locator("#card").click()
        expect(page.locator("#card-text")).to_have_text("hello")
        page.locator("#known").click()
        expect(page.locator("#complete")).to_be_visible()
        page.reload()
        page.wait_for_selector('html[data-ready="true"]')
        expect(page.locator(".list-start")).to_have_count(8)
        page.get_by_role("link", name="All the Vocabulary home").click()
        page.wait_for_selector('html[data-ready="true"]')
        expect(page.locator(".list-start")).to_have_count(8)
        assert page.evaluate("document.documentElement.scrollWidth <= innerWidth")
        screenshots = pathlib.Path("tests/screenshots")
        screenshots.mkdir(parents=True, exist_ok=True)
        page.screenshot(path=str(screenshots / "android.png"), full_page=True)
        # Restore a clean shelf so the check is repeatable on the same emulator.
        page.evaluate("localStorage.clear()")
        print("Android WebView passed: bundled Wasm, samples, practice, back navigation, import and storage")
finally:
    subprocess.run([adb, "forward", "--remove", "tcp:9223"], capture_output=True)
    command("shell", "am", "force-stop", package)
