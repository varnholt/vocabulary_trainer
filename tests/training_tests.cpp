#include "training.h"

#include <cstdlib>
#include <iostream>

void check(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

int main()
{
    for (const auto *invalid :
         {"", "\n\n", "hello", "hello;", ";world", "a;b\nbad", "a;\"unclosed", "a;\"b\"extra"})
        check(!vocab::parse(invalid).error.empty(), invalid);
    const auto csv =
        vocab::parse("\xef\xbb\xbf\n en læge ; a doctor \r\nen bil;bilen;biler;bilerne;a car\n");
    check(csv.error.empty() && csv.cards.size() == 2, "BOM, CRLF and blank lines");
    check(csv.cards[0].front == "en læge" && csv.cards[0].back == "a doctor",
          "Unicode and trimming");
    check(csv.cards[1].front == "en bil\nbilen\nbiler\nbilerne", "Legacy multi-column cards");
    const auto quoted = vocab::parse("\"hello, friend\",\"say \"\"hi\"\"\"\n\"two\nlines\",answer");
    check(quoted.error.empty() && quoted.cards[0].front == "hello, friend" &&
              quoted.cards[0].back == "say \"hi\"",
          "Quoted delimiters and escaped quotes");
    check(quoted.cards[1].front == "two\nlines", "Multiline fields");
    check(vocab::parse("a|b").cards.size() == 1, "Pipe separator");
    check(vocab::parse("a;;;b").cards[0].front == "a", "Optional empty inflections");
    check(!vocab::parse(std::string(5 * 1024 * 1024 + 1, 'a')).error.empty(), "Size limit");
    check(!vocab::parse(std::string("a;\0b", 5)).error.empty(), "Binary input");
    vocab::Session session(42);
    check(!session.current() && !session.answer(true), "Empty session safe");
    session.start(csv.cards, true);
    check(session.total() == 2 && session.remaining() == 2, "Fresh session");
    const auto first = session.current()->front;
    session.answer(false);
    check(session.current()->front != first, "Avoid repeated card when alternatives exist");
    for (int i = 0; session.current() && i < 20; ++i)
        session.answer(true);
    check(session.remaining() == 0 && !session.current(), "Session completes");
    session.start({{"front", "back", 100}}, true);
    check(session.current()->front == "back" && session.current()->score == 0 &&
              session.attempts() == 0,
          "Reset and reverse");
    session.answer(false);
    check(session.answer(true), "Failed word needs a positive score");
    check(!session.answer(true) && session.attempts() == 3, "One-card session completes");
    std::cout << "All training regression checks passed\n";
}
