#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
#include <algorithm>
#include <limits>

using namespace std;
using namespace Desdemona;

class AdvancedBot : public OthelloPlayer
{
public:
    AdvancedBot(Turn turn);
    virtual Move play(const OthelloBoard &board);

private:
    static const int MAX_DEPTH = 6; // Adjust based on time constraints
    static const int INFINITY_VALUE = numeric_limits<int>::max();

    // Evaluation weights
    static const int CORNER_WEIGHT = 100;
    static const int EDGE_WEIGHT = 10;
    static const int MOBILITY_WEIGHT = 15;
    static const int STABILITY_WEIGHT = 25;

    int negaMax(OthelloBoard &board, int depth, int alpha, int beta, Turn player);
    int evaluatePosition(const OthelloBoard &board, Turn player) const;
    bool isCorner(int x, int y) const;
    bool isEdge(int x, int y) const;
    int getStabilityScore(const OthelloBoard &board, Turn player) const;
    int getMobilityScore(const OthelloBoard &board, Turn player) const;
};

AdvancedBot::AdvancedBot(Turn turn) : OthelloPlayer(turn)
{
}

bool AdvancedBot::isCorner(int x, int y) const
{
    return (x == 0 || x == 7) && (y == 0 || y == 7);
}

bool AdvancedBot::isEdge(int x, int y) const
{
    return x == 0 || x == 7 || y == 0 || y == 7;
}

int AdvancedBot::getMobilityScore(const OthelloBoard &board, Turn player) const
{
    list<Move> moves = board.getValidMoves(player);
    return moves.size();
}

int AdvancedBot::getStabilityScore(const OthelloBoard &board, Turn player) const
{
    int score = 0;
    // Count stable pieces (corners and edges that can't be flipped)
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board.get(i, j) == player)
            {
                if (isCorner(i, j))
                {
                    score += CORNER_WEIGHT;
                }
                else if (isEdge(i, j))
                {
                    score += EDGE_WEIGHT;
                }
            }
        }
    }
    return score;
}

int AdvancedBot::evaluatePosition(const OthelloBoard &board, Turn player) const
{
    Turn opponent = (player == BLACK) ? RED : BLACK;

    // Get piece counts
    int myPieces = (player == BLACK) ? board.getBlackCount() : board.getRedCount();
    int oppPieces = (player == BLACK) ? board.getRedCount() : board.getBlackCount();

    // Calculate various strategic scores
    int stabilityScore = getStabilityScore(board, player) -
                         getStabilityScore(board, opponent);

    int mobilityScore = getMobilityScore(board, player) -
                        getMobilityScore(board, opponent);

    // Combine scores with weights
    int totalScore = (myPieces - oppPieces); // Base piece difference
    totalScore += STABILITY_WEIGHT * stabilityScore;
    totalScore += MOBILITY_WEIGHT * mobilityScore;

    return totalScore;
}

int AdvancedBot::negaMax(OthelloBoard &board, int depth, int alpha, int beta, Turn player)
{
    // Base case: leaf node or game over
    if (depth == 0 || board.getValidMoves(player).empty())
    {
        return evaluatePosition(board, player);
    }

    list<Move> moves = board.getValidMoves(player);
    int maxScore = -INFINITY_VALUE;

    // Try each move
    for (list<Move>::iterator it = moves.begin(); it != moves.end(); ++it)
    {
        OthelloBoard newBoard = board; // Create board copy
        newBoard.makeMove(player, *it);

        // Recursive call with negation and player switch
        Turn nextPlayer = (player == BLACK) ? RED : BLACK;
        int score = -negaMax(newBoard, depth - 1, -beta, -alpha, nextPlayer);

        maxScore = max(maxScore, score);
        alpha = max(alpha, score);

        // Alpha-beta pruning
        if (alpha >= beta)
        {
            break;
        }
    }

    return maxScore;
}

Move AdvancedBot::play(const OthelloBoard &board)
{
    list<Move> moves = board.getValidMoves(turn);

    if (moves.size() == 1)
    {
        return *moves.begin();
    }

    Move bestMove = *moves.begin();
    int bestScore = -INFINITY_VALUE;

    // Try each move with negamax
    for (list<Move>::iterator it = moves.begin(); it != moves.end(); ++it)
    {
        OthelloBoard newBoard = board;
        newBoard.makeMove(turn, *it);

        Turn opponent = (turn == BLACK) ? RED : BLACK;
        int score = -negaMax(newBoard, MAX_DEPTH - 1, -INFINITY_VALUE, INFINITY_VALUE, opponent);

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = *it;
        }
    }

    return bestMove;
}

// Required bot module creation functions
extern "C"
{
    OthelloPlayer *createBot(Turn turn)
    {
        return new AdvancedBot(turn);
    }

    void destroyBot(OthelloPlayer *bot)
    {
        delete bot;
    }
}