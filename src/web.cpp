#include "training.h"
#include <cstdlib>
#include <emscripten/emscripten.h>
#include <string>

namespace
{
vocab::Session session;
std::string error;
} // namespace

extern "C"
{
    EMSCRIPTEN_KEEPALIVE void *allocate_input(int size)
    {
        return size > 0 && size <= 5 * 1024 * 1024 + 1 ? std::malloc(size) : nullptr;
    }
    EMSCRIPTEN_KEEPALIVE void release_input(void *input) { std::free(input); }
    EMSCRIPTEN_KEEPALIVE int load_list(const char *csv, int reverse)
    {
        auto result = vocab::parse(csv);
        error = result.error;
        if (!error.empty())
            return 0;
        session.start(std::move(result.cards), reverse != 0);
        return 1;
    }
    EMSCRIPTEN_KEEPALIVE const char *last_error() { return error.c_str(); }
    EMSCRIPTEN_KEEPALIVE const char *card_front()
    {
        const auto *card = session.current();
        return card ? card->front.c_str() : "";
    }
    EMSCRIPTEN_KEEPALIVE const char *card_back()
    {
        const auto *card = session.current();
        return card ? card->back.c_str() : "";
    }
    EMSCRIPTEN_KEEPALIVE int answer_card(int known) { return session.answer(known != 0) ? 1 : 0; }
    EMSCRIPTEN_KEEPALIVE int remaining_cards() { return static_cast<int>(session.remaining()); }
    EMSCRIPTEN_KEEPALIVE int total_cards() { return static_cast<int>(session.total()); }
    EMSCRIPTEN_KEEPALIVE int answer_count() { return static_cast<int>(session.attempts()); }
}
