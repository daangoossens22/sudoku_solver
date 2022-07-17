#include <iostream>
#include <vector>

using namespace std;

vector<vector<int>> parse_sudoku(string grid);
void print_sudoku_board(vector<vector<int>> sudoku);

bool one_bit_set(int num)
{
  return num && !(num & (num - 1));
}

bool solve_sudoku(vector<vector<int>>& grid_solution)
{
  vector<vector<int>> grid = {grid_solution}; // make a local copy
  // propagate/inference/deduction/pruning step
  // get a list of all the coord to search
  vector<pair<int, int>> coords;
  for (int y = 0; y < grid.size(); ++y)
  {
    for (int x = 0; x < grid.size(); ++x)
    {
      coords.push_back(make_pair(x, y));
    }
  }
  int index = 0;
  int old_size = coords.size();
  while (true)
  {
    const auto& [x, y] = coords[index];
    int value = grid[x][y];
    if (one_bit_set(value))
    {
      // prune row
      for (int i = 0; i < grid.size(); ++i)
      {
        if (i != y && grid[x][i] & value)
        {
          grid[x][i] &= ~value;
        }
      }
      // prune column
      for (int i = 0; i < grid.size(); ++i)
      {
        if (i != x && grid[i][y] & value)
        {
          grid[i][y] &= ~value;
        }
      }
      // prune grid cell
      int x_temp2 = x - (x % 3);
      int y_temp2 = y - (y % 3);
      int x_stop = x_temp2 + 3;
      int y_stop = y_temp2 + 3;
      for (int x_temp = x_temp2; x_temp < x_stop; ++x_temp)
      {
        for (int y_temp = y_temp2; y_temp < y_stop; ++y_temp)
        {
          if (x_temp != x && y_temp != y)
          {
            grid[x_temp][y_temp] &= ~value;
          }
        }
      }
      coords.erase(begin(coords) + index); // this also changes x & y values for some reason
    }
    else
    {
      // // prune row
      int possibilities = (1 << 9) - 1;
      for (int i = 0; i < grid.size(); ++i)
      {
        if (i != y)
        {
          possibilities &= ~grid[x][i];
        }
      }
      // if (one_bit_set(possibilities))
      if (possibilities)
      {
        int new_value = grid[x][y] & possibilities;
        if (one_bit_set(new_value)) { grid[x][y] = new_value; }
      }
      // prune column
      possibilities = (1 << 9) - 1;
      for (int i = 0; i < grid.size(); ++i)
      {
        if (i != x)
        {
          possibilities &= ~grid[i][y];
        }
      }
      // if (one_bit_set(possibilities))
      if (possibilities)
      {
        int new_value = grid[x][y] & possibilities;
        if (one_bit_set(new_value)) { grid[x][y] = new_value; }
      }
      // prune grid cell
      possibilities = (1 << 9) - 1;
      int x_temp2 = x - (x % 3);
      int y_temp2 = y - (y % 3);
      int x_stop = x_temp2 + 3;
      int y_stop = y_temp2 + 3;
      for (int x_temp = x_temp2; x_temp < x_stop; ++x_temp)
      {
        for (int y_temp = y_temp2; y_temp < y_stop; ++y_temp)
        {
          if (x_temp != x || y_temp != y)
          {
            possibilities &= ~grid[x_temp][y_temp];
          }
        }
      }
      // if (one_bit_set(possibilities))
      if (possibilities)
      {
        int new_value = grid[x][y] & possibilities;
        if (one_bit_set(new_value)) { grid[x][y] = new_value; }
      }

      ++index;
    }

    if (index == coords.size())
    {
      if (coords.size() == old_size || coords.size() == 0) break;
      old_size = coords.size();
      index = 0;
    }
  }
  // empty domain?
  for (const vector<int>& l : grid)
  {
    for (const int& c : l)
    {
      if (!c) { return false; }
    }
  }
  // select a non-set variable and assign it a value in that domain
  int changes_made = 0;
  int assigned_x = 0;
  int assigned_y = 0;
  bool variable_selected = false;
  for (int x = 0; x < grid.size(); ++x)
  {
    vector<int>& row = grid[x];
    for (int y = 0; y < grid.size(); ++y)
    {
      int& value = row[y];
      if (!one_bit_set(value))
      {
        for (int i = 0; i < grid.size(); ++i)
        {
          if (value & (1 << i))
          {
            assigned_x = x;
            assigned_y = y;
            variable_selected = true;
            changes_made = value & ~(1 << i);
            value = (1 << i);
            goto variable_found;
          }
        }
      }
    }
  }
variable_found:
  // full solution?
  bool solution_found = true;
  for (int x = 0; x < grid.size(); ++x)
  {
    vector<int>& row = grid[x];
    for (int y = 0; y < grid.size(); ++y)
    {
      int& value = row[y];
      if (!one_bit_set(value))
      {
        solution_found = false;
        goto not_full_solution;
      }
    }
  }
not_full_solution:

  if (solution_found)
  {
    for (int x = 0; x < grid.size(); ++x)
    {
      vector<int>& row = grid[x];
      for (int y = 0; y < grid.size(); ++y)
      {
        int& value = row[y];
        grid_solution[x][y] = value;
      }
    }
    return true;
  }
  else
  {
    bool solution = solve_sudoku(grid);
    if (solution)
    {
      for (int x = 0; x < grid.size(); ++x)
      {
        vector<int>& row = grid[x];
        for (int y = 0; y < grid.size(); ++y)
        {
          int& value = row[y];
          grid_solution[x][y] = value;
        }
      }
      return true;
    }
  }

  if (variable_selected)
  {
    grid[assigned_x][assigned_y] = changes_made;
    bool solution = solve_sudoku(grid);
    if (solution)
    {
      for (int x = 0; x < grid.size(); ++x)
      {
        vector<int>& row = grid[x];
        for (int y = 0; y < grid.size(); ++y)
        {
          int& value = row[y];
          grid_solution[x][y] = value;
        }
      }
      return true;
    }
  }

  return false;
}

int main(int argc, char** argv)
{
  string grid = "-2-|---|---|"
                "---|6--|--3|"
                "-74|-8-|---|"
                "-----------|"
                "---|--3|--2|"
                "-8-|-4-|-1-|"
                "6--|5--|---|"
                "-----------|"
                "---|-1-|78-|"
                "5--|--9|---|"
                "---|---|-4-|"
                "-----------";

  // string grid = "-2-|5-1|-9-|"
                // "8--|2-3|--6|"
                // "-3-|-6-|27-|"
                // "-----------|"
                // "--1|---|6--|"
                // "54-|---|-19|"
                // "--2|---|7--|"
                // "-----------|"
                // "-9-|-32|-8-|"
                // "2--|8-4|--7|"
                // "-1-|9-7|-6-|"
                // "-----------";
// 
  // string solution = "426|571|398|"
                    // "857|293|146|"
                    // "139|468|275|"
                    // "-----------|"
                    // "971|385|624|"
                    // "543|726|819|"
                    // "682|149|753|"
                    // "-----------|"
                    // "794|632|581|"
                    // "265|814|937|"
                    // "318|957|462|"
                    // "-----------";
// 
  // string solutio2 = "4 6| 7 |3 8|"
                    // " 57| 9 |14 |"
                    // "1 9|4 8|  5|"
                    // "-----------|"
                    // "97 |385| 24|"
                    // "  3|726|8  |"
                    // "68 |149| 53|"
                    // "-----------|"
                    // "7 4|6  |5 1|"
                    // " 65| 1 |93 |"
                    // "3 8| 5 |4 2|"
                    // "-----------";

  // cout << grid << endl;
  auto sudoku = parse_sudoku(grid);

  cout << "before:" << endl;
  print_sudoku_board(sudoku);
  cout << endl << endl;
  solve_sudoku(sudoku);
  cout << "after:" << endl;
  print_sudoku_board(sudoku);

  return 0;
}

vector<vector<int>> parse_sudoku(string grid)
{
  vector<vector<int>> res;
  for (int i = 0; i < 9; ++i) { res.push_back({}); }
  int index = 0;
  string delimiter = "|";
  size_t pos = 0;
  string temp;
  while ((pos = grid.find(delimiter)) != string::npos)
  {
    if (pos != 11)
    {
      int temp_index = index / 3;
      temp = grid.substr(0, pos);
      for (int i = 0; i < temp.size(); ++i)
      {
        int val;
        if (temp[i] == '-')
        {
          val = (1 << 9) - 1; // all values are possible
        }
        else
        {
          val = temp[i] - '0';
          val = (1 << val - 1);
        }
        res[temp_index].push_back(val);
      }
      ++index;
    }
    grid.erase(0, pos + delimiter.length());
  }
  return res;
}

void print_sudoku_board(vector<vector<int>> sudoku)
{
  vector<int> nums = { -1, 1, 2, 4, 8, 16, 32, 64, 128, 256 };

  vector<int> to_print;
  int index = 0;
  for (vector<int> l : sudoku)
  {
    for (int v : l)
    {
      if (one_bit_set(v))
      {
        for (int i = 1; i <= 9; ++i)
        {
          if (v == nums[i])
          {
            cout << i;
            break;
          }
        }
      }
      else
      {
        cout << '-';
        to_print.push_back(v);
      }
      ++index;
      if (!(index % 3) && (index % 9)) { cout << "|"; }
    }
    cout << '\t';
    for (int v : to_print)
    {
      cout << "{ ";
      for (int i = 1; i <= 9; ++i)
      {
        if (nums[i] & v)
        {
          cout << i << " ";
        }
      }
      cout << "} ";
    }
    to_print = {};

    if (!(index % 27) && index != sudoku.size()*sudoku.size()) { cout << "\n-----------"; }
    cout << endl;
  }
}
