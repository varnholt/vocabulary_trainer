"use strict";

const $ = (id) => document.getElementById(id);
const storageKey = "all-the-vocabulary-v2";
const samples = [
  ["lektion_1.csv", "Everyday Danish"],
  ["lektion_1_sporgeord.csv", "Question words"],
  ["lektion_1_poss_pronoun.csv", "Possessive pronouns"],
  ["lektion_3_substantiver.csv", "Nouns & their forms"],
  ["lektion_3_verber.csv", "Verbs & their forms"],
  ["lektion_3_centraladjektiver.csv", "Core adjectives"],
  ["lektion_3_adjektiver_adverbier.csv", "Adjectives & adverbs"],
];
let api;
let custom = [];
let active;
let revealed = false;
let view = "library";
let storageWarning = "";

function notify(message = "", error = false) {
  $("status").textContent = message;
  $("status").classList.toggle("error", error);
}

try {
  const saved = JSON.parse(localStorage.getItem(storageKey) || "{}");
  if (Array.isArray(saved.lists))
    custom = saved.lists.filter(
      (list) =>
        typeof list.name === "string" &&
        typeof list.csv === "string" &&
        list.name.length <= 80 &&
        new TextEncoder().encode(list.csv).length <= 5 * 1024 * 1024,
    );
  $("reverse").checked = saved.reverse === true;
} catch {
  storageWarning =
    "Browser storage is unavailable or unreadable. You can still practice; keep your original CSV files.";
}

function persist() {
  try {
    localStorage.setItem(
      storageKey,
      JSON.stringify({ lists: custom, reverse: $("reverse").checked }),
    );
    storageWarning = "";
    return true;
  } catch {
    storageWarning =
      "Could not save to browser storage. Your lists work until this tab closes; keep your original CSV files.";
    notify(storageWarning, true);
    return false;
  }
}

function show(next) {
  view = next;
  for (const name of ["library", "practice", "complete"])
    $(name).hidden = name !== next;
}

function renderLists() {
  $("lists").replaceChildren();
  const lists = [...samples.map(([file, name]) => ({ file, name })), ...custom];
  $("list-count").textContent = `${lists.length} lists`;
  for (const list of lists) {
    const row = document.createElement("div");
    row.className = "list-row";
    const button = document.createElement("button");
    button.className = "list-start";
    button.disabled = !api;
    const title = document.createElement("strong");
    title.textContent = list.name;
    const subtitle = document.createElement("span");
    subtitle.textContent = list.file
      ? "Danish → English · sample list"
      : "Your vocabulary · stored on this device";
    button.append(title, subtitle);
    button.addEventListener("click", () => start(list));
    row.append(button);
    if (!list.file) {
      const remove = document.createElement("button");
      remove.className = "delete";
      remove.textContent = "×";
      remove.setAttribute("aria-label", `Remove ${list.name}`);
      remove.addEventListener("click", () => {
        if (
          !window.confirm(
            `Remove “${list.name}” from this browser? Keep your original CSV file if you want to import it again.`,
          )
        )
          return;
        custom = custom.filter((item) => item !== list);
        persist();
        renderLists();
      });
      row.append(remove);
    }
    $("lists").append(row);
  }
}

async function start(list) {
  if (!api) return;
  try {
    const response = list.file ? await fetch(`samples/${list.file}`) : null;
    if (response && !response.ok)
      throw new Error("Could not load the sample. Please try again.");
    const csv = response ? await response.text() : list.csv;
    if (csv.includes("\0"))
      throw new Error("Use a UTF-8 text file, not a binary or UTF-16 file.");
    if (!api.load(csv, $("reverse").checked ? 1 : 0))
      throw new Error(api.error());
    active = list;
    $("session-title").textContent = list.name;
    notify();
    show("practice");
    renderCard();
    $("card").focus();
  } catch (error) {
    notify(error.message, true);
  }
}

function renderCard() {
  revealed = false;
  $("card-text").textContent = api.front();
  $("card-side").textContent = "TAKE YOUR TIME";
  $("card-hint").textContent = "Think of the answer. Tap to reveal.";
  $("again").disabled = $("known").disabled = true;
  const learned = api.total() - api.remaining();
  $("progress").max = api.total();
  $("progress").value = learned;
  $("progress-text").textContent = `${learned} of ${api.total()} words learned`;
  $("attempts").textContent =
    `${api.attempts()} ${api.attempts() === 1 ? "answer" : "answers"}`;
}

function flip() {
  if (view !== "practice") return;
  revealed = !revealed;
  $("card-text").textContent = revealed ? api.back() : api.front();
  $("card-side").textContent = revealed ? "THE OTHER SIDE" : "TAKE YOUR TIME";
  $("card-hint").textContent = revealed
    ? "How did you do? Tap to flip back."
    : "Think of the answer. Tap to reveal.";
  $("again").disabled = $("known").disabled = !revealed;
}

function answer(known) {
  if (view !== "practice" || !revealed) return;
  if (api.answer(known ? 1 : 0)) {
    renderCard();
    $("card").focus();
  } else {
    $("summary").textContent =
      `You practiced ${api.total()} ${api.total() === 1 ? "word" : "words"} in ${api.attempts()} ${api.attempts() === 1 ? "answer" : "answers"}. Come back whenever you want a refresher.`;
    show("complete");
    $("restart").focus();
  }
}

function addList(name, csv) {
  if (!api) {
    notify(
      "The practice engine is still loading. Try again in a moment.",
      true,
    );
    return;
  }
  if (new TextEncoder().encode(csv).length > 5 * 1024 * 1024) {
    notify("The list exceeds the 5 MiB limit.", true);
    return;
  }
  if (csv.includes("\0")) {
    notify("Use a UTF-8 text file, not a binary or UTF-16 file.", true);
    return;
  }
  if (!api.load(csv, 0)) {
    notify(api.error(), true);
    return;
  }
  const list = { name: (name.trim() || "My vocabulary").slice(0, 80), csv };
  custom.push(list);
  const saved = persist();
  renderLists();
  if (saved)
    notify(
      `Added “${list.name}” — ${api.total()} words. Select it to start practicing.`,
    );
}

$("reverse").addEventListener("change", persist);
$("file").addEventListener("change", async (event) => {
  const file = event.target.files[0];
  if (!file) return;
  try {
    if (file.size > 5 * 1024 * 1024)
      throw new Error("The list exceeds the 5 MiB limit.");
    const csv = new TextDecoder("utf-8", { fatal: true }).decode(
      await file.arrayBuffer(),
    );
    addList(file.name.replace(/\.csv$/i, ""), csv);
  } catch (error) {
    notify(`Could not read this list: ${error.message}`, true);
  }
  event.target.value = "";
});
$("add-paste").addEventListener("click", () =>
  addList($("list-name").value, $("csv").value),
);
$("card").addEventListener("click", flip);
$("again").addEventListener("click", () => answer(false));
$("known").addEventListener("click", () => answer(true));
function back() {
  show("library");
  renderLists();
  $("lists").querySelector("button")?.focus();
}
$("back").addEventListener("click", back);
$("finish").addEventListener("click", back);
$("restart").addEventListener("click", () => start(active));
document.addEventListener("keydown", (event) => {
  if (
    view !== "practice" ||
    event.repeat ||
    event.altKey ||
    event.ctrlKey ||
    event.metaKey
  )
    return;
  if (event.key === "Escape") {
    event.preventDefault();
    back();
  } else if (event.code === "Space") {
    event.preventDefault();
    flip();
  } else if (event.key === "ArrowLeft") {
    event.preventDefault();
    answer(false);
  } else if (event.key === "ArrowRight") {
    event.preventDefault();
    answer(true);
  }
});

window.Module = {
  onRuntimeInitialized() {
    const wrap = (name, result, args = []) => Module.cwrap(name, result, args);
    const allocate = wrap("allocate_input", "number", ["number"]);
    const release = wrap("release_input", null, ["number"]);
    const load = wrap("load_list", "number", ["number", "number"]);
    api = {
      load(csv, reverse) {
        // cwrap string arguments use the small Wasm stack. Import on the heap.
        const size = new TextEncoder().encode(csv).length + 1;
        const pointer = allocate(size);
        if (!pointer) throw new Error("This list is too large to load.");
        try {
          Module.stringToUTF8(csv, pointer, size);
          return load(pointer, reverse);
        } finally {
          release(pointer);
        }
      },
      error: wrap("last_error", "string"),
      front: wrap("card_front", "string"),
      back: wrap("card_back", "string"),
      answer: wrap("answer_card", "number", ["number"]),
      remaining: wrap("remaining_cards", "number"),
      total: wrap("total_cards", "number"),
      attempts: wrap("answer_count", "number"),
    };
    $("add-paste").disabled = false;
    renderLists();
    notify(storageWarning, Boolean(storageWarning));
    document.documentElement.dataset.ready = "true";
  },
  onAbort() {
    notify(
      "The practice engine could not start. Reload in a browser with WebAssembly support.",
      true,
    );
  },
};
renderLists();
const engine = document.createElement("script");
engine.src = "vocabulary.js";
engine.onerror = () =>
  notify(
    "Could not load the practice engine. Check your connection and reload.",
    true,
  );
document.head.append(engine);
