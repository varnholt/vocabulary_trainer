#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace vocab
{
struct Card
{
    std::string front;
    std::string back;
    int score = 0;
};

struct ParseResult
{
    std::vector<Card> cards;
    std::string error;
};

// Two or more fields: all fields except the last form the front of the card.
// Quoted separators, doubled quotes, CRLF and a UTF-8 BOM are accepted.
ParseResult parse(std::string_view csv);

class Session
{
  public:
    explicit Session(std::uint32_t seed = std::random_device{}());
    void start(std::vector<Card> cards, bool reverse = false);
    const Card *current() const;
    bool answer(bool known);
    std::size_t remaining() const;
    std::size_t total() const { return cards_.size(); }
    std::size_t attempts() const { return attempts_; }

  private:
    std::vector<Card> cards_;
    std::size_t current_ = 0;
    std::size_t attempts_ = 0;
    std::mt19937 random_;
};
} // namespace vocab
