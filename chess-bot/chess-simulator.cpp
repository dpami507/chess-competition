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
    100, 100, 100, 100, 100, 100, 100, 100,
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

const int CORNERS[] = {
    100, 90, 80, 70, 70, 80, 90, 100,
    90, 90, 70, 60, 60, 70, 90, 90,
    80, 70, 50, 40, 40, 50, 70, 80,
    70, 60, 40, 0, 0, 40, 60, 70,
    70, 60, 40, 0, 0, 40, 60, 70,
    80, 70, 50, 40, 40, 50, 70, 80,
    90, 90, 70, 60, 60, 70, 90, 90,
    100, 90, 80, 70, 70, 80, 90, 100
};

const int MIDDLE[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 20, 20, 20, 20, 0, 0,
    0, 0, 20, 20, 20, 20, 0, 0,
    0, 0, 20, 20, 20, 20, 0, 0,
    0, 0, 20, 20, 20, 20, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

//Piece Scores
const int KING_SCORE = 20000;
const int QUEEN_SCORE = 900;
const int ROOK_SCORE = 500;
const int BISHOP_SCORE = 330;
const int KNIGHT_SCORE = 320;
const int PAWN_SCORE = 100;

int getBoardScore(Board& board)
{
    int materialScore = 0;
    int score = 0;
    int dangerScore = 0;
    int defendedBonus = 1;

    chess::Color sideToMove = board.sideToMove();

    Color enemyColor;
    if (sideToMove == Color::WHITE)
        enemyColor = Color::BLACK;
    else if (sideToMove == Color::BLACK)
        enemyColor = Color::WHITE;

    bool inCheck = board.inCheck();

    Movelist moves;
    movegen::legalmoves(moves, board, sideToMove);

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
        int index;
        if (color == Color::WHITE) index = static_cast<int>(sq);
        else {
            int rank = sq / 8;
            int file = sq % 8;
            index = (7 - rank) * 8 + file;
        }

        int pieceScore = 0;

        if (type == PieceType::KING && board.us(sideToMove).count() > 8)
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

        pieceScore += MIDDLE[index];
        
        /////////////////////////////////////////////////////////////////////////////////////////

        //Checks if the space has enemy pieces attacking it or ally pieces defending it and takes that into account

        //check if a space is attacked
        Bitboard attackers = attacks::attackers(board, enemyColor, square);
        int numAttackers = attackers.count();

        //check if a space is defended
        Bitboard defenders = attacks::attackers(board, sideToMove, square);
        int numDefenders = defenders.count();

        if (numAttackers >= numDefenders)
            dangerScore += -10;
        else if (numDefenders >= numAttackers)
            dangerScore += 5;

        /////////////////////////////////////////////////////////////////////////////

        //Subtract other opponets pieces while adding score 
        if(sideToMove == Color::WHITE)
            materialScore += (color == Color::WHITE) ? pieceScore : -pieceScore;
        else
            materialScore += (color == Color::WHITE) ? -pieceScore : pieceScore;
    }

    ////////////////////////////////////////////////////////////////////////

    //Mobility

    int numLegalMoves = moves.size();
    int totalMovesWorth = 0;

    //Adds for each legal move to a safe square and adds less for each legal move to an attacked square
    for (auto move : moves)
    {
        //Prefers safe moves in mobility calculations
        int moveWorth = 2;
        if (attacks::attackers(board, enemyColor, move.to()).count() > 0)
            moveWorth -= 1;

        totalMovesWorth += moveWorth;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////

    //Puts more pressure on the opponent's king near the endgame, moving it to a corner or wall

    int numOpponentPieces = 0;
    size_t opponentKingSquare = -1;

    for (size_t sq = 0; sq < 64; sq++)
    {
        //Get the square and piece on that square
        Square square = static_cast<Square>(sq);
        Piece piece = board.at(square);

        //Get piece characteristics
        if (piece == Piece::NONE) continue;

        if (piece.color() == enemyColor)
        {
            numOpponentPieces++;
            if (piece.type() == PieceType::KING)
                opponentKingSquare = sq;
        }
    }

    if (numOpponentPieces <= 4 && opponentKingSquare != 0)
    {
        int opponentKingIndex = (enemyColor == Color::WHITE) ? static_cast<int>(opponentKingSquare)
            : 63 - static_cast<int>(opponentKingSquare);
        score += CORNERS[opponentKingIndex];
    }

    ////////////////////////////////////////////////////////////////////////////////////

    //King Safety, checks how many enemies are attacking the kings defined area
    //Numbers and formula from https://www.chessprogramming.org/King_Safety

    Square kingSquare = board.kingSq(sideToMove);
    Bitboard kingArea = attacks::king(kingSquare);
    int kingDanger = 0;
    int kingAttackerCount = 0;
    int valueOfAttacks = 0;
    const int kingAttackWeight[] = {0,0,50,75,88,94,97,99};

    Bitboard tempKingArea = kingArea;
    while (tempKingArea)
    {
        Square square = Square(tempKingArea.lsb());
        tempKingArea.pop();
        Bitboard attackers = attacks::attackers(board, enemyColor, square);

        kingAttackerCount += attackers.count();

        while (attackers)
        {
            Square attackerSquare = attackers.lsb();
            attackers.pop();
            Piece piece = board.at(attackerSquare);

            PieceType type = piece.type();
            Color color = piece.color();

            if (color == enemyColor)
            {
                if (type == PieceType::QUEEN)
                    valueOfAttacks += 80;
                else if (type == PieceType::ROOK)
                    valueOfAttacks += 40;
                else if (type == PieceType::BISHOP || type == PieceType::KNIGHT)
                    valueOfAttacks += 20;
            }
        }
    }

    if (kingAttackerCount > 7)
        kingAttackerCount = 7;

    kingDanger = (valueOfAttacks * kingAttackWeight[kingAttackerCount]) / 100;

    /////////////////////////////////////////////////////////////////////////////////////

    score += materialScore * 2;
    score += dangerScore;
    score += totalMovesWorth * 3;
    score -= kingDanger * 2;

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
    if(moves.empty()) return { getBoardScore(board), chess::Move::NO_MOVE };

    chess::Move bestMove = moves[0];

    if (getMax)
    {
        int maxEval = -1000000; // the best move placeholder

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
}