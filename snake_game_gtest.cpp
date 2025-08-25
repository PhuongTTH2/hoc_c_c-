#include <gtest/gtest.h>
#include "snake_game.cpp" // Or include header if available

// Helper: Move snake in a direction
void moveSnake(SnakeGame& game, char dir, int steps = 1) {
    for (int i = 0; i < steps; ++i) {
        game.Input(dir);
        game.Logic();
        if (game.isGameOver()) break;
    }
}

TEST(SnakeGameTest, InitializesCorrectly) {
    SnakeGame game(10, 10);
    EXPECT_FALSE(game.isGameOver());
    // Optionally check initial position, score, etc.
}

TEST(SnakeGameTest, MovesLeft) {
    SnakeGame game(10, 10);
    int oldX = game.getX();
    moveSnake(game, 'a');
    EXPECT_LT(game.getX(), oldX);
}

TEST(SnakeGameTest, EatsFruit) {
    SnakeGame game(10, 10);
    // Place fruit next to snake
    game.setFruit(game.getX() + 1, game.getY());
    moveSnake(game, 'd');
    EXPECT_EQ(game.getScore(), 10);
    EXPECT_EQ(game.getTailLength(), 1);
}

TEST(SnakeGameTest, HitsWall) {
    SnakeGame game(5, 5);
    moveSnake(game, 'a', 10); // Move left until out of bounds
    EXPECT_TRUE(game.isGameOver());
}

TEST(SnakeGameTest, CollidesWithSelf) {
    SnakeGame game(10, 10);
    // Make snake long enough to collide with itself
    game.setFruit(game.getX() + 1, game.getY());
    moveSnake(game, 'd'); // Eat fruit
    game.setFruit(game.getX(), game.getY() + 1);
    moveSnake(game, 's'); // Eat fruit
    moveSnake(game, 'a'); // Move left
    moveSnake(game, 'w'); // Move up into itself
    EXPECT_TRUE(game.isGameOver());
}

TEST(SnakeGameTest, ExitGame) {
    SnakeGame game(10, 10);
    game.Input('x');
    EXPECT_TRUE(game.isGameOver());
}