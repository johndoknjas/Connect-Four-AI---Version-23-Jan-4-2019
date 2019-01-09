/* Version 23

- Fix main.cpp playout_game() function. Make it like the Versus simulator. The inconsistency with displaying the board
        before/after doing calculations (at !found_it) are why the "hiccup" is noticeable when the comp actually has to spend
        a bit of time on find_best_move() on the next iteration.
            Nothing WRONG with playout_game(). Just can be better user experience.   DONE

- Also, changed (7) to (9), for the call to find_quick_winning_move from comp's perspective.
    - Testing this change in Versus Sim. to ensure find_best_move doesn't ever take too long.

- Finally, also accepting moves leading to a future position with a greater (not just equal) evaluation than the current position
  in find_best_move().

Done Version 23!

  RESULTS AGAINST VERSION 20:
    - Match with time_limit of 0.30:
        - Even score overall. Out of 200 trials there were 9 uneven ones, but this is expected due to time_limit.
        - Average number of moves in games where V.23 won was 30.6437 moves.
        - Average number of moves in games where V.20 won was 31.8621 moves.
        - Max amount of time V.23 spent on one call to find_best_move() was 0.156213 seconds.
        - On average, V.23 spent a millisecond or less each time on find_best_move().

    - Match with depth_limit of 8:
        - Even score, and out of 200 trials every single trial was even. No randomness due to depth_limit.
            - V.23's future_positions[i]->evaluation >= evaluation didn't seem to give any benefits, but also didn't do damage either.
        - Average number of moves in games where V.23 won was 31.8261 moves.
        - Average number of moves in games where V.20 won was 32.6211 moves.
        - Max amount of time V.23 spent on one call to find_best_move() was 0.124971 seconds.
        - On average, V.23 spent a millisecond or less each time on find_best_move().

From these match results, it's clear that:
    - V.23 and V.20 are exactly equal in strength.
    - V.23 is able to win faster and/or delay the game when it's losing, such that the games it wins are on average
      roughly 1 move shorter than the games V.20 wins.
    - V.23 never spent more than 0.156213 seconds in one call to find_best_move(), and on average spends less than a millisecond. This is
      good!




    ******IDEAS FOR FUTURE VERSIONS******:

      - If a player has a square amplifying a 2-in-a-row, do not count it if the opponent has a square the makes a 4-in-a-row directly above it.
            - Reason: if the player actually uses this square, the opponent wins immediately.
            - Of course, if the player's square amplifies a 2-in-a-row and connects with at least a single piece, then obviously count it
              since it creates a 4-in-a-row.
            - Note that it's fine if a next_square is directly below one of the opponent's squares making a 4-in-a-row. Since here,
              next_square isn't the one adding to evaluation. It's only there to see if the player's square amplifying a 2-in-a-row
              could POTENTIALLY win by filling in next_square (or already would make a 4-in-a-row if next_square is filled).
                - In this potential scenario, filling in next_square wins immediately, so the opponent's amplifying square above is moot.
            - You'll do this probably near the end of smart_evaluation():
                - Go through each player's amplifying_2_vectors. If the square does not have an 'A' in the player's copy_board
                  (or any other char representing a square amplifying a 3-in-a-row?), then it is a normal square amplifying a 2-in-a-row.
                - If so, then check the square directly above in the OPPONENT'S copy_board. If this square is in-bounds and has
                  an 'A', then that square would give the opponent a 4-in-a-row.
                - So don't give points to the user.

      - Make a third version of the "Generating File of Playable Positions" project. The current version of it right now uses
        a Version you made in October to select positions.
            - In the new version, replace the engine with this V.23 Engine. Also allow for positions with up to 8 pieces to be generated,
              and not just an even number of pieces! (although maybe you already allow for odd numbers, I don't know).
            - Then, after running overnight, copy the file of positions generated and replace the playable positions text file in this project.

      - For something like:    | | |X|X| | |

        - Count ALL FOUR of the empty squares as squares amplifying a 2-in-a-row....
            - The inner 2 squares you obviously already count, and they each have two next_squares.
            - The 2 outer squares you do not count yet. They only have ONE next_square (the respective inner square), and "other_next_square"
              is UNDEFINED.

      - For something like:  | |X| |X| |

        - Count ALL THREE of the empty squares as squares amplifying a 2-in-a-row.
            - You already count the inner square, which has two next_squares.
            - The two other squares you do not count yet. They each only have one next_square (the inner square).

      -  X
          |  |
            |  |
               X

            In the diagram above, count each of the squares as their own "amplifying 2-in-a-row" squares, since if it and its
            neighbor get filled, a 4-in-a-row is created.
                - If its neighbour (i.e., its "next_square") gets filled with an enemy piece do not count square.
                - If its neighbour gets filled with another X, obviously count the square as amplifying a 3-in-a-row, as you already do.
                - Not sure what "other_next_square" would be for each of these amplifying squares. Probably UNDEFINED.

      - squares amplifying a 2-in-a-row that can be filled in ONE MOVE should be treated differently (either positively or negatively).
        Change quiescent search requirements to also allow no squares amplifying a 2-in-a-row to be able to be filled immediately?

      - squares amplfying a 2-in-a-row that cannot ever make a 4-in-a-row (since outer 2 squares are filled with enemy piece
        or out-of-bounds), should NOT be counted.

      - Go through main.cpp and make it better? Esp. in play_game() -- make it think for 0 seconds where appropriate.

      - Play with adjusting the 0.5 and 0.75 parameters in smart_evaluation().
            - For the 0.75, I've confirmed it's most likely beneficial to have (as opposed to no parameter for it), but play with increasing
              it to something like 0.9.
                - With the parameter at 0.75, Version 17 did well against Version 16. Without the parameter (so effectively 1.0),
                  Version 17 still did well against Version 16 but not quite as well.
                - I'm assuming there may be a parabola here, with the absolute max between 0.75 and 1.0.
            - For the 0.5, not sure if it would be better raised or lowered.
            - If possible, try to fit data points to some function.

      - For evaluating a position, give a small amount to the player whose turn it is to move in that position. Since the Engines now think
        according to a time limit and not a depth limit, some positions will have comp to move and some will have user to move.
            - Actually, even when depth limit was used for thinking, quiescence search sometimes/often caused the search to go further.
            - So even then, giving the side to move a small amount (like +/- 5 or something) would have been useful.

      -	For a stacked threat of two 3-in-a-rows above the opponent’s own amplifying square for a 3-in-a-row,
            ensure the two groups have equal value, if you can!

            -	The group of two above should NOT be worth more.

      //////////////////////////////////////////////////////////////////

      - If comp is winning find shortest path to win. If for some reason this is way too hard to do, at least make the computer
        choose a move if it wins on the spot!

      - If comp is losing, play the move that takes the longest to lose. Even though the comp doesn't go for any lose-in-1 if it can
        help it, it should be putting up the tougest resistance (not just the "not easiest" resistance).

      - Used anything you learn in CMPT 225 (open-indexing hash table, quadratic hashing, etc).

      - Add a bool attribute storing whose turn it is for the "position_info_for_TT" struct. Then when adding a position to the TT
        (in the add_position_to_transposition_table() function) and when checking if a position is in the TT, make sure to include
        this bool variable.
            - The reason is to avoid treating a position the same if its the comp's turn in one and the user's turn in the other.
              This should never happen since in the 2nd constructor (which includes whenever a new game starts), the TT is reset,
              apparently including positions even with INT_MAX or INT_MIN evaluations, although double check this.

            - So while there should never be a problem, it wouldn't hurt to include the bool variable, just in case you've missed something.


      - Apparently the order of squares in each of the amplifying vectors affects the evaluation calculated
        in smart_evaluation(). See where this could be the case, and fix it if possible.
            - It should be noted though that with transposition tables, the same position (arriving via a different
              move, so different order of amplifying squares) will never have to be evaluated twice.


      - Only randomize the possible_moves vector in constructors 1 and 2 (since the position object created in them
        directly decides what move the comp plays in the actual game).
            - In constructor 3, optimize the order of moves in possible_moves. The thousands of positions created
              in constructor 3 are just for the minimax process (i.e., finding evaluations). It doesn't matter how these
              evaluations/numbers are found, so efficiency is key here. The order of possible_moves for position objects
              created in Constructor 3 don't affect the move ultimately chosen by the computer, just the speed by which
              it happens.

            - A few optimization techniques (Technique 2 is an idea for Version 10, and may allow depth_limit to
                                             be increased):
                1) Each time in analyze_last_move(), re-arrange the possible_moves vector to store moves that can
                   make a 4-in-a-row immediately.
                   DONE

                2) In the 3rd constructor (NOT THE 2ND AND 1ST!!) re-arrange the possible_moves vector to store moves
                   that, on average, tend to end up being the chosen move in the minimax calculations. To figure this out:

			  STRATEGY #1:
			  -------------
			  - Favour moves that are closer to the middle column (D) and have more pieces in a 3x3 square around them (where the move
                            is in the centre of the square).
				- Not sure whether to include all pieces ('C' and 'U') when counting in the 3x3 square, or just similar pieces.

			  - Maybe favour moves that are lower on the board (closer to the bottom row)? Not so sure about this one.....

			  - To discover more traits of good moves, see Strategy #2.



			  STRATEGY #2 (Investigating via Simulation):
			  --------------------------------------------
                        - Run a Versus Simulation, and for one (or both) of the Engines,
                          each time a move is "chosen" in its minimax process, record its
                          column & row in some static variables. You could even record the data to a file that keeps growing
                          in size over multiple simulations, giving you more and more accurate data.
                        - Note that each possible_move should be given equal fair chances to be examined sooner in the
                          minimax process. So, make sure to randomize the possible_moves vector in the 3rd constructor!
                          This won't allow randomness to affect what move an Engine actually chooses in the game, but it
                          will allow each move in possible_moves to have a fair chance of being looked at first.
                            - Then, if, for example, a particular column is "chosen" more in the minimax process, this
                              tells you it's because that column tends to be better on average, rather than because of
                              the order of moves in possible_moves.
                        - Then at the end, print all this data out (or print some averages from the data in the file).
                          This should give you an idea of which columns & rows tend
                          to be more successful (and thus should be tried first by being at the front of the possible_moves
                          vector).
                            - Note that the row value is relative compared to other moves available. For example, if a chosen
                              move has row value of 4, record how much higher/lower it is than the other moves that
                              could have been chosen. Do not just record "4".

                NOTE: Optimization Technique 1) and 2) are constantly overwriting each other...
                    - Technique 2) in Constructor 3 re-arranges possible_moves with moves that work on average. Then it
                      calls analyze_last_move() and...
                    - Technique 1) in analyze_last_move() puts moves that concretely work in the current position at the front.
                    - THIS IS GOOD. The result is a possible_moves vector with any moves that concretely work in the position
                      at the front of the vector, followed by moves that have a good chance of working on average, followed
                      by the scrappy moves at the back. Alpha-beta and minimax pruning should greatly work here!


        **NOTE**: The suggestion below is kind of useless if you do iterative deepening, since the depth searched is just how much
        time is available in an arbitrary time constraint you give to the Engine.

      - For the position class, make a static constant variable called X that equals breadth^depth, or 7^depth_limit. This is theoretically how many
        moves the Engine looks at when it thinks, if it weren't for any minimax/alpha beta pruning.
		- Then, each time the opponent moves and the Engine thinks of the new position (i.e., Constructor 2), check the size of possible_moves vector.
                - If possible_moves vector's size is less, keep incrementing depth_limit as long as possible_moves.size()^depth_limit is close to X.
		- This essentially lets the Engine increase its search depth, if one or more column(s) are full (decreasing breadth).
                - But, the Engine can never much slower than it was before, which is where the check for possible_moves.size()^depth_limit being close to X comes in.
                - By "close to X" I mean less than 2X ish. As long as the runtime isn't double as long, it's fine, since the engine is usually pretty quick.
                - Note that this means depth_limit should still be static, but cannot be const anymore since it sometimes gets updated in Constructor 2.

		- IMPORTANT UPDATE: Alpha-beta pruning is more effective when the depth limit is higher. So, let's say there were 7 possible moves, and depth limit
                  was 9. Theoretical max value of #moves = 7^9. Then, let's say there's only 3 possible moves, so we make depth limit = 20...
                  New theoretical max value of #moves = 3^20, which is 86 times greater than 7^9.

		  HOWEVER: if we assume Alpha-Beta pruning cuts down search depth by half (on average), then we get 7^4.5 and 3^10 for average number of moves
                  looked at. 3^10 is only 9.3 times greater than 7^4.5. So still not ideal whatsoever, but not nearly as drastic as 86 times greater.

                  So, incorporate this in. Maybe make a new search depth limit acceptable if, after alpha-beta prunes around half the search depth, the new average
                  #moves is <= 2x #moves at the start of the game with alpha-beta pruning. This will require trial and error, since I don't know
                  how effective alpha-beta pruning is on the search depth for my engine.

      - Let's say one square amplifies 3 'U', allowing user to win the game. Then assume the square directly above it amplifies 3 'C', allowing comp to
        win the game. The square for the comp is MUCH less valuable than the square for the user, and to explain consider the 2 possible scenarios:
		- Scenario 1) Comp is forced via zugzwang to move under the user square, allowing the user to win the game.
                - Scenario 2) User is forced via zugzwang to move under the user square, allowing comp to fill the user square, and then user fills the
                              comp square. No one wins from the interaction (okay, maybe there is ANOTHER comp square above, but two winning squares
                              directly stacked is handled by the evaluation function).

                You'll have to do some experimentation to see how much less to make the upper square worth than the lower square. Note that...
			- You should probably evaluate the lower square the same, since its value isn't affected whatsoever by the upper square.
                        - However, the upper square should be given a lesser value, since it is impossible for it to ever be used to win the game (for whichever
                          player is being considered to use it at the moment in the smart evaluation function).

      - Keep the amplifying vectors constantly sorted. Should be sorted in some fashion so that the lower squares for each
        column are looked at first.
            - When you create a new position object (going 1 depth deeper into calculations), then new amplifying squares you add
              to the amplifying vectors should be placed in sorted order.
            - Insertion sort will be great for this! Since the amplifying vectors are going to be always kept sorted in the aforementioned
              fashion.

            - Note that in the think_ functions (which are called from main()), it is necessarily guaranteed the passed
              in amplifying vector params will be sorted in the desired fashion. So in these think_ functions,
              make sure the comp sorts the passed in amplifying vectors (stl sort works best here, since it should be assumed
              the amplifying vectors aren't at all sorted).

      - A "when push comes to shove" idea for squares making a column a "finished column".
            - For this kind of square, both and user and comp are threatening a 4-in-a-row.
            - But if this square is ever utilized to seize a win, only the comp or user will get to take advantage of it.
            - So, it makes sense to figure out which player will actually be able to use the square, should one of the players
              eventually end up in zugzwang.

            - This can be easily figured out by counting the number of empty squares (not including the winning square and directly above it)
              and factoring in whose turn it is currently.
                 - Example: If it's the comp's turn and there are an odd number of squares, then if the winning square gets used
                   it will happen when the comp has to move in the square under the winning square (due to zugzwang) and the user
                   places on the winning square. So only count the winning square as an advantage for the user!
                        - Still make a column remain a finished column, of course.

            - **IMPORTANT EXCEPTION**: If the square directly under the winning square is an amplifying square into a 4-in-a-row
              for one of the players, then discard the above algorithm. Things have completely changed now.

                - If the square directly under COULD become an amplifying square for a 4-in-a-row, then things are a bit tricky:
                    - You could run the above algorithm. Then, for whoever is the "victor" of that algorithm, check if
                      the opponent could possibly make the square directly under a winning square in the future.
                        - If the opponent can, then discard the algorithm.

      - Interesting idea: I'm guessing that in smart_evaluation(), a temp_board is used to store 'A' chars in squares
              that win for one of the players. Instead, make this temp_board store enums in each square, for the following options:
                - An enum if the square has a 'C'
                - An enum if the square has a 'U'
                - An enum representing what the square does for the comp and user. Possibilites for each of them:
                    - Square could amplifying a 4-in-a-row for a player, or possibly amplify a 4-in-a-row in the future,
                      or NEVER be able to amplify a 4-in-a-row in the future.
                    - Makes 9 enums, since these 3 possibilities are for each player. Example enum value:
                      "AmplifiesCompAndNothingUser".  Means square wins for comp, and can never do anything for the user.

                - With these enums, it will now be straightforward to check if a square wins for both user and comp (causing finished column
                  and meaning the whole above algorithm runs), as well as straightforward to check the status of whether
                  a square could possibly be a 4-in-a-row for one of the players (useful for the square underneath a finished column square,
                  in the above algorithm).

                - NOTE: If making temp_board store enums is too inefficient (either time or space wise), you could continue using chars,
                  as long as you remember what char means what for the 9 possibilities for each empty square.

      - Maybe only clean up the amplifying vectors at something like depth 6 instead of at depth 0 in 2nd constructor?
        Reason is that there are about 7 times more depth 7 positions than depth 6 ones, and for every depth 6 position
        that you clean up, its 7 depth 7 child positions will be evaluated faster.

            - Note that this logic can be continually applied backwards. Maybe it's best to clean up at every depth level?
              Try to mathemtically prove something to help here.


        **NOTE**: The bottom suggestion is probably useless when using iterative deepening, since iterative deepening
        sorts moves from likely best to likely worst, based off hash table giving move orders of positions that were looked at
        at one less depth.

       - Get the possible_moves vector in position.h to store moves in this order: D,E,C,F,B,G,A
         It goes from the middle out, since in general moves in the middle tend to be better. This will allow the computer
         to find better moves quicker, allowing more alpha-beta pruning.
            - Also, stop randomizing possible_moves' order on each instantiation in constructors 2 & 3! Not efficient!

*/
