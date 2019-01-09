#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <ratio>
#include <chrono>
#include <algorithm>
#include <random>
#include <climits>
#include <cmath>
#include "tool.h"

using namespace std;

using namespace std::chrono;

struct coordinate
{
    int row;
    int col;
};

struct treasure_spot
{
    // stores the coordinate of a square that, if filled, makes a 3-in-a-row or 4-in-a-row
    // Also stores the coordinates of the two squares that would extend the newly formed hypothetical line of pieces
        // (which is formed after current_square gets filled).

    coordinate current_square;
    coordinate next_square;
    coordinate other_next_square; // Version 19!
};

struct position_info_for_TT // The key info of a position that will be stored in the transposition table.
{
    vector<vector<char>> board; // The position's 2-D vector of char board. Acts as the KEY!
    int evaluation;
    int calculation_depth_from_this_position; // stores how far ahead the computer calculated for getting the position's evaluation.
    vector<coordinate> possible_moves_sorted; // stores the position's possible moves, sorted from probable best to probable worst.
    bool is_evaluation_indisputable; // stores true if there this position's evaluation will not change by calculating deeper...
                                     // someone has a forced win/forced draw. Someone could have just won/drawn in this position too.
    bool is_comp_turn; // stores true if it's the computer's turn in the position.
};

struct coordinate_and_value // Object for storing amplifying squares along with their respective values.
{
    coordinate square; // stores the coordinates of the amplifying square.
    int value; // stores the value of the amplifying square, determined by the heuristics in smart_evaluation() and the function it calls.
};

bool operator==(const coordinate& first, const coordinate& second) // function tests for equality between two coordiate objects
{
    return (first.row == second.row && first.col == second.col);
}

class position : public tool
{
public:
    // Constructors:

    // PROGRAMMER CALLS TO START THE GAME.
    position(bool is_comp_turnP);

    // PROGRAMMER CALLS WHEN COMP/USER MAKES A MOVE IN GAME.
    position(const vector <vector<char>>& boardP, bool is_comp_turnP, coordinate last_moveP,
             const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
             const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P);

    // COMPUTER CALLS RECURSIVELY IN ITS MINIMAX CALCULATIONS.
    position(const vector <vector<char>>& boardP, bool is_comp_turnP,
             int depthP, int number_of_piecesP, coordinate last_moveP,
             const vector<coordinate>& possible_movesP, int possible_moves_index,
             int alphaP, int betaP,
             const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
             const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P,
             double pre_hash_value_of_positionP, const vector<int>& num_pieces_per_columnP);
    // No param for evaluation is sent to constructor, as this is figured out by the computer via minimax.
    // No param for future_positions is sent to constructor, as this is figured out by the computer via minimax.

    // Getters:
    vector <vector<char>> get_board() const;
//    vector <unique_ptr<position>> get_future_positions() const;
    int get_evaluation() const;
    bool get_is_comp_turn() const;
    int get_depth() const;
    unique_ptr<position> get_a_future_position(int i); // MOVES the position object at index i of future_positions and returns!
    vector <unique_ptr<position>> get_future_positions(); // MOVES the future_positions vector and returns it!
    int get_future_positions_size() const;
    coordinate get_last_move() const;
    coordinate find_best_move_for_comp(); // finds the best move to play in this current position, for the comp, and returns.
                                          // This function finds the move the comp should play against user in the game.
    vector<treasure_spot> get_squares_amplifying_comp_2() const; // returns the squares_amplifying_comp_2 vector.
    vector<treasure_spot> get_squares_amplifying_comp_3() const; // returns the squares_amplifying_comp_3 vector.
    vector<treasure_spot> get_squares_amplifying_user_2() const; // returns the squares_amplifying_user_2 vector.
    vector<treasure_spot> get_squares_amplifying_user_3() const; // returns the squares_amplifying_user_3 vector.

    // Setters:
    void set_board(const vector <vector<char>>& boardP);
//    void set_future_positions(const vector<unique_ptr<position>>& future_positionsP);
    void set_evaluation (int evalP);
    void set_is_comp_turn (bool turnP);
    void set_depth(int depthP);
    void set_future_positions_size(int i);
    void randomize_order_of_possible_moves(); // randomizes the order of elements in the possible_moves vector.
    void clean_up_amplifying_vectors(); // removes all elements in the 4 amplifying vectors that are no longer ' ' chars.
    void rearrange_possible_moves(const vector<coordinate>& front_moves); // puts the moves in front_moves at the front of
                                                                          // the possible_moves vector of the calling object.
                                                                          // All these moves should already be in possible_moves.
    void initialize_hash_value_of_position(); // multiplies the pre_hash_value_of_position to >= 1,000,000, rounds to an int, and
                                              // sets hash_value_of_position to the result.

    void add_position_to_transposition_table(bool is_evaluation_indisputable);
    // Adds this position's board (the key) and evaluation to the appropriate index in
    // the static transposition table (i.e., the hash_value of the position).

    // Helpers:
    bool did_computer_win() const; // returns true if the computer has won the game with a 4-in-a-row in the current position.
    bool did_opponent_win() const; // returns true if the user has won the game with a 4-in-a-row in the current position.
    bool is_game_drawn() const; // returns true if the game is drawn in the current position.
    bool evaluation_in_future_positions(int eval) const; // returns true if at least 1 future position has the evaluation value
                                                         // of the eval param.
    bool is_valid_move(string column) const; // Checks if the move entered by the user is valid.

    void remove_treasure_spot_objects_from_vector(vector<treasure_spot>& vec);
    // removes the treasure_spot objects from vec whose current_square attribute DOESN'T store
    // the location of an empty square in board (i.e., isn't ' ').

    void print_amplifying_vectors(); // prints the contents of all 4 amplifying vectors to the screen (this is just for testing!)

    void find_critical_moves(vector<coordinate>& critical_moves); // Fills vector with moves that can be played now, which
                                                                  // lead to a 4-in-a-row.

    void find_critical_moves_in_amplifying_vector(vector<coordinate>& critical_moves,
                                                  const vector<treasure_spot>& amplifying_vector,
                                                  bool are_3_pieces, char piece);
    // Fills critical_moves vector with moves/squares in the amplifying vector that can be played now, which lead to a
    // 4-in-a-row.
    // bool are_3_pieces param stores true if the amplifying vector is for squares amplifying 3-in-a-rows, and false if
    // it is for 2-in-a-rows.
    // char piece param is simply the type of char ('C' or 'U') being dealt with.

    bool is_in_bounds(const coordinate& square) const; // returns true if square is in-bounds in board.

    bool in_coordinate_vector(const vector<coordinate>& vec, const coordinate& element); // returns true if element is in vec.

    void remove_duplicates(vector<coordinate>& vec); // Removes duplicate elements from the vector.

    void initialize_row_barriers(); // Initializes row_barriers (private member vector) with 7 elements.
                                    // 1st element is row index of highest row (visually) allowed in column 0.

    void find_winning_squares(vector<coordinate>& vec, const vector<treasure_spot>& squares_amplifying_3,
                              const vector<treasure_spot>& squares_amplifying_2, char piece);
    // Function finds squares making a 4-in-a-row of piece and stores them in vec.

    // Functions specifically designed for, and only used in, the Versus Sim:

    double get_static_thinking_time() const;

    void set_static_thinking_time(double val);

    unique_ptr<tool> call_static_think_on_game_position(const vector <vector<char>>& boardP, bool is_comp_turnP,
                                                        coordinate last_moveP, const vector<treasure_spot>& squares_amplifying_comp_2P,
                                                        const vector<treasure_spot>& squares_amplifying_comp_3P,
                                                        const vector<treasure_spot>& squares_amplifying_user_2P,
                                                        const vector<treasure_spot>& squares_amplifying_user_3P, bool starting_new_game);

    unique_ptr<tool> call_static_think_on_game_position(bool is_comp_turnP, bool starting_new_game);

    coordinate_and_value find_quick_winning_move(int max_number_moves_acceptable) const;

    coordinate return_a_move_that_wins_immediately() const;


    // Public static variables:
    static const int UNDEFINED; // used when evaluation, alpha, or beta is unknown.
    static const int max_row_index; // the max row index of board (i.e., 5, since there are 6 rows).
    static const int max_col_index; // the max col index of board (i.e., 6, since there are 7 columns).
    static int depth_limit; // the depth of the computer's calculation abilities.
    static const double PI; // PI to 11 decimal places.

    static const vector<vector<double>> hash_values_of_squares_with_C; // stores the double hash value of each square in the board, if it stores 'C'.
    static const vector<vector<double>> hash_values_of_squares_with_U; // stores the double hash value of each square in the board, if it stores 'U'.
    static const vector<vector<double>> hash_values_of_squares_empty; // stores the double hash value of each square in the board, if it stores ' '.

    static vector<vector<position_info_for_TT>> transposition_table; // A position's key & evaluation get stored here, at the appropriate
                                                                     // index (i.e., it's hash value). The inner vector is to deal with possible
                                                                     // collisions. Multiple positions can be stored at the same
                                                                     // index in the outer vector via the inner vector.

    static vector<int> indices_of_elements_in_TT; // Stores the indices of which inner vectors in the TT actually store data.

    static int counter; // counts how many times the position class is instantiated. PURELY FOR TESTING!
    static int counter_of_TT_usefulness; // counts how many times the TT is actually useful. PURELY FOR TESTING!

    static double thinking_time; // Comp spends this long thinking, plus the time it spends on the last iteration of the
                                 // iterative deepening while loop.

    static vector<treasure_spot> empty_amplifying_vector;

    // Public static methods:

    static vector<vector<double>> find_hash_values_for_all_squares_in_board(char piece);
    // Returns a 7x6 2-D vector of doubles, storing the hash value for each square (assuming char piece is in the square).

    static double cotangent_with_degrees(double angle_in_degrees); // Returns the cotangent of the angle in degrees.

    static void reset_transposition_table(); // Resets the transposition table to only store empty inner vectors.
                                             // Uses the indices_of_elements_in_TT vector to do this resetting task efficiently.
                                             // Also, this function will make the indices_of_elements_in_TT vector then be empty.

    static bool compare_future_positions_by_evaluation(const unique_ptr<position>& first_pos, const unique_ptr<position>& second_pos);
    // Function returns true if first_pos would be better than second_pos for the player. This of course depends on
    // on whose turn it is in the calling object/position, which has to be determined by looking at first_pos or second_pos,
    // since this is a static function.

    static position_info_for_TT find_duplicate_in_TT(const unique_ptr<position>& pt);
    // Searches through the TT for a duplicate of pt, and returns it.

    static unique_ptr<position> think_on_game_position(const vector <vector<char>>& boardP, bool is_comp_turnP, coordinate last_moveP,
                                    const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
                                    const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P,
                                    bool starting_new_game);
    // Function thinks and returns a unique_ptr that points to a position object of all its calculations.
    // THIS FUNCTION SHOULD BE THE ONE CALLED FROM MAIN WHENEVER THE ENGINE NEEDS TO THINK! Or the function below for starting a new game...

    static unique_ptr<position> think_on_game_position(bool is_comp_turnP, bool starting_new_game);

private:
    // Private variables:
    vector <vector<char>> board; // stores C's and U's and ' ', representing the computer and user's pieces and empty squares.
    bool is_comp_turn; // stores true if it's the computer's turn, and false if it's the user's turn.
    int depth; // stores how deep this position is in the computer's calculations.
    int number_of_pieces; // stores how many pieces are in the current board. Only 'C' and 'U' count as pieces (not ' ').
    coordinate last_move; // last_move, an object of the coordinate struct, stores the row and col of the last move (made in
                          // the previous node).
    vector <coordinate> possible_moves; // stores all the possible moves in this current position. Usually there are 7.
    int alpha; // stores the best alternative found so far FOR THE COMPUTER at this time in the entire search. (i.e., highest val).
    int beta; // stores the best alternative found so far FOR THE USER at this time in the entire search (i.e., lowest val).
    int evaluation; // stores -1 if the computer is losing, 0 if the game is drawn, and +1 if the computer is winning.
    int future_positions_size; // stores how many positions are in the future_positions vector.

    vector <unique_ptr<position>> future_positions; // stores pointers to all future positions one move ahead.
    // stored as pointers in order to be efficient with memory, as position objects are huge.

    vector<treasure_spot> squares_amplifying_comp_2; // squares that, if filled, turn the comp's 2-in-a-row into a 3-in-a-row.
    vector<treasure_spot> squares_amplifying_comp_3; // squares that, if filled, turn the comp's 3-in-a-row into a 4-in-a-row.
    vector<treasure_spot> squares_amplifying_user_2; // squares that, if filled, turn the user's 2-in-a-row into a 3-in-a-row.
    vector<treasure_spot> squares_amplifying_user_3; // squares that, if filled, turn the user's 3-in-a-row into a 4-in-a-row.

    vector<int> row_barriers; // stores the highest (visually) rows allowed for play in each column of the board,
                              // due to a square allowing both comp AND user to win (this is "finished column" algorithm).
                              // row_barriers[0] stores highest row (visually) allowed for play in column 0.
                              // row_barriers[6] stores highest row (visually) allowed for play in column 6.
                              // NOTE: This member is only initialized in smart_evaluation(), aka when depth limit is reached,
                              // since it has no use before then.

    double pre_hash_value_of_position; // stores the double value the position's hash value has, before being multiplied to >= 1,000,000 and
                                       // rounded to an int. Pass this variable on to child nodes! Since then they only have to deal with
                                       // the 'C' or 'U' replacing the ' ' at last_move's coordinates in board.

    int hash_value_of_position; // equal to the result of the above variable being multiplied to >= 1,000,000 and rounded to an int.

    bool is_a_pruned_branch; // Initialized to false - stores true if this node (and all its children) gets pruned via alpha-beta pruning.

    bool got_value_from_pruned_child; // Initialized to false - stores true if this node's final evaluation value came from
                                      // a child that got pruned (which means the child's value isn't accurate). To know
                                      // if a child got pruned, refer to the child's bool variable above ("is_a_pruned_branch").

    vector<int> num_pieces_per_column; // index 0 stores number of pieces in column 0. All the way up to...
                                       // index 6 stores number of pieces in column 6.

    int calculation_depth_from_this_position; // stores how many moves ahead the comp will calculate from this current position.

    // Private methods:
    void analyze_last_move(); // analyzes the last move to see if anyone won and to add anything to the above 4 vectors
                              // storing squares that allow 3-in-a-rows or 2-in-a-rows to be amplifyed.
    void analyze_horizontal_perspective_of_last_move(); // is the horizontal perspective of "analyze_last_move()".
    void analyze_vertical_perspective_of_last_move(); // is the vertical perspective of "analyze_last_move()".
    void analyze_positive_slope_diagonal_perspective_of_last_move(); // is the +slope diagonal perspective of
                                                                     // "analyze_last_move()".
    void analyze_negative_slope_diagonal_perspective_of_last_move(); // is the -slope diagonal perspective of
                                                                     // "analyze_last_move()".
    void add_to_appropriate_amplifying_vector(int num_pieces_in_a_row, treasure_spot empty_square);
    // function adds empty_square to one of the four amplifying vectors, depending on num_pieces_in_a_row and whose turn it is.
    void minimax(); // Employs the minimax algorithm...
                    // fills the future_positions vector with all positions one move ahead.
                    // eventually gives the evaluation attribute a value.
    void smart_evaluation(); // evaluates the position at depth_limit, if no one has won. Gives the evaluation attribute a value.
    void find_individual_player_evaluation(const vector<treasure_spot>& squares_amplifying_3,
                                          const vector<treasure_spot>& squares_amplifying_2, char piece,
                                          vector<vector<char>>& copy_board, vector<coordinate_and_value>& recorder) const;
                                           // Goes through the amplifying vectors, and evaluates each unique square.
                                           // The coordinates and value for each unique amplifying square are stored in recorder,
                                           // which is passed by reference.
                                           // All squares making a 4-in-a-row are given an 'A' in copy_board, which is also passed by reference.
    bool did_someone_win() const; // returns true if there is a 4-in-a-row in board, meaning someone won.
    bool horizontal_four_combo() const; // returns true if there is a horizontal 4-in-a-row in board.
    bool vertical_four_combo() const; // returns true if there is a vertical 4-in-a-row in board.
    bool positive_slope_diagonal_four_combo() const; // returns true if there is a positive slope diagonal 4-in-a-row in board.
    bool negative_slope_diagonal_four_combo() const; // returns true if there is a negative slope diagonal 4-in-a-row in board.
    bool is_acceptable_letter(char c) const; // returns true if char c is a letter from a-g (uppercase OR lowercase).
    bool is_element_in_vector(const vector<vector<vector<char>>>& vec, const vector<vector<char>>& element) const;
    // returns true if element is in vector vec. *Note*: vec is just a vector storing boards (i.e., 2D vectors of chars),
    // and element is a board (not necessarily the private board attribute of the calling object though!).
    coordinate find_starting_horizontal_point() const; // finds left-most connected square from last_move.
    coordinate find_ending_horizontal_point() const; // finds right_most connected square from last_move.
    coordinate find_starting_vertical_point() const; // finds the top-most connected square from last_move.
    coordinate find_ending_vertical_point() const; // finds the bottom-most connected square from last_move.
    coordinate find_starting_positive_slope_diagonal_point() const; // finds the bottom-left-most connected square from last_move.
    coordinate find_ending_positive_slope_diagonal_point() const; // finds the top_right-most connected sqaure from last_move.
    coordinate find_starting_negative_slope_diagonal_point() const; // finds the top-left-most connected square from last_move.
    coordinate find_ending_negative_slope_diagonal_point() const; // finds the bottom-right-most connected square from last_move.
};

// Initializing the static variables:

const int position::UNDEFINED = INT_MAX - 1; // just a random value.
const int position::max_row_index = 5;
const int position::max_col_index = 6;
int position::depth_limit = 1; // starts off at 1 every time the Engine thinks (iterative deepening).
const double position::PI = 3.14159265359;

const vector<vector<double>> position::hash_values_of_squares_with_C = find_hash_values_for_all_squares_in_board('C');
const vector<vector<double>> position::hash_values_of_squares_with_U = find_hash_values_for_all_squares_in_board('U');
const vector<vector<double>> position::hash_values_of_squares_empty = find_hash_values_for_all_squares_in_board(' ');

vector<vector<position_info_for_TT>> position::transposition_table(1000005);
vector<int> position::indices_of_elements_in_TT;

int position::counter = 0;
int position::counter_of_TT_usefulness = 0;

double position::thinking_time = 0.30;

vector<treasure_spot> position::empty_amplifying_vector;

// CONSTRUCTORS:

position::position(bool is_comp_turnP)
{
    // INITIALIZE BOARD:

    vector <char> row;

    for (int i = 0; i <= max_col_index; i++)
    {
        row.push_back(' ');
    }

    for (int i = 0; i <= max_row_index; i++)
    {
        board.push_back(row);
    }

    // Initialize num_pieces_per_column:

    for (int col = 0; col <= max_col_index; col++)
    {
        num_pieces_per_column.push_back(0);
    }

    is_comp_turn = is_comp_turnP;

    depth = 0;

    calculation_depth_from_this_position = depth_limit - depth;

    number_of_pieces = 0;

    last_move.row = UNDEFINED;
    last_move.col = UNDEFINED;

    // Giving last_move.row and last_move.col the UNDEFINED value, since this constructor is for starting the entire game.
    // No one has made a move yet.
    // I'll just not call any of the methods which check if the game has ended (it's impossible for it to end when it starts!).
    // The methods that check if someone won rely heavily on last_move.

    // INITIALIZE POSSIBLE_MOVES VECTOR:

    for (int c = 0; c <= max_col_index; c++)
    {
        coordinate temp;
        temp.col = c;
        temp.row = max_row_index;
        possible_moves.push_back(temp);
    }
 //   randomize_order_of_possible_moves();   FINALLY TAKING OUT RANDOMNESS
 // Besides, in most cases possible_moves will just be set to a possible moves vector from an earlier duplicate in the hash table.

    alpha = UNDEFINED;

    beta = UNDEFINED;

    evaluation = UNDEFINED;

    future_positions_size = 0;

    // Now figure out the pre-hash value of the position. All the squares store ' ', so it's easy: just sum all the entries in
    // the hash_values_of_squares_empty static vector:

    pre_hash_value_of_position = 0.0;

    for (int row = 0; row <= max_row_index; row++)
    {
        for (int col = 0; col <= max_col_index; col++)
        {
            pre_hash_value_of_position += hash_values_of_squares_empty[row][col]; // since this vector has same dimensions as board... 7x6.
        }
    }

    initialize_hash_value_of_position(); // uses pre_hash_value_of_position above to get an int >= 1,000,000 to set hash_value_of_position to.

    // Now, I don't need to check if someone won, since this constructor starts the entire game.
    // I also don't need to call the analyze_last_move() function, since there is no last_move yet!

    is_a_pruned_branch = false;
    got_value_from_pruned_child = false;

    // So, call minimax() now:

    minimax();

    unique_ptr<position> temp;
}

position::position(const vector <vector<char>>& boardP, bool is_comp_turnP, coordinate last_moveP,
                   const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
                   const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P)
{
    board = boardP;

    is_comp_turn = is_comp_turnP;

    depth = 0;

    calculation_depth_from_this_position = depth_limit - depth;

    // FIGURE OUT NUMBER_OF_PIECES, USING AN EMBEDDED FOR LOOP TO RUN THROUGH THE ENTIRE BOARD.

    number_of_pieces = 0;

    for (int row = 0; row <= max_row_index; row++)
    {
        for (int col = 0; col <= max_col_index; col++)
        {
            if (board[row][col] != ' ')
            {
                number_of_pieces ++;
            }
        }
    }

    // Initialize num_pieces_per_column:

    for (int col = 0; col <= max_col_index; col++)
    {
        num_pieces_per_column.push_back(0);
    }

    // Now figure out the number of pieces in each column, to initialize the num_pieces_per_column vector:

    for (int col = 0; col <= max_col_index; col++)
    {
        for (int row = 0; row <= max_row_index; row++)
        {
            if (board[row][col] != ' ')
            {
                num_pieces_per_column[col] ++;
            }
        }
    }

    last_move = last_moveP;

    // INITIALIZE THE POSSIBLE_MOVES VECTOR.
    // FIGURE OUT ALL POSSIBLE MOVES IN THIS POSITION.
    // THEN, RANDOMIZE THEIR ORDER IN THE VECTOR.

    for (int col = 0; col <= max_col_index; col++)
    {
        for (int row = max_row_index; row >= 0; row--)
        {
            if (board[row][col] == ' ') // found the legal move in this column:
            {
                coordinate temp;
                temp.row = row;
                temp.col = col;
                possible_moves.push_back(temp);
                break; // move on to next column via the outer for loop, since I found the one legal move of this column.
            }
        }
    }
 //   randomize_order_of_possible_moves();

    alpha = UNDEFINED; // just some random value to signify that there is no alpha value yet.

    beta = UNDEFINED; // just some random value to signify that there is no beta value yet.

    evaluation = UNDEFINED; // just some random value to signify that there is no evaluation value yet.

    future_positions_size = 0;

    squares_amplifying_comp_2 = squares_amplifying_comp_2P;

    squares_amplifying_comp_3 = squares_amplifying_comp_3P;

    squares_amplifying_user_2 = squares_amplifying_user_2P;

    squares_amplifying_user_3 = squares_amplifying_user_3P;

    clean_up_amplifying_vectors(); // gets rid of obsolete treasure_spot objects in the 4 amplifying vectors,
                                   // making it faster for comp to evaluate in its calculations later at depth_limit.
                                   // Note that I can call this clean-up function in this constructor (2nd), since this
                                   // constructor is only called once per move from main.cpp. The 3rd constructor is
                                   // called thousands of times by the computer during minimax, so I will not do
                                   // clean-up there (not efficient!).

    // Now figure out the pre-hash value of the position:

    pre_hash_value_of_position = 0.0;

    for (int row = 0; row <= max_row_index; row++)
    {
        for (int col = 0; col <= max_col_index; col++)
        {
            if (board[row][col] == ' ')
            {
                pre_hash_value_of_position += hash_values_of_squares_empty[row][col];
            }

            else if (board[row][col] == 'C')
            {
                pre_hash_value_of_position += hash_values_of_squares_with_C[row][col];
            }

            else // stores 'U':
            {
                pre_hash_value_of_position += hash_values_of_squares_with_U[row][col];
            }
        }
    }

    initialize_hash_value_of_position(); // uses pre_hash_value_of_position above to get an int >= 1,000,000 to set hash_value_of_position to.

    is_a_pruned_branch = false;
    got_value_from_pruned_child = false;

    analyze_last_move(); // will analyze the last_move, and then call minimax() if the game isn't over.
}

position::position(const vector <vector<char>>& boardP, bool is_comp_turnP,
                   int depthP, int number_of_piecesP, coordinate last_moveP,
                   const vector<coordinate>& possible_movesP, int possible_moves_index,
                   int alphaP, int betaP,
                   const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
                   const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P,
                   double pre_hash_value_of_positionP, const vector<int>& num_pieces_per_columnP)
{
    board = boardP;
    is_comp_turn = is_comp_turnP;
    depth = depthP;
    calculation_depth_from_this_position = depth_limit - depth;
    number_of_pieces = number_of_piecesP;
    last_move = last_moveP;

    num_pieces_per_column = num_pieces_per_columnP;
    num_pieces_per_column[last_move.col] ++;

    possible_moves = possible_movesP;
    possible_moves[possible_moves_index].row --;
    if (possible_moves[possible_moves_index].row == -1)
    {
        possible_moves.erase(possible_moves.begin() + possible_moves_index);
    }
 //   randomize_order_of_possible_moves();

    alpha = alphaP;
    beta = betaP;
    evaluation = UNDEFINED; // just some random value to signify there is no evaluation value yet.
    future_positions_size = 0;

    squares_amplifying_comp_2 = squares_amplifying_comp_2P;

    squares_amplifying_comp_3 = squares_amplifying_comp_3P;

    squares_amplifying_user_2 = squares_amplifying_user_2P;

    squares_amplifying_user_3 = squares_amplifying_user_3P;

    pre_hash_value_of_position = pre_hash_value_of_positionP;

    // But now, pre_hash_value_of_position must be adjusted to account for a 'C' or 'U' being at last_move's coordinates in board, instead of ' ':

    pre_hash_value_of_position -= hash_values_of_squares_empty[last_move.row][last_move.col];

    if (board[last_move.row][last_move.col] == 'C')
    {
        pre_hash_value_of_position += hash_values_of_squares_with_C[last_move.row][last_move.col];
    }

    else // 'U' at last_move's coordinates:
    {
        pre_hash_value_of_position += hash_values_of_squares_with_U[last_move.row][last_move.col];
    }

    initialize_hash_value_of_position(); // uses pre_hash_value_of_position above to get an int >= 1,000,000 to set hash_value_of_position to.

    is_a_pruned_branch = false;
    got_value_from_pruned_child = false;

    analyze_last_move(); // will analyze the last_move, and then call minimax() if the game isn't over.
}

// GETTERS:

vector <vector<char>> position::get_board() const
{
    return board;
}

int position::get_evaluation() const
{
    return evaluation;
}

bool position::get_is_comp_turn() const
{
    return is_comp_turn;
}

int position::get_depth() const
{
    return depth;
}

unique_ptr<position> position::get_a_future_position(int i)
{
    return move(future_positions[i]);
}

vector <unique_ptr<position>> position::get_future_positions()
{
    return move(future_positions);
}

int position::get_future_positions_size() const
{
    return future_positions_size;
}

coordinate position::get_last_move() const
{
    return last_move;
}

coordinate position::find_best_move_for_comp()
{
    if (depth_limit != 1)
    {
        throw runtime_error("depth limit does not equal 1 in find_best_move_for_comp()");
    }

    // This function should return the last_move attribute of the best future_position.
    // The problem is choosing which is the best future_position.

    // First, it's possible that the calling object could have an empty future_positions vector since the calling object
    // automatically accepted an indisputable evaluation from a duplicate in the TT.
        // If this is the case, just access this duplicate position from the TT and pick the best of its possible moves.

    if (future_positions.empty())
    {
        // This position must have a forced win, but it got its evaluation immediately from the TT and doesn't have a future positions vector.

        // So, create a new search right here with depth_limit = 1 or 2. The TT will be used in the search process implicitly, as it
        // would normally.

        // Then, move the new pointer being created here into the future_positions vector.

        for (const coordinate& current: possible_moves)
        {
            vector<vector<char>> copy_board = board;

            copy_board[current.row][current.col] = 'C';

            unique_ptr<position> pt = make_unique<position>(copy_board, !is_comp_turn, current,
                                                            squares_amplifying_comp_2, squares_amplifying_comp_3,
                                                            squares_amplifying_user_2, squares_amplifying_user_3);
                // NOTE: The static method for returning a unique_ptr isn't being called here, as I don't care
                // about iterative deepening. I know there's a forced win, and the TT will be used at depth = 1.

            future_positions.push_back(move(pt));
        }
    }

    if (evaluation == INT_MAX) // Comp is winning, so see if there's a solution to win in <= 9 moves:
    {
        coordinate_and_value quick_winning_move = find_quick_winning_move(9); // UNDEFINED returned for value field if no solution in <= 9 moves.

        if (quick_winning_move.value != UNDEFINED)
        {
            return quick_winning_move.square;
        }
    }

    if (evaluation == INT_MIN) // Comp is losing, so find the move that makes the comp work the longest, or at least work for 7 moves.
    {
        // Translation: Is there a comp move such that find_quick_winning_move(7) FOR USER returns UNDEFINED for the value field?
        // If not, pick the move that survives the longest.

        coordinate_and_value most_stubborn_defense; // coordinate stores the move, and value stores how many moves the user will need to win after.
        most_stubborn_defense.value = 1; // Starting off with the worst possible value to have - it says the user wins right on the spot.
        most_stubborn_defense.square = {UNDEFINED, UNDEFINED};

        for (const coordinate& current_move: possible_moves)
        {
            vector<vector<char>> assisting_board = board;

            assisting_board[current_move.row][current_move.col] = 'C';

            unique_ptr<position> pt = make_unique<position>(assisting_board, !is_comp_turn, current_move, empty_amplifying_vector,
                                                            empty_amplifying_vector, empty_amplifying_vector, empty_amplifying_vector);

            int number_of_moves_user_wins_in = pt->find_quick_winning_move(7).value;

            if (number_of_moves_user_wins_in == UNDEFINED) // user will have to work for > 7 moves, so pick this option immediately!
            {
                return current_move;
            }

            else if (most_stubborn_defense.square.row == UNDEFINED || most_stubborn_defense.value < number_of_moves_user_wins_in)
            {
                most_stubborn_defense.value = number_of_moves_user_wins_in;
                most_stubborn_defense.square = current_move;
            }
        }

        if (most_stubborn_defense.square.row == UNDEFINED)
        {
            throw runtime_error("No possible moves?\n");
        }

        return most_stubborn_defense.square;
    }

    // Randomly pick a move with the same evaluation as the calling position object.

    vector <int> indices; // will store all the possible indices of future_positions vector.

    for (int i = 0; i < future_positions.size(); i++)
    {
        indices.push_back(i);
    }

    // Shuffle indices vector:

    auto rng = default_random_engine {};
    shuffle(begin(indices), end(indices), rng);

    for (int index: indices) // index is the current ELEMENT in indices, and acts as an INDEX for the future_positions vector.
    {
        if (future_positions[index]->evaluation >= evaluation)
        {
            return (future_positions[index]->last_move);
        }
    }

    // Control should NEVER reach this point, since there MUST be an acceptable move that should have been returned already.

    throw runtime_error("Error - Control reached the end of the function in position::find_best_move_for_comp()\n");
}

vector<treasure_spot> position::get_squares_amplifying_comp_2() const
{
    return squares_amplifying_comp_2;
}

vector<treasure_spot> position::get_squares_amplifying_comp_3() const
{
    return squares_amplifying_comp_3;
}

vector<treasure_spot> position::get_squares_amplifying_user_2() const
{
    return squares_amplifying_user_2;
}

vector<treasure_spot> position::get_squares_amplifying_user_3() const
{
    return squares_amplifying_user_3;
}

// SETTERS:

void position::set_board(const vector <vector<char>>& boardP)
{
    board = boardP;
}

void position::set_evaluation (int evalP)
{
    evaluation = evalP;
}

void position::set_is_comp_turn (bool turnP)
{
    is_comp_turn = turnP;
}

void position::set_depth(int depthP)
{
    depth = depthP;
}

void position::set_future_positions_size(int i)
{
    future_positions_size = i;
}

void position::randomize_order_of_possible_moves()
{
    // I want to run through the first FOUR elements and swap them with a random element in the vector.

    for (int i = 0; i < possible_moves.size() && i < 4; i++)
    {
        int random_index = rand() % possible_moves.size();

        coordinate temp = possible_moves[random_index];

        possible_moves[random_index] = possible_moves[i];

        possible_moves[i] = temp;
    }
}

void position::clean_up_amplifying_vectors()
{
    remove_treasure_spot_objects_from_vector(squares_amplifying_comp_2);
    remove_treasure_spot_objects_from_vector(squares_amplifying_comp_3);
    remove_treasure_spot_objects_from_vector(squares_amplifying_user_2);
    remove_treasure_spot_objects_from_vector(squares_amplifying_user_3);
}

void position::rearrange_possible_moves(const vector<coordinate>& front_moves)
{
    int start_size = possible_moves.size();

    vector<coordinate> replacement = front_moves; // possible_moves will be set to this vector at the end of the function.

    for (const coordinate& temp: possible_moves)
    {
        if (!in_coordinate_vector(front_moves, temp)) // not in front_moves, so add to replacement:
        {
            replacement.push_back(temp);
        }
    }

    possible_moves = replacement;

    int end_size = possible_moves.size();

    if (end_size != start_size)
    {
        throw runtime_error("possible_moves.size changes.\n");
    }
}

void position::initialize_hash_value_of_position()
{
    double val = pre_hash_value_of_position;

    while (val < 100000.0)
    {
        val *= 10.0;
    }

    hash_value_of_position = static_cast<int>(round(val));
}

void position::add_position_to_transposition_table(bool is_evaluation_indisputable)
{
    position_info_for_TT temp;
    temp.board = board;
    temp.evaluation = evaluation;
    temp.calculation_depth_from_this_position = calculation_depth_from_this_position;
    temp.is_evaluation_indisputable = is_evaluation_indisputable;
    temp.is_comp_turn = is_comp_turn;

    // If the evaluation is indisputable, no position that uses this position in the TT needs to look at its possible moves.
    // That position should automatically just accept this position's evaluation, "no questions asked".

    // If the evaluation isn't indisputable, then any position that uses this position in the TT should get access to
    // possible moves ordered, allowing that position to efficiently conduct its own deeper search.

    if (!is_evaluation_indisputable)
    {
        sort(future_positions.begin(), future_positions.end(), compare_future_positions_by_evaluation);

        for (const unique_ptr<position>& pos: future_positions)
        {
            temp.possible_moves_sorted.push_back(pos->get_last_move());
        }
    }

    // else, temp's possible_moves_sorted vector is simply left empty.

    // Now to run through the approprixate index in the TT, and replace an earlier duplicate position of temp, if one exists.

    bool does_a_duplicate_exist = false;

    for (position_info_for_TT& current: transposition_table[hash_value_of_position]) // by reference is deliberate.
    {
        if (current.board == temp.board && current.is_comp_turn == temp.is_comp_turn)
        {
            does_a_duplicate_exist = true;

            if (temp.calculation_depth_from_this_position > current.calculation_depth_from_this_position)
            {
                current = temp;

                break;
            }
        }
    }

    if (!does_a_duplicate_exist)
    {
        // Since if a duplicate did exist, it would have either been replaced by temp, or temp would have had a lower calculation_depth
        // and wouldn't be "worthy" enough to replace it. Either way, temp should not be push_backed in that scenario.

        // On the other hand, if a duplicate doesn't exist, obviously temp should obviously be added to the TT.

        transposition_table[hash_value_of_position].push_back(temp);

        if (transposition_table[hash_value_of_position].size() == 1)
        {
            // So temp is the only element in the vector. Therefore, it was the first element to have been put there:

            indices_of_elements_in_TT.push_back(hash_value_of_position);
        }
    }
}

// HELPERS:

bool position::did_computer_win() const
{
    return (!is_comp_turn && did_someone_win());
}

bool position::did_opponent_win() const
{
    return (is_comp_turn && did_someone_win());
}

// Pre-condition: It has already been checked that no one has won the game.
// Post-condition: The function will return true if the board is full... it is not guaranteed no one has four-in-a-row.
bool position::is_game_drawn() const
{
    return (number_of_pieces == 42); // since a 7x6 board will have 42 pieces when filled up completely.
}

bool position::evaluation_in_future_positions(int eval) const
{
    for (int i = 0; i < future_positions.size(); i++)
    {
        if (future_positions[i]->evaluation == eval)
        {
            return true;
        }
    }

    return false;
}

bool position::is_valid_move(string column) const
{
    // First, check if column is only 1 in size:

    if (column.size() != 1)
    {
        return false;
    }

    char letter = column[0]; // just easier to deal with chars.

    // Now check if letter is between 'A' - 'G' (or 'a' - 'g')

    if (!is_acceptable_letter(letter))
    {
        return false;
    }

    // Now to figure out what column is represented by letter:

    int col = 0;

    if (letter >= 'a') // lowercase...
    {
        col = letter - 'a';
    }

    else // uppercase...
    {
        col = letter - 'A';
    }

    // Now to check if the top spot of col in board is empty:

    return (board[0][col] == ' ');
}

void position::remove_treasure_spot_objects_from_vector(vector<treasure_spot>& vec)
{
    vector<treasure_spot> updated_vec; // will store the good elements of vec (the ones that shouldn't be removed from vec).

    for (const treasure_spot& element: vec)
    {
        coordinate square = element.current_square;

        if (board[square.row][square.col] == ' ') // square is safe, since its location in board is empty (stores ' ').
        {
            updated_vec.push_back(element);
        }
    }

    vec = updated_vec;
}

void position::print_amplifying_vectors()
{
    for (const treasure_spot& temp: squares_amplifying_comp_2)
    {
        coordinate square = temp.current_square;

        cout << char('A' + square.col) << (6 - square.row) << "\n";
    }

    for (const treasure_spot& temp: squares_amplifying_comp_3)
    {
        coordinate square = temp.current_square;

        cout << char('A' + square.col) << (6 - square.row) << "\n";
    }

    for (const treasure_spot& temp: squares_amplifying_user_2)
    {
        coordinate square = temp.current_square;

        cout << char('A' + square.col) << (6 - square.row) << "\n";
    }

    for (const treasure_spot& temp: squares_amplifying_user_3)
    {
        coordinate square = temp.current_square;

        cout << char('A' + square.col) << (6 - square.row) << "\n";
    }
}

void position::find_critical_moves(vector<coordinate>& critical_moves)
{
    // If it is the comp's turn in this position, I'll want to first add any moves that win for the comp to the
    // critical_moves vector first. This is because the critical_moves will be put at the front of possible_moves vector,
    // and if the comp can win then I want it to examine it right away (to allow minimax to prune other moves immediately).

    if (is_comp_turn)
    {
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_comp_2, false, 'C');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_comp_3, true, 'C');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_user_2, false, 'U');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_user_3, true, 'U');
    }

    else // user's turn, so look at moves that win for the user first...
    {
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_user_2, false, 'U');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_user_3, true, 'U');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_comp_2, false, 'C');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_comp_3, true, 'C');
    }

    remove_duplicates(critical_moves);
}

void position::find_critical_moves_in_amplifying_vector(vector<coordinate>& critical_moves,
                                                        const vector<treasure_spot>& amplifying_vector,
                                                        bool are_3_pieces, char piece)
{
    for (const treasure_spot& temp: amplifying_vector)
    {
        coordinate current_square = temp.current_square;
        coordinate next_square = temp.next_square;
        coordinate other_next_square = temp.other_next_square;

        if (board[current_square.row][current_square.col] == ' ') // so the square is still a valid amplifying square...
        {
            // Now to see if the square can be filled in one move...

            if (current_square.row == max_row_index || board[current_square.row + 1][current_square.col] != ' ')
            {
                // Now to see if the square creates a 4-in-a-row, if filled...

                if (are_3_pieces || (is_in_bounds(next_square) && board[next_square.row][next_square.col] == piece) ||
                    (is_in_bounds(other_next_square) && board[other_next_square.row][other_next_square.col] == piece))
                {
                    // Since the square/move either amplifies a 3-in-a-row or connects a 2-in-a-row with a piece,
                    // add it to the critical_moves vector (since it creates a 4-in-a-row):

                    critical_moves.push_back(current_square);
                }
            }
        }
    }
}

 bool position::is_in_bounds(const coordinate& square) const
 {
     return (square.row >= 0 && square.row <= max_row_index && square.col >= 0 && square.col <= max_col_index);
 }

 bool position::in_coordinate_vector(const vector<coordinate>& vec, const coordinate& element)
 {
     for (const coordinate& temp: vec)
     {
         if (temp == element)
         {
             return true;
         }
     }

     return false;
 }

 void position::remove_duplicates(vector<coordinate>& vec)
 {
     vector<coordinate> replacement; // vec will be set equal to replacement at the end of this function.

     for (const coordinate& temp: vec)
     {
         if (!in_coordinate_vector(replacement, temp))
         {
             replacement.push_back(temp);
         }
     }

     vec = replacement;
 }

 void position::initialize_row_barriers()
 {
    // Find all squares that give both comp AND user a 4-in-a-row. Could be in 2-in-a-row vectors too.

    vector<coordinate> squares_winning_for_comp;
    find_winning_squares(squares_winning_for_comp, squares_amplifying_comp_3, squares_amplifying_comp_2, 'C');

    vector<coordinate> squares_winning_for_user;
    find_winning_squares(squares_winning_for_user, squares_amplifying_user_3, squares_amplifying_user_2, 'U');

    // Now parse through squares_winning_for_comp vector, and for each square check if it's also
    // in the squares_winning_for_user vector. If so, it's a "barricade" square.

    vector<coordinate> barricade_squares;

    for (const coordinate& temp: squares_winning_for_comp)
    {
        if (in_coordinate_vector(squares_winning_for_user, temp))
        {
            barricade_squares.push_back(temp);
        }
    }

    // Now, get the lowest barricade_squares (visually) for each column in barricade_squares vector,
    // extract their row index, and store these indices in the row_barriers private member (the whole point of this function).

    for (int i = 0; i <= max_col_index; i++) // since row_barries isn't even initialized yet. Give it 7 UNDEFINED ints.
    {
        row_barriers.push_back(UNDEFINED);
    }

    for (const coordinate& temp: barricade_squares)
    {
        // square temp's col value corresponds to an index in row_barriers:

        if (row_barriers[temp.col] == UNDEFINED || row_barriers[temp.col] < temp.row)
        {
            // Either that index in row_barriers has no value (undefined), or it stores a row
            // value that is higher up (visually) on the board than square temp's row value.
                // Since lower row values barricade more of a column, I should switch to that:

            row_barriers[temp.col] = temp.row;
        }
    }
 }

 void position::find_winning_squares(vector<coordinate>& vec, const vector<treasure_spot>& squares_amplifying_3,
                                     const vector<treasure_spot>& squares_amplifying_2, char piece)
{
    for (const treasure_spot& temp: squares_amplifying_3)
    {
        if (board[temp.current_square.row][temp.current_square.col] == ' ')
        {
            vec.push_back(temp.current_square);
        }
    }

    for (const treasure_spot& temp: squares_amplifying_2)
    {
        if (board[temp.current_square.row][temp.current_square.col] == ' ' &&
            ((is_in_bounds(temp.next_square) && board[temp.next_square.row][temp.next_square.col] == piece) ||
             (is_in_bounds(temp.other_next_square) && board[temp.other_next_square.row][temp.other_next_square.col] == piece)))
        {
            vec.push_back(temp.current_square);
        }
    }

    remove_duplicates(vec);
}

// FUNCTIONS SPECIFICALLY FOR THE VERSUS SIM:

double position::get_static_thinking_time() const
{
    return position::thinking_time;
}

void position::set_static_thinking_time(double val)
{
    position::thinking_time = val;
}

unique_ptr<tool> position::call_static_think_on_game_position(const vector <vector<char>>& boardP, bool is_comp_turnP,
                                                              coordinate last_moveP, const vector<treasure_spot>& squares_amplifying_comp_2P,
                                                              const vector<treasure_spot>& squares_amplifying_comp_3P,
                                                              const vector<treasure_spot>& squares_amplifying_user_2P,
                                                              const vector<treasure_spot>& squares_amplifying_user_3P, bool starting_new_game)
{
    return move(think_on_game_position(boardP, is_comp_turnP, last_moveP, squares_amplifying_comp_2P, squares_amplifying_comp_3P,
                                       squares_amplifying_user_2P, squares_amplifying_user_3P, starting_new_game));
}

unique_ptr<tool> position::call_static_think_on_game_position(bool is_comp_turnP, bool starting_new_game)
{
    return move(think_on_game_position(is_comp_turnP, starting_new_game));
}

coordinate_and_value position::find_quick_winning_move(int max_number_moves_acceptable) const
{
    // Returns the coordinate for a move winning in <= max_number_moves_acceptable, and returns the number of moves it wins in
    // (this is why I'm returning a coordinate_and_value object).

    coordinate_and_value solution; // WILL BE RETURNED.
    solution.square = {UNDEFINED, UNDEFINED};
    solution.value = UNDEFINED;

    if (max_number_moves_acceptable < 1) // Cannot do anything in less than 1 move...
    {
        return solution; // deliberately still UNDEFINED
    }

    // Now do a preliminary pass over all possible_moves, just checking at depth 1. Purpose is to see if any win immediately,
    // which saves having to recursively delve into each of them in the rest of this function.

    coordinate immediate_win = return_a_move_that_wins_immediately(); // Function returns {UNDEFINED, UNDEFINED} if no immediate win.

    if (immediate_win.row != UNDEFINED) // An immediate win was found!
    {
        solution.square = immediate_win;
        solution.value = 1;

        return solution;
    }

    else if (max_number_moves_acceptable < 3)
    {
        // Due to the else if, immediate_win = {UNDEFINED, UNDEFINED}.
        // So there's no immediate win, yet max_number_of_moves_acceptable is less than 3. This only gives player 1 move to win.
        // Since I've verified this isn't the case, return solution, which is still UNDEFINED.

        return solution;
    }

    vector<vector<char>> assisting_board = board;

    for (const coordinate& current_move: possible_moves)
    {
        char piece_to_be_played = 'C';

        if (!is_comp_turn)
        {
            piece_to_be_played = 'U';
        }

        assisting_board[current_move.row][current_move.col] = piece_to_be_played;

        if (piece_to_be_played == 'C')
        {
            piece_to_be_played = 'U';
        }

        else
        {
            piece_to_be_played = 'C';
        }

        unique_ptr<position> p1 = make_unique<position>(assisting_board, !is_comp_turn, current_move, empty_amplifying_vector,
                                                        empty_amplifying_vector, empty_amplifying_vector, empty_amplifying_vector);

        // NOTE: I know that player couldn't have won with this move, since I already checked if the player could win immediately
        // with any of the moves in possible_moves.

        // Check if p1 is in the TT. IF it is, make sure the TT says the position's evaluation is winning for the player.

        bool is_player_winning = true;

        for (const position_info_for_TT& current: transposition_table[p1->hash_value_of_position])
        {
            if (current.board == p1->board && current.is_comp_turn == p1->is_comp_turn &&
                ((is_comp_turn && current.evaluation != INT_MAX) || (!is_comp_turn && current.evaluation != INT_MIN)))
            {
                is_player_winning = false;

                break;
            }
        }

        if (!is_player_winning)
        {
            // current_move is a failure, so continue to the next move in for loop:

            assisting_board[current_move.row][current_move.col] = ' ';

            continue;
        }

        // Now run through each of the opponent's responses, and see if player can win against all of them within the bound of moves.
        // This will involve recursion.

        int players_worst_performance_against_all_opponent_responses = 1;

        bool can_opponent_win = false; // set to true if the opponent has a move that wins.

        for (const coordinate& opponent_response: p1->possible_moves)
        {
            assisting_board[opponent_response.row][opponent_response.col] = piece_to_be_played;

            unique_ptr<position> p2 = make_unique<position>(assisting_board, !p1->is_comp_turn, opponent_response, empty_amplifying_vector,
                                                            empty_amplifying_vector, empty_amplifying_vector, empty_amplifying_vector);

            // Now, check if the opponent is winning:

            if ((is_comp_turn && p2->evaluation == INT_MIN) || (!is_comp_turn && p2->evaluation == INT_MAX))
            {
                can_opponent_win = true;

                assisting_board[opponent_response.row][opponent_response.col] = ' ';

                break; // No need to consider other moves for the opponent, as there is one move that wins for the opponent.
                       // current_move is a failure.
            }

            int number_of_moves_player_can_win_from_here = p2->find_quick_winning_move(max_number_moves_acceptable - 2).value;

            if (number_of_moves_player_can_win_from_here == UNDEFINED ||
                number_of_moves_player_can_win_from_here > players_worst_performance_against_all_opponent_responses)
            {
                players_worst_performance_against_all_opponent_responses = number_of_moves_player_can_win_from_here;
            }

            assisting_board[opponent_response.row][opponent_response.col] = ' ';

            if (players_worst_performance_against_all_opponent_responses == UNDEFINED)
            {
                break; // current_move is a failure.
            }
        }

        if (!can_opponent_win && players_worst_performance_against_all_opponent_responses != UNDEFINED)
        {
            if (solution.value == UNDEFINED || players_worst_performance_against_all_opponent_responses + 2 < solution.value)
            {
                solution.value = players_worst_performance_against_all_opponent_responses + 2;
                solution.square = current_move;
            }
        }

        assisting_board[current_move.row][current_move.col] = ' ';
    }

    return solution;
}

coordinate position::return_a_move_that_wins_immediately() const
{
    // NOTE: if such a move doesn't exist, return {UNDEFINED, UNDEFINED}.

    vector<vector<char>> assisting_board = board;

    char piece = 'C';

    if (!is_comp_turn)
    {
        piece = 'U';
    }

    for (const coordinate& current_move: possible_moves)
    {
        assisting_board[current_move.row][current_move.col] = piece;

        unique_ptr<position> pt = make_unique<position>(assisting_board, !is_comp_turn, current_move, empty_amplifying_vector,
                                                        empty_amplifying_vector, empty_amplifying_vector, empty_amplifying_vector);

        if ((is_comp_turn && pt->did_computer_win()) || (!is_comp_turn && pt->did_opponent_win()))
        {
            // pt->did_opponent_win() really means "pt->did_user_win()"

            return current_move;
        }

        assisting_board[current_move.row][current_move.col] = ' ';
    }

    return {UNDEFINED, UNDEFINED};
}

// PUBLIC STATIC METHODS:

vector<vector<double>> position::find_hash_values_for_all_squares_in_board(char piece)
{
    vector<vector<double>> vec; // will be returned.

    vector<double> single_row;

    for (int col = 0; col <= max_col_index; col++)
    {
        single_row.push_back(0.0);
    }

    for (int row = 0; row <= max_row_index; row++)
    {
        vec.push_back(single_row);
    }

    // Now run through vec, giving each element/square a double value:

    for (int row = 0; row <= max_row_index; row++)
    {
        for (int col = 0; col <= max_col_index; col++)
        {
            double x = static_cast<double>(row+1) * static_cast<double>(col+1) + static_cast<double>(col+1) / 7.0;

            // Now to use x as an input to a mathematical function to find the square's hash value. Which mathematical function
            // is chosen depends on the piece square stores:

            if (piece == 'C')
            {
                // cot(x+5) / 5:

                vec[row][col] = cotangent_with_degrees(x + 5.0) / 5.0;
            }

            else if (piece == 'U')
            {
                vec[row][col] = log10(x + 1.0);
            }

            else // piece = ' ':
            {
                vec[row][col] = sqrt(x + 1.0) - 1.3;
            }
        }
    }

    return vec;
}

double position::cotangent_with_degrees(double angle_in_degrees)
{
    // First, convert to radians:

    double angle_in_rad = angle_in_degrees * (PI / 180.0);

    // Now return the cotangent:

    return 1.0 / (tan(angle_in_rad));
}

void position::reset_transposition_table()
{
    for (int index: indices_of_elements_in_TT)
    {
        // Delete all the elements in the inner vector of the TT at index.

        transposition_table[index].clear();
    }

    indices_of_elements_in_TT.clear();
}

bool position::compare_future_positions_by_evaluation(const unique_ptr<position>& first_pos, const unique_ptr<position>& second_pos)
{
    if (first_pos->is_comp_turn)
    {
        // User's turn in the calling position/object (since its the comp's turn one move later), so the user is the player here.

        // The user would like to look at positions with a smaller evaluation:

        return (first_pos->evaluation <= second_pos->evaluation);
    }

    else
    {
        // Comp's turn in the calling position/object, so the comp is the player here.

        // The comp would like to look at positions with a greater evaluation:

        return (first_pos->evaluation >= second_pos->evaluation);
    }
}

position_info_for_TT position::find_duplicate_in_TT(const unique_ptr<position>& pt)
{
    for (const position_info_for_TT& current: transposition_table[pt->hash_value_of_position])
    {
        if (current.board == pt->board && current.is_comp_turn == pt->is_comp_turn)
        {
            return current;
        }
    }

    throw runtime_error("Did not find a duplicate in find_duplicate_in_TT()\n");
}

unique_ptr<position> position::think_on_game_position(const vector <vector<char>>& boardP, bool is_comp_turnP, coordinate last_moveP,
                                    const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
                                    const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P,
                                    bool starting_new_game)
{
    if (starting_new_game)
    {
        reset_transposition_table();
    }

    steady_clock::time_point start_time = steady_clock::now();

    unique_ptr<position> pt = make_unique<position>(boardP, is_comp_turnP, last_moveP, squares_amplifying_comp_2P, squares_amplifying_comp_3P,
                                                    squares_amplifying_user_2P, squares_amplifying_user_3P); // pt will be returned.

    duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - start_time);

    while (time_span.count() < thinking_time && !find_duplicate_in_TT(pt).is_evaluation_indisputable && pt->number_of_pieces + depth_limit <= 43)
    {
        depth_limit ++; // Iterative deepening.

        pt = make_unique<position>(boardP, is_comp_turnP, last_moveP, squares_amplifying_comp_2P, squares_amplifying_comp_3P,
                                   squares_amplifying_user_2P, squares_amplifying_user_3P); // calls constructor 2.

        time_span = duration_cast<duration<double>>(steady_clock::now() - start_time);
    }

    depth_limit = 1; // in preparation for the next time the Engine thinks.

    return move(pt);
}

unique_ptr<position> position::think_on_game_position(bool is_comp_turnP, bool starting_new_game)
{
    if (starting_new_game)
    {
        reset_transposition_table();
    }

    steady_clock::time_point start_time = steady_clock::now();

    // This function is similar to the one above, except it's for getting the computer to think at the starting position.
    // Still do iterative deepening, since I want the computer to play as good as possible even on the first move of the game.

    unique_ptr<position> pt = make_unique<position>(is_comp_turnP); // pt will be returned.

    duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - start_time);

    while (time_span.count() < thinking_time && !find_duplicate_in_TT(pt).is_evaluation_indisputable && pt->number_of_pieces + depth_limit <= 43)
    {
        depth_limit ++; // Iterative deepening.

        pt = make_unique<position>(is_comp_turnP); // calls constructor 2.

        time_span = duration_cast<duration<double>>(steady_clock::now() - start_time);
    }

    depth_limit = 1; // in preparation for the next time the Engine thinks.

    return move(pt);
}

// PRIVATE METHODS:

void position::analyze_last_move()
{
    // First, check if this position has already been analyzed, and has an evaluation in the transposition table.

    // If so, accept this evaluation if one of two conditions are met:
        // 1) The evaluation is indisputable (i.e., forced).
        // 2) The duplicate position in the hash table has a >= "calculation_depth_from_this_position" than the calling object.

    for (const position_info_for_TT& current: transposition_table[hash_value_of_position])
    {
        if (current.board == board && current.is_comp_turn == is_comp_turn &&
            (current.is_evaluation_indisputable || current.calculation_depth_from_this_position >= calculation_depth_from_this_position))
        {
            evaluation = current.evaluation;

          //  counter_of_TT_usefulness ++;

            return; // All done for this position entirely!
        }
    }

    // See how many pieces are in a row horizontally due to last_move:

    analyze_horizontal_perspective_of_last_move(); // sets evaluation to INT_MAX/INT_MIN if someone won,
                                                   // and updates one of the amplifying vectors if appropriate.

    if (evaluation == INT_MAX || evaluation == INT_MIN) // someone won...
    {
        add_position_to_transposition_table(true);

        return;
    }

    analyze_vertical_perspective_of_last_move(); // sets evaluation to INT_MAX/INT_MIN if someone won,
                                                 // and updates one of the amplifying vectors if appropriate.

    if (evaluation == INT_MAX || evaluation == INT_MIN) // someone won...
    {
        add_position_to_transposition_table(true);

        return;
    }

    analyze_positive_slope_diagonal_perspective_of_last_move(); // sets evaluation to INT_MAX/INT_MIN if someone won,
                                                                // and updates one of the amplifying vectors if appropriate.

    if (evaluation == INT_MAX || evaluation == INT_MIN) // someone won...
    {
        add_position_to_transposition_table(true);

        return;
    }

    analyze_negative_slope_diagonal_perspective_of_last_move(); // sets evaluation to INT_MAX/INT_MIN if someone won,
                                                                // and updates one of the amplifying vectors if appropriate.

    if (evaluation == INT_MAX || evaluation == INT_MIN) // someone won...
    {
        add_position_to_transposition_table(true);

        return;
    }

    // Before calling minimax(), check if the game is drawn,
    // or if the depth_limit is reached (meaning the computer should evaluate the position now if it is in a quiescent state):

    if (number_of_pieces == 42)
    {
        evaluation = 0;

        add_position_to_transposition_table(true);

        return;
    }

    vector<coordinate> critical_moves; // stores moves (for either side) that make a 4-in-a-row, that can be played now.

    find_critical_moves(critical_moves); // passed by reference.

    if (depth >= depth_limit && critical_moves.size() == 0) // Quiescent state reached at depth_limit (or beyond).
    {
        // So, smart_evaluation() is ready to evaluate the position:

        smart_evaluation(); // gives the evaluation attribute a value.

        add_position_to_transposition_table(false);

        return;
    }

    // Now see if there's an earlier duplicate of position in the TT, that has a non-empty possible moves vector.
    // If so, set this calling object's possible_moves vector to equal it.
    // Note that this is where nearly all the speed of the TT comes to fruition!

    bool found_earlier_duplicate_in_TT = false;

    for (const position_info_for_TT& current: transposition_table[hash_value_of_position])
    {
        if (current.board == board && current.is_comp_turn == is_comp_turn && !current.possible_moves_sorted.empty())
        {
            possible_moves = current.possible_moves_sorted;

            found_earlier_duplicate_in_TT = true;

            break;
        }
    }

    if (!found_earlier_duplicate_in_TT)
    {
        rearrange_possible_moves(critical_moves); // Function puts the critical_moves in possible_moves at the front
                                                  // of possible_moves.
    }

    minimax();
}

void position::analyze_horizontal_perspective_of_last_move()
{
    coordinate start_point = find_starting_horizontal_point();
    coordinate end_point = find_ending_horizontal_point();
    int num_pieces_in_a_row = end_point.col - start_point.col + 1;

    if (num_pieces_in_a_row == 1) // "1-in-a-row"... Version 20 values it!
    {
        char piece = board[last_move.row][last_move.col];

        // By definition, there are empty squares on either side of last_move, or out-of-bounds.
            // I don't need to check if such a square is empty, due to num_pieces_in_a_row = 1.
            // I don't need to explicitly check if such a square is in-bounds, if I check the square two over from last_move is in-bounds.

        // See if the square to the left amplifies a "2-in-a-row":

        if (last_move.col - 2 >= 0 && board[last_move.row][last_move.col - 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;

            temp.next_square = temp.current_square;
            temp.next_square.col -= 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col += 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        if (last_move.col + 2 <= max_col_index && board[last_move.row][last_move.col + 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;

            temp.next_square = temp.current_square;
            temp.next_square.col += 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col -= 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        return;
    }

    if (num_pieces_in_a_row >= 4)
    {
        if (is_comp_turn) // comp's turn now, so user just moved and got 4-in-a-row:
        {
            evaluation = INT_MIN;
            return;
        }

        evaluation = INT_MAX;
        return;
    }

    // num_pieces_in_a_row must be 2 or 3. This means I may want to add to one of the amplifying vectors.
    // Create a treasure_spot object to possibly do this:

    treasure_spot preceding_point; // space to the left of start_point on board.

    preceding_point.current_square = start_point;
    preceding_point.current_square.col--;

    preceding_point.next_square = preceding_point.current_square;
    preceding_point.next_square.col--;

    treasure_spot succeeding_point; // space to the right of end_point on board.

    succeeding_point.current_square = end_point;
    succeeding_point.current_square.col++;

    succeeding_point.next_square = succeeding_point.current_square;
    succeeding_point.next_square.col++;

    // Version 19 additions:

    preceding_point.other_next_square = succeeding_point.current_square;
    succeeding_point.other_next_square = preceding_point.current_square;

    if (preceding_point.current_square.col >= 0 &&
        board[preceding_point.current_square.row][preceding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, preceding_point);
    }

    if (succeeding_point.current_square.col <= max_col_index &&
        board[succeeding_point.current_square.row][succeeding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, succeeding_point);
    }
}

void position::analyze_vertical_perspective_of_last_move()
{
    coordinate start_point = find_starting_vertical_point();
    coordinate end_point = find_ending_vertical_point();
    int num_pieces_in_a_row = end_point.row - start_point.row + 1;

    if (num_pieces_in_a_row == 1) // "1-in-a-row"... adds no value vertically, and not worth recording:
    {
        return;
    }

    if (num_pieces_in_a_row >= 4)
    {
        if (is_comp_turn) // comp's turn now, so user just moved and got 4-in-a-row:
        {
            evaluation = INT_MIN;
            return;
        }

        evaluation = INT_MAX;
        return;
    }

    // num_pieces_in_a_row must be 2 or 3. This means I may want to add to one of the amplifying vectors:
    // Create a treasure_spot object to possibly do this:

    treasure_spot preceding_point; // space above start_point on board.

    preceding_point.current_square = start_point;
    preceding_point.current_square.row--;

    preceding_point.next_square = preceding_point.current_square;
    preceding_point.next_square.row--;

    // Version 19 addition:

    preceding_point.other_next_square.row = UNDEFINED;
    preceding_point.other_next_square.col = UNDEFINED;
    // Since vertically, there is no other next square, only the one above. Below is filled with pieces.

    if (preceding_point.current_square.row >= 0) // don't need to also check if the square is empty since it's guaranteed to be.
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, preceding_point);
    }

    // Note that I only need to check above the start_point. Checking below the end_point is pointless since the
    // column is filled with pieces under last_move (there are no empty squares).
}

void position::analyze_positive_slope_diagonal_perspective_of_last_move()
{
    coordinate start_point = find_starting_positive_slope_diagonal_point();
    coordinate end_point = find_ending_positive_slope_diagonal_point();
    int num_pieces_in_a_row = end_point.col - start_point.col + 1; // could also have used start_point.row - end_point.row + 1.

    if (num_pieces_in_a_row == 1) // "1-in-a-row"... Version 20 values it!
    {
        char piece = board[last_move.row][last_move.col];

        // By definition, there are empty squares on either side of last_move, or out-of-bounds.
            // I don't need to check if such a square is empty, due to num_pieces_in_a_row = 1.
            // I don't need to explicitly check if such a square is in-bounds, if I check the square two over from last_move is in-bounds.

        // See if the square to the down-left amplifies a "2-in-a-row":

        if (last_move.col - 2 >= 0 && last_move.row + 2 <= max_row_index && board[last_move.row + 2][last_move.col - 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;
            temp.current_square.row++;

            temp.next_square = temp.current_square;
            temp.next_square.col -= 2;
            temp.next_square.row += 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col += 2;
            temp.other_next_square.row -= 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        // See if the square to the up-right amplifies a "2-in-a-row":

        if (last_move.col + 2 <= max_col_index && last_move.row - 2 >= 0 && board[last_move.row - 2][last_move.col + 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;
            temp.current_square.row--;

            temp.next_square = temp.current_square;
            temp.next_square.col += 2;
            temp.next_square.row -= 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col -= 2;
            temp.other_next_square.row += 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        return;
    }

    if (num_pieces_in_a_row >= 4)
    {
        if (is_comp_turn) // comp's turn now, so user just moved and got 4-in-a-row:
        {
            evaluation = INT_MIN;
            return;
        }

        evaluation = INT_MAX;
        return;
    }

    // num_pieces_in_a_row must be 2 or 3. This means I may want to add to one of the amplifying vectors:
    // Create a treasure_spot object to possibly do this:

    treasure_spot preceding_point; // space to the down-left of start_point on board.

    preceding_point.current_square = start_point;
    preceding_point.current_square.col--;
    preceding_point.current_square.row++;

    preceding_point.next_square = preceding_point.current_square;
    preceding_point.next_square.col--;
    preceding_point.next_square.row++;

    treasure_spot succeeding_point; // space to the up-right of end_point on board.

    succeeding_point.current_square = end_point;
    succeeding_point.current_square.col++;
    succeeding_point.current_square.row--;

    succeeding_point.next_square = succeeding_point.current_square;
    succeeding_point.next_square.col++;
    succeeding_point.next_square.row--;

    // Version 19 additions:

    preceding_point.other_next_square = succeeding_point.current_square;
    succeeding_point.other_next_square = preceding_point.current_square;

    if (preceding_point.current_square.col >= 0 &&
        preceding_point.current_square.row <= max_row_index &&
        board[preceding_point.current_square.row][preceding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, preceding_point);
    }

    if (succeeding_point.current_square.col <= max_col_index &&
        succeeding_point.current_square.row >= 0 &&
        board[succeeding_point.current_square.row][succeeding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, succeeding_point);
    }
}

void position::analyze_negative_slope_diagonal_perspective_of_last_move()
{
    coordinate start_point = find_starting_negative_slope_diagonal_point();
    coordinate end_point = find_ending_negative_slope_diagonal_point();
    int num_pieces_in_a_row = end_point.col - start_point.col + 1; // could also have used end_point.row - start_point.row + 1.

    if (num_pieces_in_a_row == 1) // "1-in-a-row"... Version 20 values it!
    {
        char piece = board[last_move.row][last_move.col];

        // By definition, there are empty squares on either side of last_move, or out-of-bounds.
            // I don't need to check if such a square is empty, due to num_pieces_in_a_row = 1.
            // I don't need to explicitly check if such a square is in-bounds, if I check the square two over from last_move is in-bounds.

        // See if the square to the up-left amplifies a "2-in-a-row":

        if (last_move.col - 2 >= 0 && last_move.row - 2 >= 0 && board[last_move.row - 2][last_move.col - 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;
            temp.current_square.row--;

            temp.next_square = temp.current_square;
            temp.next_square.col -= 2;
            temp.next_square.row -= 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col += 2;
            temp.other_next_square.row += 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        // See if the square to the down-right amplifies a "2-in-a-row":

        if (last_move.col + 2 <= max_col_index && last_move.row + 2 <= max_row_index && board[last_move.row + 2][last_move.col + 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;
            temp.current_square.row++;

            temp.next_square = temp.current_square;
            temp.next_square.col += 2;
            temp.next_square.row += 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col -= 2;
            temp.other_next_square.row -= 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        return;
    }

    if (num_pieces_in_a_row >= 4)
    {
        if (is_comp_turn) // comp's turn now, so user just moved and got 4-in-a-row:
        {
            evaluation = INT_MIN;
            return;
        }

        evaluation = INT_MAX;
        return;
    }

    // num_pieces_in_a_row must be 2 or 3. This means I may want to add to one of the amplifying vectors:
    // Create a treasure_spot object to possibly do this:

    treasure_spot preceding_point; // space to the up-left of start_point on board.

    preceding_point.current_square = start_point;
    preceding_point.current_square.col--;
    preceding_point.current_square.row--;

    preceding_point.next_square = preceding_point.current_square;
    preceding_point.next_square.col--;
    preceding_point.next_square.row--;

    treasure_spot succeeding_point; // space to the down-right of end_point on board.

    succeeding_point.current_square = end_point;
    succeeding_point.current_square.col++;
    succeeding_point.current_square.row++;

    succeeding_point.next_square = succeeding_point.current_square;
    succeeding_point.next_square.col++;
    succeeding_point.next_square.row++;

    // Version 19 additions:

    preceding_point.other_next_square = succeeding_point.current_square;
    succeeding_point.other_next_square = preceding_point.current_square;

    if (preceding_point.current_square.col >= 0 &&
        preceding_point.current_square.row >= 0 &&
        board[preceding_point.current_square.row][preceding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, preceding_point);
    }

    if (succeeding_point.current_square.col <= max_col_index &&
        succeeding_point.current_square.row <= max_row_index &&
        board[succeeding_point.current_square.row][succeeding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, succeeding_point);
    }
}

void position::add_to_appropriate_amplifying_vector(int num_pieces_in_a_row, treasure_spot empty_square)
{
    if (is_comp_turn && num_pieces_in_a_row == 2) // user just moved, and has 2 pieces in a row from their last_move:
    {
        squares_amplifying_user_2.push_back(empty_square);
    }

    else if (!is_comp_turn && num_pieces_in_a_row == 2) // comp just moved, and has 2 pieces in a row from its last_move:
    {
        squares_amplifying_comp_2.push_back(empty_square);
    }

    else if (is_comp_turn && num_pieces_in_a_row == 3) // user just moved, and has 3 pieces in a row from their last_move:
    {
        squares_amplifying_user_3.push_back(empty_square);
    }

    else if (!is_comp_turn && num_pieces_in_a_row == 3) // comp just moved, and has 3 pieces in a row from its last_move:
    {
        squares_amplifying_comp_3.push_back(empty_square);
    }
}

void position::minimax()
{
    // Here's where all the magic happens.

    // The game is not over, so look at all positions one move ahead.
    // Then, set evaluation accordingly, using the minimax algorithm...

    for (int i = 0; i < possible_moves.size(); i++) // running through the possible_moves vector to play out each move.
    {
        coordinate current_move = possible_moves[i];

        // Now to make a copy of the current board:

        vector <vector<char>> copy_board = board;

        if (is_comp_turn)
        {
            copy_board[current_move.row][current_move.col] = 'C';
        }

        else // opponent's turn:
        {
            copy_board[current_move.row][current_move.col] = 'U';
        }

        // Now to make a new position object, with this updated board that's one move ahead.

        unique_ptr<position> pt = make_unique<position>(copy_board, !is_comp_turn, depth + 1,
                                                        number_of_pieces + 1, current_move,
                                                        possible_moves, i, alpha, beta,
                                                        squares_amplifying_comp_2, squares_amplifying_comp_3,
                                                        squares_amplifying_user_2, squares_amplifying_user_3,
                                                        pre_hash_value_of_position, num_pieces_per_column);
                                                        // Note that this position's 4 amplifying vectors are being sent.
                                                        // Any necessary additions to be made to the amplifying vectors
                                                        // due to last_move (represented by current_move here) will be
                                                        // handled in the constructor.
                                                        // Also, this position's pre_hash_value_of_position variable is being sent.
                                                        // It will be updated appropriately in the constructor of the child position node.
                                                        // Finally, num_pieces_per_column for this position is being sent,
                                                        // and then necessary change due to last_move will be taken care of in constructor.

        int future_evaluation = pt->evaluation;

        bool is_child_pruned = pt->is_a_pruned_branch; // stores true if pt (this node's child) got pruned from alpha-beta.
                                                       // This is important to know since if this node's final evaluation actually
                                                       // = child's, it is unstable (and shouldn't be stored in the TT).

        future_positions.push_back(move(pt));

        future_positions_size ++;

        // Test if a winning move was found for the comp or user:

        if (future_evaluation == INT_MAX && is_comp_turn) // so the comp can make a move that wins...
        {
            evaluation = INT_MAX;

            add_position_to_transposition_table(true);

            return;
        }

        if (future_evaluation == INT_MIN && !is_comp_turn) // so the user can make a move that wins for them...
        {
            evaluation = INT_MIN;

            add_position_to_transposition_table(true);

            return;
        }

        if (evaluation == UNDEFINED) // no evaluation for this position yet, so for now:
        {
            evaluation = future_evaluation;

            if (is_child_pruned)
            {
                got_value_from_pruned_child = true; // note that this might change, since there are likely other child nodes of this node to look at.
            }
        }

        else // this position already has an evaluation from a future position previously examined, so I need to see if
        {    // I should replace it with an updated evaluation value from the current future position being examined now.
            if ((future_evaluation > evaluation && is_comp_turn) || (future_evaluation < evaluation && !is_comp_turn))
            {
                evaluation = future_evaluation;

                if (is_child_pruned)
                {
                    got_value_from_pruned_child = true;
                }

                else
                {
                    got_value_from_pruned_child = false;
                }
            }
        }

        // ALPHA-BETA PRUNING:

        // Let's check if this position is a MAX block (comp's turn) or MIN block (user's turn):

        if (is_comp_turn) // This is a MAX block... check beta to try to prune, and possibly reset alpha.
        {
            // SEE IF I CAN PRUNE:
            if (beta != UNDEFINED && beta <= evaluation) // there is a real beta value, and it is <= evaluation.
            {
                // This position's evaluation can only stay the same or get higher, since the comp will find the position
                // with the highest evaluation (this is a MAX node).

                // Already, beta is <= evaluation.

                // Therefore, the user (in the previous MIN node) would not have picked this branch. They
                // would have picked the branch with the value of beta (i.e., lowest value).

                // So, this branch will be TRIMMED.

                evaluation++; // To ensure this branch is not favoured over the previous good branch
                              // with the value of beta (since beta could = evaluation right now). The parent MIN node of this current MAX node will
                              // not choose this node since it can choose a node with at least 1 lower evaluation than this node.

                is_a_pruned_branch = true;

                return;
            }

            // SEE IF ALPHA SHOULD BE RESET (or given a value, if it doesn't have one yet):
            if (alpha == UNDEFINED || evaluation > alpha)
            {
                alpha = evaluation;
            }
        }

        else // user's turn, so MIN block... check alpha to see if branch should be pruned, and possibly reset beta.
        {
            // SEE IF I CAN PRUNE:
            if (alpha != UNDEFINED && alpha >= evaluation) // there is a real alpha value, and it is >= evaluation.
            {
                // This position's evaluation can only stay the same or get lower, since the user will find the position
                // with the lowest evaluation (this is a MIN block).

                // Already, alpha is >= evaluation.

                // Therefore, the comp (in the previous MAX node) would not have picked this branch. It would have picked
                // the branch with the value of alpha (i.e., highest value).

                // So, this branch will be TRIMMED.

                evaluation--; // To ensure this branch is not favoured over the previous good branch
                              // with the value of alpha. The parent MAX node of this current MIN node will
                              // not choose this node since there's another node with at least 1 greater evaluation than this node.

                is_a_pruned_branch = true;

                return;
            }

            // SEE IF BETA SHOULD BE RESET (or given a value, if it doesn't have one yet):
            if (beta == UNDEFINED || evaluation < beta)
            {
                beta = evaluation;
            }
        }
    }

    // Now, before adding this position to the transposition table, ensure that it didn't get its evaluation from a pruned child node:

    if (!got_value_from_pruned_child)
    {
        add_position_to_transposition_table(false); // since at the end of this minimax() function, evaluation has been finalized.
    }
}

void position::smart_evaluation()
{
    initialize_row_barriers(); // implements finished column algorithm, by finding the squares in each
                               // column that is as far as play can possibly go (due to a square allowing comp AND user to win).
                               // These squares will be stored in the private member, "row_barriers", and
                               // row_barriers has 7 elements. row_barriers[0] is the row value of the
                               // highest square allowed for play in column 0,.... row_barriers[6] is the row value of the
                               // highest square allowed for play in column 6.

    vector<vector<char>> copy_board_for_comp = board; // copy of board that will store 'A' for comp's valid amplifying squares to make 4-in-a-row.
    vector<vector<char>> copy_board_for_user = board; // copy of board that will store 'A' for user's valid amplifying squares to make 4-in-a-row.

    vector<coordinate_and_value> info_for_comp_valid_amplifying_squares;
    // Stores the coordinates of each of the comp's valid amplifying squraes (both for 2-in-a-rows and 3-in-a-rows), along with each square's
    // respective value (determined in find_individual_player_evaluation()).

    vector<coordinate_and_value> info_for_user_valid_amplifying_squares;
    // Stores the coordinates of each of the user's valid amplifying squraes (both for 2-in-a-rows and 3-in-a-rows), along with each square's
    // respective value (determined in find_individual_player_evaluation()).

    find_individual_player_evaluation(squares_amplifying_comp_3, squares_amplifying_comp_2, 'C',
                                      copy_board_for_comp, info_for_comp_valid_amplifying_squares);
    find_individual_player_evaluation(squares_amplifying_user_3, squares_amplifying_user_2, 'U',
                                      copy_board_for_user, info_for_user_valid_amplifying_squares);

    double temp_evaluation_as_double = 0.0;

    for (const coordinate_and_value& current: info_for_comp_valid_amplifying_squares)
    {
        if (current.square.row + 1 <= max_row_index && copy_board_for_user[current.square.row + 1][current.square.col] == 'A')
        {
            temp_evaluation_as_double += (static_cast<double>(current.value) * 0.25);
        }

        else if (current.square.row + 2 <= max_row_index && copy_board_for_comp[current.square.row + 2][current.square.col] == 'A')
        {
            temp_evaluation_as_double += (static_cast<double>(current.value) * 0.75);
        }

        else
        {
            temp_evaluation_as_double += current.value;
        }
    }

    for (const coordinate_and_value& current: info_for_user_valid_amplifying_squares)
    {
        if (current.square.row + 1 <= max_row_index && copy_board_for_comp[current.square.row + 1][current.square.col] == 'A')
        {
            temp_evaluation_as_double -= (static_cast<double>(current.value) * 0.25);
        }

        else if (current.square.row + 2 <= max_row_index && copy_board_for_user[current.square.row + 2][current.square.col] == 'A')
        {
            temp_evaluation_as_double -= (static_cast<double>(current.value) * 0.75);
        }

        else
        {
            temp_evaluation_as_double -= current.value;
        }
    }

    evaluation = round(temp_evaluation_as_double);
}

void position::find_individual_player_evaluation(const vector<treasure_spot>& squares_amplifying_3,
                                                const vector<treasure_spot>& squares_amplifying_2, char piece,
                                                vector<vector<char>>& copy_board, vector<coordinate_and_value>& recorder) const
{
    // copy_board should be worked on, storing 'A' in it at the squares creating a 4-in-a-row.
    // recorder is used to store the coordinates and derived values of all unique amplifying squares (both for 2-in-a-row and 3-in-a-row).

    // Use row_barriers private member vector when seeing if an amplifying square should be counted.

    const int big_amount = 10; // how many points to add if there is an amplifying square completes a 3-in-a-row.
    const int small_amount = 3; // NOTE: if you change this to a double, recorder (and its parent in prev. function) should store coordinate and DOUBLE.
                                // and calculations done in this function should be in doubles, not ints whatsoever. how many points to add if an amplifying square completes a 2-in-a-row.
    const int stacked_threat_coefficient = 5; // What to multiply the normal value of a square that let's a 3-in-a-row be
                                              // completed into a 4-in-a-row, if there's a similar square above/below it.

    for (const treasure_spot& space: squares_amplifying_3) // running through squares completing a 3-in-a-row.
    {
        coordinate current_square = space.current_square;

        if (copy_board[current_square.row][current_square.col] == ' ' &&
            (row_barriers[current_square.col] == UNDEFINED || row_barriers[current_square.col] <= current_square.row))
            // Second line applies the finished column algorithm.
        {
            coordinate_and_value temp;

            temp.square = current_square;

            temp.value = big_amount * (current_square.row + 1 + num_pieces_per_column[current_square.col]);

            // I want to check if the square above or below has an 'A' in it, since this means I must have already
            // examined it and found it led to a 4-in-a-row. If this is the case, the computer will have two threats to
            // make a 4-in-a-row lined up on top of each over (which is extremely powerful).

            if ((current_square.row + 1 <= max_row_index && copy_board[current_square.row + 1][current_square.col] == 'A') ||
                (current_square.row - 1 >= 0 && copy_board[current_square.row - 1][current_square.col] == 'A'))
            {
                temp.value *= stacked_threat_coefficient; // since the threat is much more powerful now.
            }

            copy_board[current_square.row][current_square.col] = 'A';

            recorder.push_back(temp);
        }
    }

    for (const treasure_spot& space: squares_amplifying_2) // running through spaces completing a 2-in-a-row.
    {
        coordinate current_square = space.current_square;
        coordinate next_square = space.next_square;
        coordinate other_next_square = space.other_next_square;

        if (copy_board[current_square.row][current_square.col] == ' ' &&
            (row_barriers[current_square.col] == UNDEFINED || row_barriers[current_square.col] <= current_square.row))
        {
            bool is_next_square_in_bounds = is_in_bounds(next_square);
            bool is_other_next_square_in_bounds = is_in_bounds(other_next_square);

            // So the square amplifies a 2-in-a-row and is empty.

            // If one of the next squares is the same piece as those in the 2-in-a-row, then current_square
            // would make a 4-in-a-row if filled in. Add big value to this, and use stacked_threat_coefficient if applicable!

            // Else if one of the next squares is empty (and is accessible via finished column!), just add small value to score.

            // Else, both next squares are filled with the enemy piece, making a 4-in-a-row impossible. Add nothing to the score,
            // so don't even have an else statement (does nothing!).

            // This If-ElseIf situation is dealt with below:

            if ((is_next_square_in_bounds && copy_board[next_square.row][next_square.col] == piece) ||
                (is_other_next_square_in_bounds && copy_board[other_next_square.row][other_next_square.col] == piece))
            {
                coordinate_and_value temp;

                temp.square = current_square;

                // Treat current_square as if it completed a 3-in-a-row into a 4-in-a-row, since filling it ==> wins the game.

                temp.value = big_amount * (current_square.row + 1 + num_pieces_per_column[current_square.col]);

                // I want to check if the square above or below has an 'A' in it, since this means I must have already
                // examined it and found it led to a 4-in-a-row if filled. If this is the case, the computer will have two threats to
                // make a 4-in-a-row lined up on top of each over (which is extremely powerful).

                if ((current_square.row + 1 <= max_row_index && copy_board[current_square.row + 1][current_square.col] == 'A') ||
                    (current_square.row - 1 >= 0 && copy_board[current_square.row - 1][current_square.col] == 'A'))
                {
                    temp.value *= stacked_threat_coefficient; // since the threat is much more powerful now.
                }

                copy_board[current_square.row][current_square.col] = 'A';

                recorder.push_back(temp);
            }

            else if ((is_next_square_in_bounds && copy_board[next_square.row][next_square.col] == ' ' &&
                     (row_barriers[next_square.col] == UNDEFINED || row_barriers[next_square.col] <= next_square.row)) ||
                     (is_other_next_square_in_bounds && copy_board[other_next_square.row][other_next_square.col] == ' ' &&
                     (row_barriers[other_next_square.col] == UNDEFINED || row_barriers[other_next_square.col] <= other_next_square.row)))
            {
                // So current_square only turns a 2-in-a-row into a plain old 3-in-a-row.
                // At least one of the next squares is both empty and accessible (via finished column algorithm), so the
                // 2-in-a-row is at least worth something since it has the POTENTIAL to turn into a 4-in-a-row:

                coordinate_and_value temp;

                temp.square = current_square;

                temp.value = small_amount * (current_square.row + 1 + num_pieces_per_column[current_square.col]);

                // since higher row index ==> lower on the board ==> better threat.
                // don't fill in square in copy_board with 'A', since this square can only complete a 2-in-a-row into a 3-in-a-row.
                // It cannot join a 2-in-a-row with another piece to make a 4-in-a-row, since next_square is empty.

                recorder.push_back(temp);
            }

            // else: both next squares are empty or innaccessible via the finished column algorithm
            // but then I wouldn't add anything to the score since it's impossible to ever create a 4-in-a-row!
        }
    }
}

bool position::did_someone_win() const
{
    // First, ensure that last_move has been initialized.

    if (last_move.row == UNDEFINED || last_move.col == UNDEFINED)
    {
        return false; // this is the starting position...
    }

    // I need to check for 4-in-a-row horizontally, vertically, positive-slope diagonally, negative-slope diagonally.

    // RETURN BASED OFF CALLS TO ALL 4 OF THESE METHODS:

    return (horizontal_four_combo() ||
            vertical_four_combo() ||
            positive_slope_diagonal_four_combo() ||
            negative_slope_diagonal_four_combo());
}

bool position::horizontal_four_combo() const
{
    if (last_move.col + 1 > max_col_index || board[last_move.row][last_move.col + 1] != board[last_move.row][last_move.col])
    {
        return (last_move.col - 3 >= 0 &&
                board[last_move.row][last_move.col - 3] == board[last_move.row][last_move.col] &&
                board[last_move.row][last_move.col - 2] == board[last_move.row][last_move.col] &&
                board[last_move.row][last_move.col - 1] == board[last_move.row][last_move.col]);
    }

    if (last_move.col - 1 < 0 || board[last_move.row][last_move.col - 1] != board[last_move.row][last_move.col])
    {
        return (last_move.col + 3 <= max_col_index &&
                board[last_move.row][last_move.col + 3] == board[last_move.row][last_move.col] &&
                board[last_move.row][last_move.col + 2] == board[last_move.row][last_move.col]);
    }

    return ((last_move.col - 2 >= 0 && board[last_move.row][last_move.col - 2] == board[last_move.row][last_move.col]) ||
            (last_move.col + 2 <= max_col_index && board[last_move.row][last_move.col + 2] == board[last_move.row][last_move.col]));
}

bool position::vertical_four_combo() const
{
    // This one is easy to check, since there can be no pieces above the coordinates of the last move.

    // Therefore, I only need to check if there are 3 pieces under the last move, and if these pieces match the last move.

    return (last_move.row + 3 <= max_row_index &&
            board[last_move.row + 3][last_move.col] == board[last_move.row][last_move.col] &&
            board[last_move.row + 2][last_move.col] == board[last_move.row][last_move.col] &&
            board[last_move.row + 1][last_move.col] == board[last_move.row][last_move.col]);
}

bool position::positive_slope_diagonal_four_combo() const
{
    if (last_move.row - 1 < 0 ||
        last_move.col + 1 > max_col_index ||
        board[last_move.row - 1][last_move.col + 1] != board[last_move.row][last_move.col])
    {
        return (last_move.row + 3 <= max_row_index &&
                last_move.col - 3 >= 0 &&
                board[last_move.row + 3][last_move.col - 3] == board[last_move.row][last_move.col] &&
                board[last_move.row + 2][last_move.col - 2] == board[last_move.row][last_move.col] &&
                board[last_move.row + 1][last_move.col - 1] == board[last_move.row][last_move.col]);
    }

    if (last_move.row + 1 > max_row_index ||
        last_move.col - 1 < 0 ||
        board[last_move.row + 1][last_move.col - 1] != board[last_move.row][last_move.col])
    {
        return (last_move.row - 3 >= 0 &&
                last_move.col + 3 <= max_col_index &&
                board[last_move.row - 3][last_move.col + 3] == board[last_move.row][last_move.col] &&
                board[last_move.row - 2][last_move.col + 2] == board[last_move.row][last_move.col]);
    }

    return ((last_move.row + 2 <= max_row_index &&
             last_move.col - 2 >= 0 &&
             board[last_move.row + 2][last_move.col - 2] == board[last_move.row][last_move.col])
            ||
            (last_move.row - 2 >= 0 &&
             last_move.col + 2 <= max_col_index &&
             board[last_move.row - 2][last_move.col + 2] == board[last_move.row][last_move.col]));
}

bool position::negative_slope_diagonal_four_combo() const
{
    if (last_move.row + 1 > max_row_index ||
        last_move.col + 1 > max_col_index ||
        board[last_move.row + 1][last_move.col + 1] != board[last_move.row][last_move.col])
    {
        return (last_move.row - 3 >= 0 &&
                last_move.col - 3 >= 0 &&
                board[last_move.row - 3][last_move.col - 3] == board[last_move.row][last_move.col] &&
                board[last_move.row - 2][last_move.col - 2] == board[last_move.row][last_move.col] &&
                board[last_move.row - 1][last_move.col - 1] == board[last_move.row][last_move.col]);
    }

    if (last_move.row - 1 < 0 ||
        last_move.col - 1 < 0 ||
        board[last_move.row - 1][last_move.col - 1] != board[last_move.row][last_move.col])
    {
        return (last_move.row + 3 <= max_row_index &&
                last_move.col + 3 <= max_col_index &&
                board[last_move.row + 3][last_move.col + 3] == board[last_move.row][last_move.col] &&
                board[last_move.row + 2][last_move.col + 2] == board[last_move.row][last_move.col]);
    }

    return ((last_move.row - 2 >= 0 &&
             last_move.col - 2 >= 0 &&
             board[last_move.row - 2][last_move.col - 2] == board[last_move.row][last_move.col])
            ||
            (last_move.row + 2 <= max_row_index &&
             last_move.col + 2 <= max_col_index &&
             board[last_move.row + 2][last_move.col + 2] == board[last_move.row][last_move.col]));
}

bool position::is_acceptable_letter(char c) const
{
    return ((c >= 'A' && c <= 'G') || (c >= 'a' && c <= 'g'));
}

bool position::is_element_in_vector(const vector<vector<vector<char>>>& vec, const vector<vector<char>>& element) const
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec[i] == element)
        {
            return true;
        }
    }

    return false;
}

coordinate position::find_starting_horizontal_point() const
{
    char piece = board[last_move.row][last_move.col];

    coordinate left_most_point = last_move;

    while (true)
    {
        if (left_most_point.col - 1 < 0 || board[left_most_point.row][left_most_point.col - 1] != piece)
        {
            return left_most_point; // since the next square over left is either out-of-bounds or not equal to piece.
        }

        left_most_point.col --;
    }
}

coordinate position::find_ending_horizontal_point() const
{
    char piece = board[last_move.row][last_move.col];

    coordinate right_most_point = last_move;

    while (true)
    {
        if (right_most_point.col + 1 > max_col_index || board[right_most_point.row][right_most_point.col + 1] != piece)
        {
            return right_most_point; // since the next square over right is either out-of-bounds or not equal to piece.
        }

        right_most_point.col ++;
    }
}

coordinate position::find_starting_vertical_point() const
{
    return last_move;
}

coordinate position::find_ending_vertical_point() const
{
    char piece = board[last_move.row][last_move.col];

    coordinate bottom_most_point = last_move;

    while (true)
    {
        if (bottom_most_point.row + 1 > max_row_index || board[bottom_most_point.row + 1][bottom_most_point.col] != piece)
        {
            return bottom_most_point; // since the next square down under is either out-of-bounds or not equal to piece.
        }

        bottom_most_point.row ++;
    }
}

coordinate position::find_starting_positive_slope_diagonal_point() const
{
    char piece = board[last_move.row][last_move.col];

    coordinate bottom_left_most_point = last_move;

    while (true)
    {
        if (bottom_left_most_point.row + 1 > max_row_index ||
            bottom_left_most_point.col - 1 < 0 ||
            board[bottom_left_most_point.row + 1][bottom_left_most_point.col - 1] != piece)
        {
            return bottom_left_most_point; // since the next square over down-left is either out-of-bounds or not equal to piece.
        }

        bottom_left_most_point.row ++;
        bottom_left_most_point.col --;
    }
}

coordinate position::find_ending_positive_slope_diagonal_point() const
{
    char piece = board[last_move.row][last_move.col];

    coordinate top_right_most_point = last_move;

    while (true)
    {
        if (top_right_most_point.row - 1 < 0 ||
            top_right_most_point.col + 1 > max_col_index ||
            board[top_right_most_point.row - 1][top_right_most_point.col + 1] != piece)
        {
            return top_right_most_point; // since the next square up-right is either out-of-bounds or not equal to piece.
        }

        top_right_most_point.row --;
        top_right_most_point.col ++;
    }
}

coordinate position::find_starting_negative_slope_diagonal_point() const
{
    char piece = board[last_move.row][last_move.col];

    coordinate top_left_most_point = last_move;

    while (true)
    {
        if (top_left_most_point.row - 1 < 0 ||
            top_left_most_point.col - 1 < 0 ||
            board[top_left_most_point.row - 1][top_left_most_point.col - 1] != piece)
        {
            return top_left_most_point; // since the next square up-left is either out-of-bounds or not equal to piece.
        }

        top_left_most_point.row --;
        top_left_most_point.col --;
    }
}

coordinate position::find_ending_negative_slope_diagonal_point() const
{
    char piece = board[last_move.row][last_move.col];

    coordinate bottom_right_most_point = last_move;

    while (true)
    {
        if (bottom_right_most_point.row + 1 > max_row_index ||
            bottom_right_most_point.col + 1 > max_col_index ||
            board[bottom_right_most_point.row + 1][bottom_right_most_point.col + 1] != piece)
        {
            return bottom_right_most_point; // since the next square over down-right is either out-of-bounds or not equal to piece.
        }

        bottom_right_most_point.row ++;
        bottom_right_most_point.col ++;
    }
}

