#include "chess-simulator.h"
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
#include "chess.hpp"
#include <random>
using namespace ChessSimulator;
using namespace chess;

// https://www.chessprogramming.org/Simplified_Evaluation_Function
// We did some borrowing from this handy dandy website
const int PSQT_pawn[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5, 5, 10, 25, 25, 10, 5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0
};
const int PSQT_knight[] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50 };
const int PSQT_bishop[] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};
const int PSQT_rook[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0
};
const int PSQT_queen[] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};
const int PSQT_king[] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20, 0, 0, 0, 0, 20, 20,
    20, 30, 10, 0, 0, 10, 30, 20
};

int getBoardScore(Board board)
{
    int score = 0;

    //Go through all the squares
    for (size_t sq = 0; sq < 64; sq++)
    {
        //Get the square and piece on that square
        Square square = static_cast<Square>(sq);
        Piece piece = board.at(square);

        //Get piece characteristics
        if (piece == Piece::NONE) continue;
        PieceType type = piece.type();
        Color color = piece.color();

        //Index for the PSQT_tables 
        int index = (color == Color::WHITE) ? sq : (63 - sq);
        int pieceScore = 0;

        if (type == PieceType::underlying::KING)
        {
            pieceScore += 20000;
            pieceScore += PSQT_king[index];
        }
        else if (type == PieceType::underlying::QUEEN)
        {
            pieceScore += 900;
            pieceScore += PSQT_queen[index];
        }
        else if (type == PieceType::underlying::ROOK)
        {
            pieceScore += 500;
            pieceScore += PSQT_rook[index];
        }
        else if (type == PieceType::underlying::BISHOP)
        {
            pieceScore += 330;
            pieceScore += PSQT_bishop[index];
        }
        else if (type == PieceType::underlying::KNIGHT)
        {
            pieceScore += 320;
            pieceScore += PSQT_knight[index];
        }
        else if (type == PieceType::underlying::PAWN)
        {
            pieceScore += 100;
            pieceScore += PSQT_pawn[index];
        }

        std::cout << "Score is " << pieceScore << " for the piece " << piece << " at the location " << square << "\n";

        score += (color == Color::WHITE) ? pieceScore : -pieceScore;
    }

    return score;
}

std::pair<int, int> miniMax(int currentDepth, int index, bool getMax, int maxDepth, chess::Board board) {

    //List of all legal move that can be made
    Movelist moves;
    movegen::legalmoves(moves, board);

    int bestScore = (getMax) ? -100000 : 100000;
    int bestIndex = 0;

    //Base Case
    if (currentDepth == maxDepth || moves.empty()) 
    { 
        //Get the score of where we are 
        int score = getBoardScore(board); 
        
        // Return the pair of score and index 
        return { score, index }; 
    } 

    //For every move 
    for (chess::Move move : moves) 
    { 
        //Make the move on a new board
        Board tempBoard = board;
        tempBoard.makeMove(move); 

        std::pair<int, int> mM = miniMax(currentDepth + 1, index * 2, !getMax, maxDepth, tempBoard);

        if (getMax)
        {
            if (mM.first > bestScore)
            {
                bestScore = mM.first;
                bestIndex = mM.second;
            }
        }
        else 
        {
            if (mM.first < bestScore)
            {
                bestScore = mM.first;
                bestIndex = mM.second;
            }
        }
    } 

    std::cout << "Using best score of " << bestScore << "\n";
    return { bestScore, bestIndex };
}

std::string ChessSimulator::Move(std::string fen) {
  // create your board based on the board string following the FEN notation
  // search for the best move using minimax / monte carlo tree search /
  // alpha-beta pruning / ... try to use nice heuristics to speed up the search
  // and have better results return the best move in UCI notation you will gain
  // extra points if you create your own board/move representation instead of
  // using the one provided by the library

  // here goes a random movement
  Board board(fen);
  Movelist moves;
  movegen::legalmoves(moves, board);

  if(moves.size() == 0)
    return "";

  //WARNING IF YOU SET DEPTH TO 4 OR GREATER YOU COMPUTER WILL PROBABLY DIE
  //Wow its great and it works but BY GOD IS IT SLOW welp we gotta implement alpha-beta pruning
  std::pair<int, int> mM = miniMax(0, 0, true, 2, board);
  std::cout << "Minimax Index: " << mM.second << std::endl;
  std::cout << "Minimax Score: " << mM.first << std::endl;

  // get random move
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, moves.size() - 1);
  //auto move = moves[dist(gen)];
  auto move = moves[mM.second];
  return chess::uci::moveToUci(move);
}