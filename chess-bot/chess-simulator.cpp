#include "chess-simulator.h"
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
#include "chess.hpp"
#include <random>
using namespace ChessSimulator;

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

//Get the coords of a move from the position -> ex: "a6"
std::pair<int, int> getCoords(const std::string& position)
{
    char lettersAsIndex[] = {'h', 'g', 'f', 'e', 'd', 'c', 'b', 'a'};

    for (size_t i = 0; i < 8; i++)
    {
        if (lettersAsIndex[i] == position[0])
        {
            int x = position[0] - 'a';
            int y = position[1] - '1';

            return { x, y };
        }
    }

    std::cout << "FAILED TO CREATE PAIR\n";
}

int getScore(chess::Board board, chess::Move move)
{
    std::string moveCoord = move.to();
    std::pair<int, int> coords = getCoords(moveCoord);

    int score = 0;
    int pieceScore = 0;

    chess::Square square((coords.second * 8) + coords.first);
    chess::Piece piece = board.at(square);

    switch (piece.type()) {
    case chess::PieceType(chess::PieceType::KING):
        pieceScore += 20000;
        pieceScore += PSQT_king[coords.second * 8 + coords.first];
        break;
    case chess::PieceType(chess::PieceType::QUEEN):
        pieceScore += 900;
        pieceScore += PSQT_queen[coords.second * 8 + coords.first];
        break;
    case chess::PieceType(chess::PieceType::ROOK):
        pieceScore += 500;
        pieceScore += PSQT_rook[coords.second * 8 + coords.first];
        break;
    case chess::PieceType(chess::PieceType::KNIGHT):
        pieceScore += 320;
        pieceScore += PSQT_knight[coords.second * 8 + coords.first];
        break;
    case chess::PieceType(chess::PieceType::BISHOP):
        pieceScore += 330;
        pieceScore += PSQT_bishop[coords.second * 8 + coords.first];
        break;
    case chess::PieceType(chess::PieceType::PAWN):
        pieceScore += 100;
        pieceScore += PSQT_pawn[coords.second * 8 + coords.first];
        break;
    default:
        break;
    }
    if (piece.color() == chess::Color::BLACK)
        score -= pieceScore;
    else
        score += pieceScore;

    return score;
}

std::pair<int, int> miniMax(int currentDepth, int index, bool getMax, int maxDepth, chess::Board board, chess::Movelist moves) {
    if (currentDepth == maxDepth || moves.empty()) 
    { 
        //Get the score of where we are 
        int score = getScore(board, chess::Move()); 
        
        // Return the pair of score and index 
        return { score, index }; 
    } 
    //For every move 
    for (auto move : moves) 
    { 
        //Make the move 
        chess::Board tempBoard = board;
        tempBoard.makeMove(move); 
        chess::Movelist newMoves; 
        chess::movegen::legalmoves(newMoves, tempBoard); 
        if (getMax) 
            return std::max(miniMax(currentDepth + 1, index * 2, false, maxDepth, tempBoard, newMoves),
                            miniMax(currentDepth + 1, index * 2 + 1, false, maxDepth, tempBoard, newMoves)); 
        else 
            return std::min(miniMax(currentDepth + 1, index * 2, true, maxDepth, tempBoard, newMoves), 
                        miniMax(currentDepth + 1, index * 2 + 1, true, maxDepth, tempBoard, newMoves)); 
    } 
}

std::string ChessSimulator::Move(std::string fen) {
  // create your board based on the board string following the FEN notation
  // search for the best move using minimax / monte carlo tree search /
  // alpha-beta pruning / ... try to use nice heuristics to speed up the search
  // and have better results return the best move in UCI notation you will gain
  // extra points if you create your own board/move representation instead of
  // using the one provided by the library

  // here goes a random movement
  chess::Board board(fen);
  chess::Movelist moves;
  chess::movegen::legalmoves(moves, board);

  if(moves.size() == 0)
    return "";

  std::pair<int, int> mM = miniMax(0, 0, true, 4, board, moves);
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