#include "../headers/UIDequeSFML.h"

#include "../headers/SegmentedDeque.h"
#include "../headers/SegmentedDequeIO.h"
#include "../headers/Sequence.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using T = int;
using DequeType = SegmentedDeque<T>;
using SeqType = Sequence<T>;

namespace {

const sf::Color Background(21, 25, 32);
const sf::Color Panel(31, 38, 49);
const sf::Color PanelLight(42, 51, 65);
const sf::Color Ink(238, 242, 247);
const sf::Color Muted(156, 168, 184);
const sf::Color Accent(83, 168, 255);
const sf::Color AccentSoft(47, 105, 168);
const sf::Color Good(83, 202, 138);
const sf::Color Warn(255, 189, 89);
const sf::Color EmptySlot(55, 64, 77);
const sf::Color Border(82, 95, 112);
const float SidebarWidth = 252.f;
const float ContentLeft = 276.f;

enum class Action {
    NewDeque,
    Info,
    Append,
    Prepend,
    InsertAt,
    DeleteAt,
    Subsequence,
    Concat,
    FindSubsequence,
    PopFirst,
    PopLast,
    PeekFirst,
    PeekLast,
    Clear,
    Demo
};

struct Button {
    Action action;
    std::string label;
    sf::FloatRect rect;
};

struct InputField {
    std::string label;
    std::string placeholder;
    std::string value;
    sf::FloatRect rect;
};

struct ParsedSlots {
    std::vector<std::vector<std::optional<int>>> segments;
};

std::string trim(const std::string &text) {
    size_t first = 0;
    while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) {
        ++first;
    }

    size_t last = text.size();
    while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
        --last;
    }

    return text.substr(first, last - first);
}

std::string normalizeNumericInput(const std::string &text) {
    std::string result;
    bool previousWasSpace = false;

    for (char ch: text) {
        if (std::isdigit(static_cast<unsigned char>(ch)) || ch == '-') {
            result.push_back(ch);
            previousWasSpace = false;
        } else if (ch == ',' || ch == ';' || std::isspace(static_cast<unsigned char>(ch))) {
            if (!previousWasSpace) {
                result.push_back(' ');
                previousWasSpace = true;
            }
        }
    }

    return trim(result);
}

bool parseInt(const std::string &text, int &value) {
    std::istringstream input(normalizeNumericInput(text));
    input >> value;
    return !input.fail();
}

bool parseSize(const std::string &text, size_t &value) {
    int signedValue = 0;
    if (!parseInt(text, signedValue) || signedValue < 0) {
        return false;
    }
    value = static_cast<size_t>(signedValue);
    return true;
}

std::vector<int> parseList(const std::string &text) {
    std::istringstream input(normalizeNumericInput(text));
    std::vector<int> values;
    int value = 0;
    while (input >> value) {
        values.push_back(value);
    }

    if (values.empty()) {
        throw std::invalid_argument("List field must contain at least one number");
    }

    input.clear();
    std::string rest;
    if (input >> rest) {
        throw std::invalid_argument("List field contains an invalid token");
    }

    return values;
}

void replaceDeque(std::unique_ptr<DequeType> &current, SeqType *result) {
    if (!result || result == current.get()) {
        return;
    }

    auto *dequeResult = dynamic_cast<DequeType *>(result);
    if (!dequeResult) {
        delete result;
        throw std::runtime_error("Operation returned an unexpected sequence type");
    }

    current.reset(dequeResult);
}

std::vector<int> logicalValues(const DequeType &deque) {
    std::vector<int> values;
    std::unique_ptr<IEnumerator<int>> iterator(deque.GetEnumerator());
    while (iterator->MoveNext()) {
        values.push_back(iterator->Current());
    }
    return values;
}

ParsedSlots parseSlots(const DequeType &deque) {
    std::ostringstream output;
    output << deque;

    ParsedSlots result;
    const std::string raw = output.str();
    size_t pos = 0;

    while ((pos = raw.find('[', pos)) != std::string::npos) {
        const size_t end = raw.find(']', pos);
        if (end == std::string::npos) {
            break;
        }

        std::vector<std::optional<int>> segment;
        std::stringstream cells(raw.substr(pos + 1, end - pos - 1));
        std::string token;
        while (std::getline(cells, token, ',')) {
            token = trim(token);
            if (token == "_") {
                segment.push_back(std::nullopt);
            } else {
                segment.push_back(std::stoi(token));
            }
        }

        result.segments.push_back(segment);
        pos = end + 1;
    }

    return result;
}

std::string dequeValuesText(const DequeType &deque) {
    const auto values = logicalValues(deque);
    if (values.empty()) {
        return "Logical order: <empty>";
    }

    std::ostringstream output;
    output << "Logical order: ";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            output << "  ";
        }
        output << values[i];
    }
    return output.str();
}

bool loadUiFont(sf::Font &font) {
    const std::vector<std::string> candidates = {
            "C:/Windows/Fonts/segoeui.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/consola.ttf",
            "arial.ttf"
    };

    for (const auto &path: candidates) {
        if (font.openFromFile(path)) {
            return true;
        }
    }
    return false;
}

void drawRect(sf::RenderWindow &window,
              const sf::FloatRect &rect,
              sf::Color fill,
              sf::Color outline = sf::Color::Transparent,
              float thickness = 0.f) {
    sf::RectangleShape shape(rect.size);
    shape.setPosition(rect.position);
    shape.setFillColor(fill);
    shape.setOutlineColor(outline);
    shape.setOutlineThickness(thickness);
    window.draw(shape);
}

void drawText(sf::RenderWindow &window,
              const sf::Font &font,
              const std::string &text,
              unsigned int size,
              sf::Vector2f position,
              sf::Color color = Ink,
              std::uint32_t style = sf::Text::Regular) {
    sf::Text drawable(font, text, size);
    drawable.setPosition(position);
    drawable.setFillColor(color);
    drawable.setStyle(style);
    window.draw(drawable);
}

void drawCenteredText(sf::RenderWindow &window,
                      const sf::Font &font,
                      const std::string &text,
                      unsigned int size,
                      const sf::FloatRect &box,
                      sf::Color color = Ink,
                      std::uint32_t style = sf::Text::Regular) {
    sf::Text drawable(font, text, size);
    drawable.setFillColor(color);
    drawable.setStyle(style);
    const sf::FloatRect bounds = drawable.getLocalBounds();
    drawable.setPosition({
            box.position.x + (box.size.x - bounds.size.x) / 2.f - bounds.position.x,
            box.position.y + (box.size.y - bounds.size.y) / 2.f - bounds.position.y - 2.f
    });
    window.draw(drawable);
}

bool contains(const sf::FloatRect &rect, sf::Vector2f point) {
    return rect.contains(point);
}

sf::Color mix(sf::Color a, sf::Color b, float t) {
    t = std::clamp(t, 0.f, 1.f);
    const auto blend = [t](std::uint8_t left, std::uint8_t right) {
        return static_cast<std::uint8_t>(std::round(left + (right - left) * t));
    };

    return {
            blend(a.r, b.r),
            blend(a.g, b.g),
            blend(a.b, b.b),
            blend(a.a, b.a)
    };
}

void drawButton(sf::RenderWindow &window, const sf::Font &font, const Button &button, sf::Vector2f mouse, bool selected) {
    const bool hovered = contains(button.rect, mouse);
    const sf::Color fill = selected ? sf::Color(54, 128, 204) :
                          hovered ? sf::Color(49, 90, 135) :
                                    sf::Color(39, 48, 62);
    const sf::Color outline = selected ? sf::Color(142, 207, 255) :
                             hovered ? sf::Color(120, 193, 255) :
                                       sf::Color(72, 86, 105);
    drawRect(window, button.rect, fill, outline, selected ? 2.f : 1.f);
    drawCenteredText(window, font, button.label, 15, button.rect, Ink, sf::Text::Bold);
}

void drawField(sf::RenderWindow &window, const sf::Font &font, const InputField &field, bool active) {
    drawText(window, font, field.label, 14, {field.rect.position.x, field.rect.position.y - 22.f}, Muted);
    drawRect(window, field.rect, active ? sf::Color(36, 48, 66) : Panel,
             active ? Accent : Border, active ? 2.f : 1.f);

    std::string text = field.value;
    if (text.empty()) {
        text = field.placeholder;
    }
    drawText(window, font, text, 17, {field.rect.position.x + 10.f, field.rect.position.y + 9.f},
             field.value.empty() ? sf::Color(104, 116, 132) : Ink);
}

std::unique_ptr<DequeType> makeDequeFromValues(size_t segmentLength, const std::vector<int> &values) {
    auto deque = std::make_unique<DequeType>(segmentLength);
    for (int value: values) {
        deque->AppendImpl(value);
    }
    return deque;
}

void drawDeque(sf::RenderWindow &window,
               const sf::Font &font,
               const DequeType &deque,
               const std::vector<size_t> &highlighted,
               float flashProgress) {
    const ParsedSlots slots = parseSlots(deque);
    const sf::Vector2u windowSize = window.getSize();
    const float left = ContentLeft;
    const float top = 322.f;
    const float maxRight = std::max(left + 320.f, static_cast<float>(windowSize.x) - 40.f);
    const float rowGap = 30.f;
    const float segmentGap = 18.f;
    float x = left;
    float y = top;
    size_t logicalIndex = 0;

    const sf::FloatRect stage({left - 14.f, top - 54.f}, {maxRight - left + 28.f, std::max(230.f, static_cast<float>(windowSize.y) - top - 250.f)});
    const float logTop = std::max(560.f, static_cast<float>(windowSize.y) - 156.f);
    drawRect(window, stage, sf::Color(18, 23, 31), sf::Color(38, 49, 64), 1.f);
    drawText(window, font, dequeValuesText(deque), 18, {left, logTop - 32.f}, Ink);

    if (slots.segments.empty()) {
        drawText(window, font, "No segment data", 22, {left, top}, Muted);
        return;
    }

    for (size_t segIdx = 0; segIdx < slots.segments.size(); ++segIdx) {
        const auto &segment = slots.segments[segIdx];
        const float rawCellWidth = (maxRight - left - 36.f) / static_cast<float>(std::max<size_t>(segment.size(), 1));
        const float cellWidth = std::clamp(rawCellWidth, 34.f, 58.f);
        const float cellHeight = 48.f;
        const float segWidth = segment.size() * cellWidth + 12.f;

        if (x + segWidth > maxRight && x > left) {
            x = left;
            y += cellHeight + rowGap + 26.f;
        }

        sf::FloatRect segmentBox({x - 6.f, y - 28.f}, {segWidth, cellHeight + 54.f});
        drawRect(window, segmentBox, sf::Color(27, 33, 43), Border, 1.f);
        drawText(window, font, "segment " + std::to_string(segIdx), 13, {x, y - 24.f}, Muted);

        for (size_t cellIdx = 0; cellIdx < segment.size(); ++cellIdx) {
            const sf::FloatRect cellBox({x + cellIdx * cellWidth, y}, {cellWidth - 4.f, cellHeight});
            sf::Color fill = EmptySlot;
            sf::Color outline = Border;
            std::string label = "_";

            if (segment[cellIdx].has_value()) {
                const bool isHighlighted = std::find(highlighted.begin(), highlighted.end(), logicalIndex) != highlighted.end();
                fill = isHighlighted ? mix(Accent, Good, flashProgress) : AccentSoft;
                outline = isHighlighted ? Good : Accent;
                label = std::to_string(*segment[cellIdx]);
                ++logicalIndex;
            }

            drawRect(window, cellBox, fill, outline, 1.f);
            drawCenteredText(window, font, label, label.size() > 3 ? 15 : 18, cellBox,
                             segment[cellIdx].has_value() ? Ink : Muted, sf::Text::Bold);
        }

        x += segWidth + segmentGap;
    }
}

void pushLog(std::vector<std::string> &log, const std::string &message) {
    log.push_back(message);
    if (log.size() > 5) {
        log.erase(log.begin());
    }
}

std::vector<Button> makeButtons() {
    std::vector<Button> buttons;
    const std::vector<std::pair<Action, std::string>> data = {
            {Action::NewDeque, "New deque"},
            {Action::Info, "Info"},
            {Action::Append, "Append"},
            {Action::Prepend, "Prepend"},
            {Action::InsertAt, "InsertAt"},
            {Action::DeleteAt, "DeleteAt"},
            {Action::Subsequence, "Subsequence"},
            {Action::Concat, "Concat"},
            {Action::FindSubsequence, "FindSubseq"},
            {Action::PopFirst, "PopFirst"},
            {Action::PopLast, "PopLast"},
            {Action::PeekFirst, "PeekFirst"},
            {Action::PeekLast, "PeekLast"},
            {Action::Clear, "Clear empty"},
            {Action::Demo, "Demo"}
    };

    const float x = 24.f;
    float y = 86.f;
    for (const auto &[action, label]: data) {
        buttons.push_back({action, label, {{x, y}, {204.f, 34.f}}});
        y += 40.f;
    }
    return buttons;
}

void ensureDeque(const std::unique_ptr<DequeType> &current) {
    if (!current) {
        throw std::runtime_error("Create a deque first");
    }
}

bool actionNeedsInput(Action action) {
    switch (action) {
        case Action::NewDeque:
        case Action::Append:
        case Action::Prepend:
        case Action::InsertAt:
        case Action::DeleteAt:
        case Action::Subsequence:
        case Action::Concat:
        case Action::FindSubsequence:
            return true;
        default:
            return false;
    }
}

std::optional<char> keyToInputChar(sf::Keyboard::Key key, sf::Keyboard::Scancode scancode) {
    using Key = sf::Keyboard::Key;
    using Scan = sf::Keyboard::Scan;

    switch (scancode) {
        case Scan::Num0:
        case Scan::Numpad0:
            return '0';
        case Scan::Num1:
        case Scan::Numpad1:
            return '1';
        case Scan::Num2:
        case Scan::Numpad2:
            return '2';
        case Scan::Num3:
        case Scan::Numpad3:
            return '3';
        case Scan::Num4:
        case Scan::Numpad4:
            return '4';
        case Scan::Num5:
        case Scan::Numpad5:
            return '5';
        case Scan::Num6:
        case Scan::Numpad6:
            return '6';
        case Scan::Num7:
        case Scan::Numpad7:
            return '7';
        case Scan::Num8:
        case Scan::Numpad8:
            return '8';
        case Scan::Num9:
        case Scan::Numpad9:
            return '9';
        case Scan::Hyphen:
        case Scan::NumpadMinus:
            return '-';
        case Scan::Comma:
            return ',';
        case Scan::Semicolon:
            return ';';
        case Scan::Space:
            return ' ';
        default:
            break;
    }

    switch (key) {
        case Key::Num0:
        case Key::Numpad0:
            return '0';
        case Key::Num1:
        case Key::Numpad1:
            return '1';
        case Key::Num2:
        case Key::Numpad2:
            return '2';
        case Key::Num3:
        case Key::Numpad3:
            return '3';
        case Key::Num4:
        case Key::Numpad4:
            return '4';
        case Key::Num5:
        case Key::Numpad5:
            return '5';
        case Key::Num6:
        case Key::Numpad6:
            return '6';
        case Key::Num7:
        case Key::Numpad7:
            return '7';
        case Key::Num8:
        case Key::Numpad8:
            return '8';
        case Key::Num9:
        case Key::Numpad9:
            return '9';
        case Key::Hyphen:
        case Key::Subtract:
            return '-';
        case Key::Comma:
            return ',';
        case Key::Semicolon:
            return ';';
        case Key::Space:
            return ' ';
        default:
            return std::nullopt;
    }
}

InputField makeInputForAction(Action action) {
    switch (action) {
        case Action::NewDeque:
            return {"New deque: segment length", "example: 4", "4", {{274.f, 126.f}, {556.f, 42.f}}};
        case Action::Append:
            return {"Append: value", "example: 10", "", {{274.f, 126.f}, {556.f, 42.f}}};
        case Action::Prepend:
            return {"Prepend: value", "example: 10", "", {{274.f, 126.f}, {556.f, 42.f}}};
        case Action::InsertAt:
            return {"InsertAt: value index", "example: 99 2", "", {{274.f, 126.f}, {556.f, 42.f}}};
        case Action::DeleteAt:
            return {"DeleteAt: index", "example: 2", "", {{274.f, 126.f}, {556.f, 42.f}}};
        case Action::Subsequence:
            return {"Subsequence: start end", "example: 1 3", "", {{274.f, 126.f}, {556.f, 42.f}}};
        case Action::Concat:
            return {"Concat: values", "example: 1, 2, 3", "", {{274.f, 126.f}, {556.f, 42.f}}};
        case Action::FindSubsequence:
            return {"FindSubsequence: values", "example: 1, 2, 3", "", {{274.f, 126.f}, {556.f, 42.f}}};
        default:
            return {"", "", "", {{274.f, 126.f}, {556.f, 42.f}}};
    }
}

std::pair<int, size_t> parseValueAndIndex(const std::string &text) {
    std::istringstream input(normalizeNumericInput(text));
    int value = 0;
    int index = 0;
    input >> value >> index;
    input >> std::ws;
    if (input.fail() || !input.eof() || index < 0) {
        throw std::invalid_argument("Expected: value index");
    }
    return {value, static_cast<size_t>(index)};
}

std::pair<size_t, size_t> parseTwoIndices(const std::string &text) {
    std::istringstream input(normalizeNumericInput(text));
    int first = 0;
    int second = 0;
    input >> first >> second;
    input >> std::ws;
    if (input.fail() || !input.eof() || first < 0 || second < 0) {
        throw std::invalid_argument("Expected: start end");
    }
    return {static_cast<size_t>(first), static_cast<size_t>(second)};
}

void runAction(Action action,
               std::unique_ptr<DequeType> &current,
               size_t &segmentLength,
               const std::string &inputText,
               std::vector<std::string> &log,
               std::vector<size_t> &highlighted,
               sf::Clock &flashClock) {
    highlighted.clear();

    switch (action) {
        case Action::NewDeque: {
            size_t newSegmentLength = 0;
            if (!parseSize(inputText, newSegmentLength) || newSegmentLength < 2) {
                throw std::invalid_argument("Segment length must be >= 2");
            }
            segmentLength = newSegmentLength;
            current = std::make_unique<DequeType>(segmentLength);
            pushLog(log, "Created empty deque with segment length " + std::to_string(segmentLength));
            break;
        }
        case Action::Info:
            ensureDeque(current);
            pushLog(log, "Length = " + std::to_string(current->GetLength()) +
                         ", empty = " + std::string(current->IsEmpty() ? "true" : "false"));
            break;
        case Action::Append: {
            ensureDeque(current);
            int value = 0;
            if (!parseInt(inputText, value)) {
                throw std::invalid_argument("Value field must contain an integer");
            }
            const size_t index = current->GetLength();
            replaceDeque(current, current->AppendImpl(value));
            highlighted.push_back(index);
            pushLog(log, "Append(" + std::to_string(value) + ")");
            break;
        }
        case Action::Prepend: {
            ensureDeque(current);
            int value = 0;
            if (!parseInt(inputText, value)) {
                throw std::invalid_argument("Value field must contain an integer");
            }
            replaceDeque(current, current->PrependImpl(value));
            highlighted.push_back(0);
            pushLog(log, "Prepend(" + std::to_string(value) + ")");
            break;
        }
        case Action::InsertAt: {
            ensureDeque(current);
            const auto [value, index] = parseValueAndIndex(inputText);
            replaceDeque(current, current->InsertAtImpl(value, index));
            highlighted.push_back(index);
            pushLog(log, "InsertAt(" + std::to_string(value) + ", " + std::to_string(index) + ")");
            break;
        }
        case Action::DeleteAt: {
            ensureDeque(current);
            size_t index = 0;
            if (!parseSize(inputText, index)) {
                throw std::invalid_argument("Index field must contain a valid index");
            }
            replaceDeque(current, current->DelImpl(index));
            if (current->GetLength() > 0) {
                highlighted.push_back(std::min(index, current->GetLength() - 1));
            }
            pushLog(log, "DeleteAt(" + std::to_string(index) + ")");
            break;
        }
        case Action::Subsequence: {
            ensureDeque(current);
            const auto [start, end] = parseTwoIndices(inputText);
            replaceDeque(current, current->GetSubsequence(start, end));
            for (size_t i = 0; i < current->GetLength(); ++i) {
                highlighted.push_back(i);
            }
            pushLog(log, "Replaced by subsequence [" + std::to_string(start) + ", " + std::to_string(end) + "]");
            break;
        }
        case Action::Concat: {
            ensureDeque(current);
            const std::vector<int> values = parseList(inputText);
            auto other = makeDequeFromValues(segmentLength, values);
            const size_t oldLength = current->GetLength();
            replaceDeque(current, current->ConcatImpl(*other));
            for (size_t i = oldLength; i < current->GetLength(); ++i) {
                highlighted.push_back(i);
            }
            pushLog(log, "Concat with " + std::to_string(values.size()) + " values");
            break;
        }
        case Action::FindSubsequence: {
            ensureDeque(current);
            const std::vector<int> values = parseList(inputText);
            auto other = makeDequeFromValues(segmentLength, values);
            const int position = current->FindSubsequence(*other);
            if (position >= 0) {
                for (size_t i = 0; i < values.size(); ++i) {
                    highlighted.push_back(static_cast<size_t>(position) + i);
                }
                pushLog(log, "FindSubsequence: found at index " + std::to_string(position));
            } else {
                pushLog(log, "FindSubsequence: not found");
            }
            break;
        }
        case Action::PopFirst: {
            ensureDeque(current);
            const int value = current->PopFirst();
            if (current->GetLength() > 0) {
                highlighted.push_back(0);
            }
            pushLog(log, "PopFirst -> " + std::to_string(value));
            break;
        }
        case Action::PopLast: {
            ensureDeque(current);
            const int value = current->PopLast();
            if (current->GetLength() > 0) {
                highlighted.push_back(current->GetLength() - 1);
            }
            pushLog(log, "PopLast -> " + std::to_string(value));
            break;
        }
        case Action::PeekFirst:
            ensureDeque(current);
            highlighted.push_back(0);
            pushLog(log, "PeekFirst -> " + std::to_string(current->GetFirst()));
            break;
        case Action::PeekLast:
            ensureDeque(current);
            highlighted.push_back(current->GetLength() - 1);
            pushLog(log, "PeekLast -> " + std::to_string(current->GetLast()));
            break;
        case Action::Clear:
            ensureDeque(current);
            replaceDeque(current, current->CreateEmpty());
            pushLog(log, "Replaced by empty deque");
            break;
        case Action::Demo: {
            segmentLength = 4;
            current = std::make_unique<DequeType>(segmentLength);
            for (int value: {10, 20, 30}) {
                current->AppendImpl(value);
            }
            current->PrependImpl(5);
            current->AppendImpl(40);
            highlighted = {0, 1, 2, 3, 4};
            pushLog(log, "Demo loaded: 5, 10, 20, 30, 40");
            break;
        }
    }

    flashClock.restart();
}

void drawLog(sf::RenderWindow &window, const sf::Font &font, const std::vector<std::string> &log) {
    const sf::Vector2u windowSize = window.getSize();
    const float right = std::max(ContentLeft + 360.f, static_cast<float>(windowSize.x) - 40.f);
    const float top = std::max(560.f, static_cast<float>(windowSize.y) - 156.f);
    const sf::FloatRect rect({ContentLeft, top}, {right - ContentLeft, 126.f});
    drawRect(window, rect, sf::Color(28, 35, 46), sf::Color(86, 104, 126), 1.f);
    drawText(window, font, "Operation log", 15, {rect.position.x + 12.f, rect.position.y + 10.f}, Muted, sf::Text::Bold);

    float y = rect.position.y + 34.f;
    for (const std::string &message: log) {
        drawText(window, font, message, 16, {rect.position.x + 12.f, y}, Ink);
        y += 20.f;
    }
}

} // namespace

int runSFML() {
    sf::Font font;
    if (!loadUiFont(font)) {
        return 1;
    }

    const sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode(desktop.size), "SegmentedDeque SFML UI", sf::Style::Default);
    window.setFramerateLimit(60);

    std::unique_ptr<DequeType> current = std::make_unique<DequeType>(4);
    size_t segmentLength = 4;
    std::vector<Button> buttons = makeButtons();
    std::vector<std::string> log = {"Ready. Use the controls on the left."};
    std::vector<size_t> highlighted;
    sf::Clock flashClock;
    bool inputVisible = false;
    std::optional<Action> pendingAction;
    InputField commandInput = makeInputForAction(Action::Append);
    const sf::FloatRect applyRect({850.f, 126.f}, {128.f, 42.f});

    const auto executePending = [&]() {
        if (!pendingAction.has_value()) {
            return;
        }
        const std::string normalized = normalizeNumericInput(commandInput.value);
        runAction(*pendingAction, current, segmentLength, normalized, log, highlighted, flashClock);
        inputVisible = false;
        pendingAction.reset();
    };

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto *resized = event->getIf<sf::Event::Resized>()) {
                window.setView(sf::View(sf::FloatRect({0.f, 0.f},
                                                      {static_cast<float>(resized->size.x),
                                                       static_cast<float>(resized->size.y)})));
            }

            if (const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    const sf::Vector2f mouse(static_cast<float>(mousePressed->position.x),
                                             static_cast<float>(mousePressed->position.y));

                    for (const Button &button: buttons) {
                        if (contains(button.rect, mouse)) {
                            try {
                                if (actionNeedsInput(button.action)) {
                                    commandInput = makeInputForAction(button.action);
                                    pendingAction = button.action;
                                    inputVisible = true;
                                    pushLog(log, "Selected: " + commandInput.label);
                                } else {
                                    runAction(button.action, current, segmentLength, "", log, highlighted, flashClock);
                                    inputVisible = false;
                                    pendingAction.reset();
                                }
                            } catch (const std::exception &error) {
                                highlighted.clear();
                                pushLog(log, std::string("Error: ") + error.what());
                                flashClock.restart();
                            }
                        }
                    }

                    if (inputVisible && contains(applyRect, mouse)) {
                        try {
                            executePending();
                        } catch (const std::exception &error) {
                            highlighted.clear();
                            pushLog(log, std::string("Error: ") + error.what());
                            flashClock.restart();
                        }
                    }
                }
            }

            if (const auto *text = event->getIf<sf::Event::TextEntered>()) {
                if (inputVisible && text->unicode >= 32 && text->unicode < 128) {
                    const char ch = static_cast<char>(text->unicode);
                    const bool allowed = std::isdigit(static_cast<unsigned char>(ch)) ||
                                         ch == '-' || ch == ',' || ch == ';' || ch == ' ';
                    const bool alreadyAddedByKeyPressed = !commandInput.value.empty() &&
                                                          commandInput.value.back() == ch;
                    if (allowed && !alreadyAddedByKeyPressed) {
                        commandInput.value.push_back(ch);
                    }
                }
            }

            if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                } else if (key->code == sf::Keyboard::Key::Backspace && inputVisible) {
                    if (!commandInput.value.empty()) {
                        commandInput.value.pop_back();
                    }
                } else if (key->code == sf::Keyboard::Key::Delete && inputVisible) {
                    commandInput.value.clear();
                } else if (key->code == sf::Keyboard::Key::Enter && inputVisible) {
                    try {
                        executePending();
                    } catch (const std::exception &error) {
                        highlighted.clear();
                        pushLog(log, std::string("Error: ") + error.what());
                        flashClock.restart();
                    }
                } else if (inputVisible) {
                    if (const std::optional<char> ch = keyToInputChar(key->code, key->scancode)) {
                        commandInput.value.push_back(*ch);
                    }
                }
            }
        }

        const sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        const sf::Vector2f mouse(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));
        const float flashProgress = (std::sin(flashClock.getElapsedTime().asSeconds() * 7.f) + 1.f) / 2.f;
        const sf::Vector2u windowSize = window.getSize();

        window.clear(Background);
        drawRect(window, {{0.f, 0.f}, {SidebarWidth, static_cast<float>(windowSize.y)}}, Panel, sf::Color::Transparent);
        drawRect(window, {{SidebarWidth, 0.f}, {2.f, static_cast<float>(windowSize.y)}}, sf::Color(44, 56, 73));
        drawText(window, font, "SegmentedDeque", 24, {24.f, 24.f}, Ink, sf::Text::Bold);
        drawText(window, font, "SFML visual UI", 15, {25.f, 54.f}, Muted);

        for (const Button &button: buttons) {
            drawButton(window, font, button, mouse, pendingAction.has_value() && *pendingAction == button.action);
        }

        if (inputVisible) {
            drawField(window, font, commandInput, true);
            drawRect(window, applyRect, contains(applyRect, mouse) ? Good : AccentSoft,
                     contains(applyRect, mouse) ? Ink : Accent, 1.f);
            drawCenteredText(window, font, "Apply", 16, applyRect, Ink, sf::Text::Bold);
            drawText(window, font, "Enter = apply, Delete = clear field, Esc = exit",
                     14, {274.f, 184.f}, Muted);
        } else {
            drawText(window, font, "Choose a command. Input appears only when the command needs it.",
                     15, {274.f, 136.f}, Muted);
            drawText(window, font, "Esc = exit", 14, {274.f, 164.f}, Muted);
        }

        drawText(window, font, "Segment length: " + std::to_string(segmentLength),
                 14, {1012.f, 184.f}, Muted);

        if (current) {
            drawDeque(window, font, *current, highlighted, flashProgress);
        }
        drawLog(window, font, log);

        window.display();
    }

    return 0;
}
