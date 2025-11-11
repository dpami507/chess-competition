#include "chess-simulator.h"
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
#include "chess.hpp"
#include <random>
#include <algorithm>
using namespace ChessSimulator;
using namespace chess;

int moveCount = 0;

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
    -50, -40, -30, -30, -30, -30, -40, -50 
};
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

int getBoardScore( Board& board)
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
        int index = (color == Color::WHITE) ? static_cast<int>(sq)
            : 63 - static_cast<int>(sq);
        //int index = sq;

        int pieceScore = 0;

        if (type == PieceType::KING)
        {
            pieceScore += 20000;
            pieceScore += PSQT_king[index];
        }
        else if (type == PieceType::QUEEN)
        {
            pieceScore += 900;
            pieceScore += PSQT_queen[index];
        }
        else if (type == PieceType::ROOK)
        {
            pieceScore += 500;
            pieceScore += PSQT_rook[index];
        }
        else if (type == PieceType::BISHOP)
        {
            pieceScore += 330;
            pieceScore += PSQT_bishop[index];
        }
        else if (type == PieceType::KNIGHT)
        {
            pieceScore += 320;
            pieceScore += PSQT_knight[index];
        }
        else if (type == PieceType::PAWN)
        {
            pieceScore += 100;
            pieceScore += PSQT_pawn[index];
        }

        score += (color == Color::WHITE) ? pieceScore : -pieceScore; // I think this is the correct line since it takes into account both colors
        //score += pieceScore;
    }


    return score;
}

bool GameOver(const Board& board)
{
    switch (board.isGameOver().first)
    {
    case GameResultReason::NONE:
        return false;
        break;
    default:
        moveCount = 0;
        return true;
        break;
    }
}

std::pair<int, chess::Move> newMiniMax(Board& board, int depth, bool getMax, int alpha, int beta)
{
    moveCount++;
    //Base Case
    if (depth == 0 || GameOver(board))
    {
        int score = getBoardScore(board);
        return { getMax ? score : -score, chess::Move::NO_MOVE};
    }

    //List of all legal move that can be made
    Movelist moves;
    movegen::legalmoves(moves, board);

    if(moves.empty())
    {
        return { getBoardScore(board), chess::Move::NO_MOVE };
    }

    chess::Move bestMove = moves[0];

    //If its whites turn
    if (getMax)
    {
        int maxEval = -1000000; // the best move placeholder

        //chess::Move bestMove = moves[0];

        for (auto move : moves)
        {
            board.makeMove(move);
            auto eval = newMiniMax(board, depth - 1, false, alpha, beta);
            board.unmakeMove(move);


            if (eval.first > maxEval)
            {
                maxEval = eval.first;
                bestMove = move;
            }

            // alpha beta pruning dlc
            alpha = std::max(alpha, eval.first);
            if (beta <= alpha) {
                //std::cout << "WHITE Pruned at depth " << depth << "\n";
                break;
            }
        }

        return { maxEval, bestMove };
    }
    else
    {
        int minEval = 1000000;

        //chess::Move bestMove = moves[0];

        for (auto move : moves)
        {
            board.makeMove(move);
            auto eval = newMiniMax(board, depth - 1, true, alpha, beta);
            board.unmakeMove(move);

            if (eval.first < minEval)
            {
                minEval = eval.first;
                bestMove = move;
            }

            // alpha beta pruning dlc
            beta = std::min(beta, eval.first);
            if (beta <= alpha) {
                //std::cout << "BLACK Pruned at depth " << depth << "\n";
                break;
            }
        }

        return { minEval, bestMove };
    }
}

// old function that isn't used at all
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

    //Better Heuristics and alpha-beta pruning (!! THERES A SEBASTIAN LEAUGE VIDEO !!)

  Board board(fen);
  Movelist moves;

  //Check if there are any legal moves
  movegen::legalmoves(moves, board);
  if(moves.size() == 0)
    return "";

  //WARNING IF YOU SET DEPTH TO 4 OR GREATER YOU COMPUTER WILL PROBABLY DIE
  //Wow its great and it works but BY GOD IS IT SLOW welp we gotta implement alpha-beta pruning
  //std::pair<int, int> mM = miniMax(0, 0, true, 2, board);
  //std::cout << "Minimax Index: " << mM.second << std::endl;
  //std::cout << "Minimax Score: " << mM.first << std::endl;

  auto newMM = newMiniMax(board, 4, board.sideToMove() == chess::Color::WHITE, -1000000, 1000000);
  std::cout << "========================\n";
  std::cout << "Best Move is: " << chess::uci::moveToUci(newMM.second) << std::endl;
  std::cout << "Best Score is: " << newMM.first << std::endl;
  std::cout << moveCount << std::endl;

  // Check if the move is valid
  if (newMM.second == chess::Move::NO_MOVE) {
      return "";  // No valid move
  }


  return chess::uci::moveToUci(newMM.second);

  // get random move
  //std::random_device rd;
  //std::mt19937 gen(rd());
  //std::uniform_int_distribution<> dist(0, moves.size() - 1);
  //auto move = moves[dist(gen)];
  //return chess::uci::moveToUci(move);
}