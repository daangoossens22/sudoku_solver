#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <vector>

constexpr size_t BLOCK_WIDTH = 3;
constexpr size_t NUMBER_RANGE = BLOCK_WIDTH * BLOCK_WIDTH;
constexpr size_t NUM_BOARD_CELLS = NUMBER_RANGE * NUMBER_RANGE;
constexpr std::bitset<NUMBER_RANGE> ALL_SET = { (1 << NUMBER_RANGE) - 1 };
// constexpr size_t ALL_POSSIBLE_VALUE = (1 << NUMBER_RANGE) - 1;
// static_assert(, "only supports 3 x 3 sudoku size where each subblock is a 1x1, 2x2, 3x3, etc");

using cell = std::bitset<NUMBER_RANGE>;
using board = std::array<cell, NUM_BOARD_CELLS>;

board parse_sudoku(const std::string& grid)
{
    assert(grid.size() == NUM_BOARD_CELLS);

    board res = {};
    std::ranges::transform(grid, res.begin(), [](char c) -> std::bitset<NUMBER_RANGE> {
        int num = c - '0';
        return (num == std::clamp(num, 1, (int)NUMBER_RANGE)) ? (1 << (num - 1)) : ALL_SET;
    });

    return res;
}

std::string board_to_str(const board& b, bool debug = false)
{
    std::stringstream ss;
    std::stringstream ss_meta;
    for (size_t i = 0; i < NUM_BOARD_CELLS; ++i)
    {
        if (i != 0)
        {
            if (i % (BLOCK_WIDTH * NUMBER_RANGE) == 0) ss << "\n---|---|---";
            if (i % NUMBER_RANGE == 0)
            {
                if (debug)
                {
                    ss << ss_meta.str();
                    ss_meta.str(std::string());
                }
                ss << '\n';
            }
            else if (i % 3 == 0) ss << '|';
        }
        if (b[i].count() == 1) ss << std::bit_width(b[i].to_ulong());
        else
        {
            ss << '.';
            ss_meta << " { ";
            for (size_t x = 0; x < NUMBER_RANGE; ++x)
            {
                if ((b[i].to_ulong() >> x) & 1) ss_meta << x + 1 << " ";
            }
            ss_meta << "}";
        }
    }

    return ss.str();
}

// solve using backtracking with pruning
// 1. propagate/inference/deduction/pruning step
// 2. conflict? -> backtrack
// 3. full solution? -> DONE
// 4. select a variable
// 5. assign the variable a value in its domain
// 6. full solution? -> backtrack else goto step 1
bool solve_sudoku(board& b)
{
    board b2 = { b };
    // step 1.
    auto one_bit_set = [](const cell& i) -> bool { return i.count() == 1; };
    auto no_bit_set = [](const cell& i) -> bool { return i.none(); };
    auto multiple_bits_set = [](const cell& i) -> bool { return i.count() > 1; };
    // auto set_bits = std::views::all(b2) | std::views::filter(one_bit_set);

    auto get_row_idx = [](size_t i) -> size_t { return i / NUMBER_RANGE; };
    auto get_col_idx = [](size_t i) -> size_t { return i % NUMBER_RANGE; };
    auto get_box_idx = [](size_t row, size_t col) -> size_t {
        size_t box_row = row / BLOCK_WIDTH;
        size_t box_col = col / BLOCK_WIDTH;
        return box_col + (box_row * BLOCK_WIDTH);
    };

    // std::cout << board_to_str(b) << "\n" << std::endl;

    bool repeat = true;
    while (repeat)
    {
        repeat = false;
        for (size_t i = 0; i < b2.size(); ++i)
        {
            size_t cur_row = get_row_idx(i);
            size_t cur_col = get_col_idx(i);
            size_t cur_box = get_box_idx(cur_row, cur_col);
            enum Area
            {
                ROW,
                COLUMN,
                BOX,
                ALL_OF_THE_ABOVE
            };
            auto get_view = [&](Area a) {
                return std::views::iota(0, (int)NUM_BOARD_CELLS) |
                       std::views::filter([i](int x) { return x != (int)i; }) |
                       std::views::filter([=](int x) {
                           size_t r = get_row_idx(x);
                           size_t c = get_col_idx(x);
                           size_t b = get_box_idx(r, c);
                           if (a == Area::ROW) return r == cur_row;
                           else if (a == Area::COLUMN) return c == cur_col;
                           else if (a == Area::BOX) return b == cur_box;
                           else return r == cur_row || c == cur_col || b == cur_box;
                       });
            };
            if (one_bit_set(b2[i]))
            {
                size_t idx = std::bit_width(b2[i].to_ulong()) - 1;
                for (int x : get_view(Area::ALL_OF_THE_ABOVE))
                {
                    // b2[x] &= (~b2[i]);
                    repeat |= b2[x].test(idx);
                    b2[x].reset(idx);
                }
            }
            else
            {
                // if the cell contains a value, which is already ruled out for all cells in the
                // row or column or box, then that cell must have that value
                auto check_only_possibility = [&](Area a) {
                    cell temp = { b2[i] };
                    for (int x : get_view(a)) temp &= (~b2[x]);
                    if (one_bit_set(temp))
                    {
                        b2[i] = temp;
                        return true;
                    }
                    return false;
                };

                bool changed = check_only_possibility(Area::ROW) || check_only_possibility(Area::COLUMN) ||
                               check_only_possibility(Area::BOX);
                repeat |= changed;
            }
        }
    }

    // step 2.
    // only when a domain is empty
    if (std::ranges::any_of(b2, no_bit_set)) return false;

    // step 3
    if (std::ranges::all_of(b2, one_bit_set))
    {
        b = b2;
        return true;
    }

    // step 4
    auto c = std::ranges::find_if(b2, multiple_bits_set);
    // step 5
    // step 6
    while (c->count() != 0)
    {
        cell if_fail = *c;
        *c = std::bit_floor(c->to_ulong());
        if_fail &= (~(*c));
        if (solve_sudoku(b2))
        {
            b = b2;
            return true;
        }
        *c = if_fail;
    }
    return false;
}

// TODO: print each solve state and continue when a key is pressed
int main()
{
    std::string start = "-2-------"
                        "---6----3"
                        "-74-8----"
                        "-----3--2"
                        "-8--4--1-"
                        "6--5-----"
                        "----1-78-"
                        "5----9---"
                        "-------4-";

    board start_board = parse_sudoku(start);
    board solution = { start_board };
    solve_sudoku(solution);

    std::cout << "before: \n"
              << board_to_str(start_board) << "\n\nafter: \n"
              << board_to_str(solution) << std::endl;
}
