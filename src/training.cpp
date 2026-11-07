#include "training.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace vocab
{
namespace
{
std::string trim(std::string_view value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos)
        return {};
    return std::string(value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1));
}
} // namespace

ParseResult parse(std::string_view csv)
{
    ParseResult result;
    if (csv.size() > 5 * 1024 * 1024)
        return {{}, "The list exceeds the 5 MiB limit."};
    if (csv.find('\0') != std::string_view::npos)
        return {{}, "Use a UTF-8 text file, not a binary or UTF-16 file."};
    if (csv.substr(0, 3) == "\xef\xbb\xbf")
        csv.remove_prefix(3);
    char delimiter = 0;
    bool quoted = false;
    // Detect on the first nonblank record, ignoring separators in quotes.
    for (std::size_t i = 0; i < csv.size(); ++i)
    {
        const char c = csv[i];
        if (c == '"')
            quoted = !quoted;
        if (!quoted && (c == ';' || c == ',' || c == '|'))
        {
            delimiter = c;
            break;
        }
        if (!quoted && c == '\n' && !trim(csv.substr(0, i)).empty())
            break;
    }
    if (!delimiter)
        return {{}, "No separator found. Use semicolon, comma or pipe between translations."};

    std::vector<std::string> fields;
    std::string field;
    bool inQuote = false;
    bool afterQuote = false;
    std::size_t line = 1;
    const auto error = [&]()
    {
        return ParseResult{{},
                           "Invalid CSV near line " + std::to_string(line) +
                               ". Check quotes and empty translations."};
    };
    const auto finishRecord = [&]() -> bool
    {
        fields.push_back(trim(field));
        field.clear();
        afterQuote = false;
        if (fields.size() == 1 && fields.front().empty())
        {
            fields.clear();
            return true;
        }
        if (fields.size() < 2 || fields.front().empty() || fields.back().empty())
            return false;
        Card card;
        for (std::size_t i = 0; i + 1 < fields.size(); ++i)
        {
            if (fields[i].empty())
                continue; // Optional inflections in the original lists.
            if (!card.front.empty())
                card.front += '\n';
            card.front += fields[i];
        }
        card.back = std::move(fields.back());
        result.cards.push_back(std::move(card));
        fields.clear();
        return true;
    };
    for (std::size_t i = 0; i < csv.size(); ++i)
    {
        const char c = csv[i];
        if (inQuote)
        {
            if (c == '"')
            {
                if (i + 1 < csv.size() && csv[i + 1] == '"')
                {
                    field += '"';
                    ++i;
                }
                else
                {
                    inQuote = false;
                    afterQuote = true;
                }
            }
            else if (c != '\r')
                field += c;
        }
        else if (c == delimiter)
        {
            fields.push_back(trim(field));
            field.clear();
            afterQuote = false;
        }
        else if (c == '\n')
        {
            if (!finishRecord())
                return error();
        }
        else if (c == '\r')
        {
        }
        else if (c == '"')
        {
            if (afterQuote || !trim(field).empty())
                return error();
            field.clear();
            inQuote = true;
        }
        else
        {
            if (afterQuote && c != ' ' && c != '\t')
                return error();
            field += c;
        }
        if (c == '\n')
            ++line;
    }
    if (inQuote)
        return error();
    if ((!field.empty() || !fields.empty() || afterQuote) && !finishRecord())
        return error();
    if (result.cards.empty())
        return {{}, "The list contains no vocabulary pairs."};
    return result;
}

Session::Session(std::uint32_t seed) : random_(seed) {}

void Session::start(std::vector<Card> cards, bool reverse)
{
    cards_ = std::move(cards);
    attempts_ = 0;
    current_ = 0;
    for (auto &card : cards_)
    {
        card.score = 0;
        if (reverse)
            std::swap(card.front, card.back);
    }
    if (!cards_.empty())
        current_ = std::uniform_int_distribution<std::size_t>(0, cards_.size() - 1)(random_);
}

const Card *Session::current() const { return remaining() ? &cards_[current_] : nullptr; }

std::size_t Session::remaining() const
{
    return static_cast<std::size_t>(std::count_if(cards_.begin(), cards_.end(), [](const auto &card)
                                                  { return card.score <= 0; }));
}

bool Session::answer(bool known)
{
    if (!current())
        return false;
    auto &score = cards_[current_].score;
    if (known)
        ++score;
    else if (score > std::numeric_limits<int>::min() + 1)
        --score;
    ++attempts_;
    std::vector<std::size_t> candidates;
    for (std::size_t i = 0; i < cards_.size(); ++i)
        if (cards_[i].score <= 0 && i != current_)
            candidates.push_back(i);
    if (!candidates.empty())
        current_ = candidates[std::uniform_int_distribution<std::size_t>(0, candidates.size() -
                                                                                1)(random_)];
    return current() != nullptr;
}
} // namespace vocab
