#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

constexpr size_t BLOCK_WIDTH = 3;
constexpr size_t NUMBER_RANGE = BLOCK_WIDTH * BLOCK_WIDTH;
constexpr size_t NUM_BOARD_CELLS = NUMBER_RANGE * NUMBER_RANGE;
constexpr std::bitset<NUMBER_RANGE> ALL_SET = { (1 << NUMBER_RANGE) - 1 };
static_assert(BLOCK_WIDTH > 1); // whole sudoku is filled with only 1s -> no solution

using cell = std::bitset<NUMBER_RANGE>;
using board = std::array<cell, NUM_BOARD_CELLS>;

namespace
{
bool one_bit_set(const cell& i)
{
    return i.count() == 1;
};
bool no_bit_set(const cell& i)
{
    return i.none();
};
bool multiple_bits_set(const cell& i)
{
    return i.count() > 1;
};

size_t get_row_idx(const size_t i)
{
    return i / NUMBER_RANGE;
};
size_t get_col_idx(const size_t i)
{
    return i % NUMBER_RANGE;
};
size_t get_box_idx(const size_t row, const size_t col)
{
    size_t box_row = row / BLOCK_WIDTH;
    size_t box_col = col / BLOCK_WIDTH;
    return box_col + (box_row * BLOCK_WIDTH);
};

enum Area
{
    ROW,
    COLUMN,
    BOX,
    ALL_OF_THE_ABOVE
};
auto get_view(Area a, size_t idx)
{
    size_t cur_row = get_row_idx(idx);
    size_t cur_col = get_col_idx(idx);
    size_t cur_box = get_box_idx(cur_row, cur_col);
    return std::views::iota(0, (int)NUM_BOARD_CELLS) |
           std::views::filter([idx](int x) { return x != (int)idx; }) | std::views::filter([=](int x) {
               size_t r = get_row_idx(x);
               size_t c = get_col_idx(x);
               size_t b = get_box_idx(r, c);
               if (a == Area::ROW) return r == cur_row;
               else if (a == Area::COLUMN) return c == cur_col;
               else if (a == Area::BOX) return b == cur_box;
               else return r == cur_row || c == cur_col || b == cur_box;
           });
};

} // namespace

board parse_sudoku(const std::string& grid)
{
    assert(grid.size() == NUM_BOARD_CELLS);

    board res = {};
    std::ranges::transform(grid, res.begin(), [](char c) -> std::bitset<NUMBER_RANGE> {
        int num = c - '0'; // TODO: make this better
        return (num == std::clamp(num, 1, (int)NUMBER_RANGE)) ? (1 << (num - 1)) : ALL_SET;
    });

    return res;
}

std::string board_to_str(const board& b, bool debug = false)
{
    // TODO: simplify
    std::stringstream ss;
    std::stringstream ss_meta;
    std::string dash_block(BLOCK_WIDTH, '-');
    for (size_t i = 0; i < NUM_BOARD_CELLS; ++i)
    {
        if (i != 0)
        {
            if (i % (BLOCK_WIDTH * NUMBER_RANGE) == 0)
            {
                ss << "\n" << dash_block;
                for (size_t j = 1; j < BLOCK_WIDTH; ++j)
                {
                    ss << "|" << dash_block;
                }
            }
            if (i % NUMBER_RANGE == 0)
            {
                if (debug)
                {
                    ss << ss_meta.str();
                    ss_meta.str(std::string());
                }
                ss << '\n';
            }
            else if (i % BLOCK_WIDTH == 0) ss << '|';
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

    bool repeat = true;
    while (repeat)
    {
        repeat = false;
        for (size_t i = 0; i < b2.size(); ++i)
        {
            if (one_bit_set(b2[i]))
            {
                size_t set_idx = std::bit_width(b2[i].to_ulong()) - 1;
                for (int x : get_view(Area::ALL_OF_THE_ABOVE, i))
                {
                    // b2[x] &= (~b2[i]);
                    repeat |= b2[x].test(set_idx);
                    b2[x].reset(set_idx);
                }
            }
            else
            {
                // if the cell contains a value, which is already ruled out for all cells in the
                // row or column or box, then that cell must have that value
                auto check_only_possibility = [&](Area a) {
                    cell temp = { b2[i] };
                    for (int x : get_view(a, i)) temp &= (~b2[x]);
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
// TODO: make this a class
// TODO: add tests
int main()
{
    std::string start_3x3 = "-2-------"
                            "---6----3"
                            "-74-8----"
                            "-----3--2"
                            "-8--4--1-"
                            "6--5-----"
                            "----1-78-"
                            "5----9---"
                            "-------4-";
    std::string start_2x2 = "12--"
                            "3--2"
                            "2-4-"
                            "4--1";

    board start_board = parse_sudoku(start_3x3);
    board solution = { start_board };
    solve_sudoku(solution);

    std::cout << "before: \n"
              << board_to_str(start_board) << "\n\nafter: \n"
              << board_to_str(solution) << std::endl;
}
