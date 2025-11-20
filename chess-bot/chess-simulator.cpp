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
    int materialScore = 0;
    int score = 0;
    chess::Color sideToMove = board.sideToMove();

    bool inCheck = board.inCheck();

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

        const int KING_SCORE = 20000;
        const int QUEEN_SCORE = 900;
        const int ROOK_SCORE = 500;
        const int BISHOP_SCORE = 330;
        const int KNIGHT_SCORE = 320;
        const int PAWN_SCORE = 100;


        if (type == PieceType::KING)
        {
            pieceScore += KING_SCORE;
            pieceScore += PSQT_king[index];
        }
        else if (type == PieceType::QUEEN)
        {
            pieceScore += QUEEN_SCORE;
            pieceScore += PSQT_queen[index];
        }
        else if (type == PieceType::ROOK)
        {
            pieceScore += ROOK_SCORE;
            pieceScore += PSQT_rook[index];
        }
        else if (type == PieceType::BISHOP)
        {
            pieceScore += BISHOP_SCORE;
            pieceScore += PSQT_bishop[index];
        }
        else if (type == PieceType::KNIGHT)
        {
            pieceScore += KNIGHT_SCORE;
            pieceScore += PSQT_knight[index];
        }
        else if (type == PieceType::PAWN)
        {
            pieceScore += PAWN_SCORE;
            pieceScore += PSQT_pawn[index];
        }

        /////////////////////////////////////////////////

        //Checks if the space has enemy pieces attacking it or ally pieces defending it
        Movelist moves;
        movegen::legalmoves(moves, board, sideToMove);

        //set attacker color
        Color attackerColor;
        if (color == Color::WHITE)
            attackerColor = Color::BLACK;
        else if (color == Color::BLACK)
            attackerColor = Color::WHITE;

        //check if a space is attacked
        Bitboard attackers = attacks::attackers(board, attackerColor, square);
        int numAttackers = attackers.count();

        //check if a space is defended
        Bitboard defenders = attacks::attackers(board, color, square);
        int numDefenders = defenders.count();

        int dangerScore = 0;

        if (numAttackers >= numDefenders)
        {
            if (type == PieceType::KING)
                dangerScore -= KING_SCORE;
            else if (type == PieceType::QUEEN)
                dangerScore -= QUEEN_SCORE;
            else if (type == PieceType::ROOK)
                dangerScore -= ROOK_SCORE;
            else if (type == PieceType::BISHOP)
                dangerScore -= BISHOP_SCORE;
            else if (type == PieceType::KNIGHT)
                dangerScore -= KNIGHT_SCORE;
            else if (type == PieceType::PAWN)
                dangerScore -= PAWN_SCORE;
        }

        ////////////////////////////////////////////////////////////////////////

        //Subtract other opponets pieces while adding score 
        if(sideToMove == Color::WHITE)
            materialScore += (color == Color::WHITE) ? pieceScore : -pieceScore;
        else
            materialScore += (color == Color::WHITE) ? -pieceScore : pieceScore;

        score += pieceScore;
        score += dangerScore;
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

std::pair<int, chess::Move> alphaBetaPruning(Board& board, int depth, bool getMax, int alpha, int beta)
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
            auto eval = alphaBetaPruning(board, depth - 1, false, alpha, beta);
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
            auto eval = alphaBetaPruning(board, depth - 1, true, alpha, beta);
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

  auto newMM = alphaBetaPruning(board, 4, board.sideToMove() == chess::Color::WHITE, -2147483647, 2147483647);

  std::string turnString = (board.sideToMove() == chess::Color::WHITE) ? " Whites Turn " : " Blacks Turn ";
  std::cout << "===========" << turnString << "===========\n";
  std::cout << "Best Move is: " << chess::uci::moveToUci(newMM.second) << std::endl;
  std::cout << "Best Score is: " << newMM.first << std::endl;
  std::cout << "Made: " << moveCount << " ghost moves\n";

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