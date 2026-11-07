"""Exercise the real WebAssembly app, served from a temporary HTTP server."""
import functools
import http.server
import os
import pathlib
import sys
import threading

from playwright.sync_api import sync_playwright, expect


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *args):
        pass


root = pathlib.Path(sys.argv[1]).resolve()
server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), functools.partial(QuietHandler, directory=str(root)))
threading.Thread(target=server.serve_forever, daemon=True).start()
try:
    with sync_playwright() as p:
        browser = p.chromium.launch(executable_path=os.environ.get("CHROMIUM_PATH"))
        page = browser.new_page(viewport={"width": 1440, "height": 1050})
        errors = []
        page.on("pageerror", lambda error: errors.append(str(error)))
        url = f"http://127.0.0.1:{server.server_port}/"
        page.goto(url)
        page.wait_for_selector('html[data-ready="true"]')
        expect(page.locator(".list-start")).to_have_count(7)
        screenshots = pathlib.Path("tests/screenshots")
        screenshots.mkdir(parents=True, exist_ok=True)
        page.screenshot(path=str(screenshots / "library.png"), full_page=True)
        # Every bundled list must load through the actual Wasm parser.
        for index in range(7):
            page.locator(".list-start").nth(index).click()
            try:
                expect(page.locator("#practice")).to_be_visible()
            except AssertionError:
                raise AssertionError(f"Sample {index}: {page.locator('#status').inner_text()}; browser errors: {errors}")
            expect(page.locator("#known")).to_be_disabled()
            page.keyboard.press("Space")
            expect(page.locator("#known")).to_be_enabled()
            if index == 0:
                page.screenshot(path=str(screenshots / "practice.png"), full_page=True)
            page.keyboard.press("Escape")
        # Upload, UTF-8, reverse, repeated practice, completion and persistence.
        page.locator("#file").set_input_files({"name": "Danish.csv", "mimeType": "text/csv", "buffer": "læge;doctor\n".encode()})
        expect(page.locator(".list-start")).to_have_count(8)
        page.locator("#reverse").check()
        page.locator(".list-start").last.click()
        expect(page.locator("#card-text")).to_have_text("doctor")
        page.keyboard.press("ArrowRight")
        expect(page.locator("#attempts")).to_have_text("0 answers")
        page.keyboard.press("Space")
        expect(page.locator("#card-text")).to_have_text("læge")
        page.keyboard.press("ArrowLeft")
        for _ in range(2):
            page.keyboard.press("Space")
            page.keyboard.press("ArrowRight")
        expect(page.locator("#complete")).to_be_visible()
        expect(page.locator("#summary")).to_contain_text("1 word in 3 answers")
        page.reload()
        page.wait_for_selector('html[data-ready="true"]')
        expect(page.locator(".list-start")).to_have_count(8)
        expect(page.locator("#reverse")).to_be_checked()
        # Invalid and malicious-looking text cannot become HTML.
        page.locator("#file").set_input_files({"name": "bad.csv", "mimeType": "text/csv", "buffer": b"word;"})
        expect(page.locator("#status")).to_contain_text("Invalid CSV")
        page.locator("summary").click()
        page.locator("#list-name").fill("<img src=x onerror=alert(1)>")
        page.locator("#csv").fill('"<b>hello</b>";world')
        page.locator("#add-paste").click()
        expect(page.locator(".list-start")).to_have_count(9)
        expect(page.locator("#lists img")).to_have_count(0)
        page.locator(".list-start").last.click()
        page.locator("#card").click()
        expect(page.locator("#card-text")).to_have_text("<b>hello</b>")
        expect(page.locator("#card-text b")).to_have_count(0)
        page.locator("#back").click()
        page.on("dialog", lambda dialog: dialog.accept())
        page.locator(".delete").last.click()
        expect(page.locator(".list-start")).to_have_count(8)
        page.locator("#list-name").fill("")
        page.locator("#csv").fill("")
        page.locator("summary").click()
        # Narrow viewport and touch-sized controls without horizontal overflow.
        page.set_viewport_size({"width": 390, "height": 844})
        page.screenshot(path=str(screenshots / "mobile.png"), full_page=True)
        assert page.evaluate("document.documentElement.scrollWidth <= innerWidth")
        page.locator(".list-start").first.click()
        page.locator("#card").click()
        page.locator("#known").click()
        assert page.evaluate("document.documentElement.scrollWidth <= innerWidth")
        assert not errors, errors
        browser.close()
        print("Browser workflows passed: samples, imports, reverse, scoring, storage, keyboard, safe text and mobile layout")
finally:
    server.shutdown()
