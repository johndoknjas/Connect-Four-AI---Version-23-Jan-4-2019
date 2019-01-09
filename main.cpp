#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <time.h>
#include <cstdlib>
#include <algorithm>
#include <climits>
#include <fstream>
#include <ctime>
#include <ratio>
#include <chrono>

#include "position.h"

using namespace std;

void set_pregame_data(bool& user_goes_first, bool& x_represents_user)
{
    cout << "Type y to go first, and n for the computer to go first: ";

    string user_input = "";

    cin >> user_input;
    cin.clear();
    cin.ignore(INT_MAX, '\n');

    while (user_input != "y" && user_input != "Y" && user_input != "n" && user_input != "N") // invalid data entered...
    {
        cout << "You didn't enter y or n. Please try again: ";

        cin >> user_input;
        cin.clear();
        cin.ignore(INT_MAX, '\n');
    }

    if (user_input == "y" || user_input == "Y")
    {
        user_goes_first = true;
    }

    else
    {
        user_goes_first = false;
    }

    cout << "Type x or o for what piece you want: ";

    cin >> user_input;
    cin.clear();
    cin.ignore(INT_MAX, '\n');

    while (user_input != "o" && user_input != "O" && user_input != "x" && user_input != "X") // invalid data entered...
    {
        cout << "You didn't enter x or o. Please try again: ";

        cin >> user_input;
        cin.clear();
        cin.ignore(INT_MAX, '\n');
    }

    if (user_input == "x" || user_input == "X")
    {
        x_represents_user = true;
    }

    else
    {
        x_represents_user = false;
    }
}

void display_board(vector<vector<char>> board, bool x_represents_user, bool is_starting_position, coordinate last_move)
{
    // First, change the 'C' and 'U' in board to 'X' and 'O', depending on if 'X' or 'O' represents the user...

    for (int row = 0; row <= 5; row++)
    {
        for (int col = 0; col <= 6; col++)
        {
            if (board[row][col] == 'U')
            {
                if (x_represents_user)
                {
                    board[row][col] = 'X';
                }

                else
                {
                    board[row][col] = 'O';
                }
            }

            else if (board[row][col] == 'C')
            {
                if (x_represents_user) // so 'O' represents the computer:
                {
                    board[row][col] = 'O';
                }

                else
                {
                    board[row][col] = 'X';
                }
            }
        }
    }

    // Print who moved, and to where, assuming there is a last move (only not the case when this is the starting position).

    if (!is_starting_position)
    {
        if (last_move.row == position::UNDEFINED)
        {
            throw runtime_error("last_move is UNDEFINED even though this isn't the starting position.\n");
        }

        if ((board[last_move.row][last_move.col] == 'X') == (x_represents_user)) // testing for a logical equivalence between two boolean values.
        {
            cout << "\nYou just moved to square: ";
        }

        else
        {
            cout << "The computer just moved to square: ";
        }

        cout << char('A' + last_move.col) << (6 - last_move.row) << "\n";
    }

    cout << "\n    A   B   C   D   E   F   G\n\n";

    for (int row = 0; row <= 5; row++)
    {
        cout << (6 - row) << " | "; // Since I want row numbers to be displayed increasing from bottom up, not top down.

        for (int col = 0; col <= 6; col++)
        {
            cout << board[row][col] << " | ";
        }

        cout << "\n" << "  |---|---|---|---|---|---|---|\n";
    }
}

void remove_set_at_index(vector<vector<coordinate>>& vec, int index)
{
    if (vec.empty())
    {
        throw runtime_error("vec is empty in remove_set_at_index()\n");
    }

    vec[index] = vec[vec.size()-1];

    vec.pop_back();
}

unique_ptr<position> get_to_chosen_starting_position(bool does_comp_go_first, const vector<coordinate> set_of_moves)
{
    // Get the Engine to play out the set_of_moves param, and only think for a reasonable amount of time
    // on the last move (since that's where the game begins).

    if (set_of_moves.empty())
    {
        throw runtime_error("set_of_moves is empty in get_to_chosen_starting_position()\n");
    }

    const double old_thinking_time = position::thinking_time;

    position::thinking_time = 0; // temporarily reducing it, until reaching the actual starting position for the game.

    // Now, I need to figure out if a 'C' or 'U' should be played first on the empty board.
    // The "does_comp_go_first" param stores who goes first in the actual starting position played, 4-9 moves later.

    bool does_comp_move_first_in_empty_board = true;

    if ((does_comp_go_first && set_of_moves.size() % 2 != 0) || (!does_comp_go_first && set_of_moves.size() % 2 == 0))
    {
        // In either case, 'U' should be the first piece placed on the empty board:

        does_comp_move_first_in_empty_board = false;
    }

    unique_ptr<position> pt = position::think_on_game_position(does_comp_move_first_in_empty_board, true);

    for (int i = 0; i < set_of_moves.size(); i++)
    {
        if (i == set_of_moves.size()-1) // On the move that yields the starting position, so return thinking_time to its original value:
        {
            position::thinking_time = old_thinking_time;
        }

        vector<vector<char>> temp_board = pt->get_board();

        if (pt->get_is_comp_turn())
        {
            temp_board[set_of_moves[i].row][set_of_moves[i].col] = 'C';
        }

        else
        {
            temp_board[set_of_moves[i].row][set_of_moves[i].col] = 'U';
        }

        pt = position::think_on_game_position(temp_board, !pt->get_is_comp_turn(), set_of_moves[i], pt->get_squares_amplifying_comp_2(),
                                              pt->get_squares_amplifying_comp_3(), pt->get_squares_amplifying_user_2(),
                                              pt->get_squares_amplifying_user_3(), true);
                                              // Sending "true" for starting new game since I don't want the TT used. In order to be fair, the comp
                                              // shouldn't be able to use calculations it did to get to the starting position.
    }

    if (position::thinking_time != old_thinking_time)
    {
        throw runtime_error("thinking_time was not reset to its standard value!\n");
    }

    return move(pt);
}

int get_column_user_wants_to_move_in(const unique_ptr<position>& pt)
{
    // Function returns the column the user wants to move in:

    string user_input = "";

    cout << "Enter a column from 'A'-'G' (or 'a'-'g') to move: ";

    cin >> user_input;

    cin.clear();

    cin.ignore(INT_MAX, '\n');

    while (!pt->is_valid_move(user_input))
    {
        cout << "You entered an invalid move. Please try again: ";

        cin >> user_input;

        cin.clear();

        cin.ignore(INT_MAX, '\n');
    }

    // Now I know the user inputted a valid move, so figure out what col they meant:

    char letter = user_input[0];

    if (letter >= 'a') // lowercase:
    {
        return static_cast<int>(letter - 'a');
    }

    else // uppercase:
    {
        return static_cast<int>(letter - 'A');
    }
}

void wait(double waiting_time)
{
    steady_clock::time_point start = steady_clock::now();

    while (duration_cast<duration<double>>(steady_clock::now() - start).count() < waiting_time)
    {
        // Left unfilled deliberately - only purpose is to wait.
    }
}

void play_game(vector<vector<coordinate>>& moves_reaching_starting_positions)
{
    bool user_goes_first = false;
    bool x_represents_user = false;

    set_pregame_data(user_goes_first, x_represents_user);

    // Now to pick a set of moves for the comp and user to play:

    int random_index = rand() % moves_reaching_starting_positions.size();

    vector<coordinate> chosen_set_of_moves = moves_reaching_starting_positions[random_index];

    remove_set_at_index(moves_reaching_starting_positions, random_index); // function will replace the bad set with the last set in the vector
                                                                          // and then pop_back.

    unique_ptr<position> pos = get_to_chosen_starting_position(!user_goes_first, chosen_set_of_moves);

    vector<vector<char>> assisting_board = pos->get_board();

    cout << "\nSTARTING POSITION:\n";

    display_board(assisting_board, x_represents_user, true, {position::UNDEFINED, position::UNDEFINED});
    // UNDEFINED for last_move since this is the starting position.

    if (!user_goes_first) // comp moving first, so want to wait a bit to get the starting position displayed.
    {
       wait(0.8);
    }

    while (!pos->did_computer_win() && !pos->did_opponent_win() && !pos->is_game_drawn()) // while the game is still going on...
    {
        if (pos->get_is_comp_turn()) // computer's turn:
        {
            coordinate best_move = pos->find_best_move_for_comp();
            // This is the move the computer should play in this position.

            assisting_board[best_move.row][best_move.col] = 'C';

            double old_thinking_time = position::thinking_time;

            position::thinking_time = 0;

            pos = position::think_on_game_position(assisting_board, false, best_move, pos->get_squares_amplifying_comp_2(),
                                                   pos->get_squares_amplifying_comp_3(), pos->get_squares_amplifying_user_2(),
                                                   pos->get_squares_amplifying_user_3(), false);

            position::thinking_time = old_thinking_time;

            cout << "\n\n";

            display_board(assisting_board, x_represents_user, false, best_move);
        }

        else // user's turn:
        {
            cout << "\n\n\n";

            int col = get_column_user_wants_to_move_in(pos);

            // Now to make the user's move on the assisting_board:

            int row = position::max_row_index;

            coordinate move_chosen_by_user{position::UNDEFINED, position::UNDEFINED};

            while (row >= 0)
            {
                if (assisting_board[row][col] == ' ') // empty square... move a piece here:
                {
                    assisting_board[row][col] = 'U';

                    move_chosen_by_user.row = row;

                    move_chosen_by_user.col = col;

                    break;
                }

                row --;
            }

            if (row < 0 || move_chosen_by_user.row == position::UNDEFINED)
            {
                throw runtime_error("row variable in play_game() is smaller than 0, or there is no move chosen by the user recorded.\n");
            }

            display_board(assisting_board, x_represents_user, false, move_chosen_by_user);

            cout << "\n";

            pos = position::think_on_game_position(assisting_board, true, move_chosen_by_user, pos->get_squares_amplifying_comp_2(),
                                                   pos->get_squares_amplifying_comp_3(), pos->get_squares_amplifying_user_2(),
                                                   pos->get_squares_amplifying_user_3(), false);
        }
    }

    cout << "\n\n\n";

    // At this point, the game has ended. I should display the winner:

    if (pos->did_computer_win())
    {
        cout << "The computer won!\n\n";
    }

    else if (pos->did_opponent_win())
    {
        cout << "You won!\n\n";
    }

    else
    {
        cout << "The game is a draw!\n\n";
    }
}

void read_file_into_vector(vector<vector<coordinate>>& vec)
{
    ifstream fin("MovesReachingPositions.txt");

    if (fin.fail())
    {
        throw runtime_error("fin failed to read file in read_file_into_vector() function.\n");
    }

    coordinate current_move_found;
    current_move_found.row = position::UNDEFINED;
    current_move_found.col = position::UNDEFINED;

    vector<coordinate> empty_vec;

    vec.push_back(empty_vec);

    char c = fin.get();

    while (!fin.eof())
    {
        if (c - '0' >= 0 && c - '0' <= 9) // is a digit, so work with it...
        {
            if (current_move_found.row == position::UNDEFINED)
            {
                current_move_found.row = c - '0';
            }

            else
            {
                current_move_found.col = c - '0';

                vec[vec.size()-1].push_back(current_move_found);

                // Now reset current_move_found:

                current_move_found.row = position::UNDEFINED;
                current_move_found.col = position::UNDEFINED;
            }
        }

        else if (c == '\n')
        {
            // About to be a new line in the file (representing a set of moves), so create a new spot in the param vector:

            vec.push_back(empty_vec);

            // Note that if that '\n' was the last newline character in the file, then vec's last element will be an empty vector.

            // So, I take care of this near the end of the function.
        }

        else if (c != '(' && c != ')' && c != ',' && c != ' ')
        {
            // c doesn't equal any of the other allowable characters in the file. Something's wrong:

            throw runtime_error("Found an unexpected character in the file in read_file_into_vector()\n");
        }

        c = fin.get();
    }

    if (vec[vec.size()-1].size() == 0)
    {
        // The last set of moves in vec is empty, which makes sense.

        vec.pop_back();
    }
}

bool are_all_moves_valid(const vector<vector<coordinate>>& vec)
{
    for (const vector<coordinate>& current_set: vec)
    {
        if (current_set.empty() || current_set.size() < 4 || current_set.size() > 9)
        {
            return false;
        }

        for (const coordinate& current_move: current_set)
        {
            if (current_move.row < 0 || current_move.row > 5 || current_move.col < 0 || current_move.col > 6)
            {
                return false;
            }
        }
    }

    return true;
}

int main()
{
    srand(time(NULL));

    cout << "Enter approximately how long you want the Engine to think on each move: ";

    cin >> position::thinking_time;

    vector<vector<coordinate>> moves_reaching_starting_positions; // Will store all the sets of moves reaching starting positions (all fair!).

    read_file_into_vector(moves_reaching_starting_positions); // reads the file I have containing these sets of moves.

    if (!are_all_moves_valid(moves_reaching_starting_positions))
    {
        throw runtime_error("Found invalid move(s) in the moves_reaching_starting_positions vector in main()\n");
    }

    char user_input = ' ';

    cout << "To play, press 1 and enter: ";

    cin >> user_input;

    while (user_input == '1')
    {
        if (moves_reaching_starting_positions.empty()) // Somehow all starting positions have been played out, so read in file again...
        {
            read_file_into_vector(moves_reaching_starting_positions);
        }

        play_game(moves_reaching_starting_positions);

        cout << "To play again, press 1 and enter: ";

        cin >> user_input;
    }
}

