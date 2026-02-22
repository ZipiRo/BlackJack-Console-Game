#define USE_UI_GET_INFO

#include "BlackJack.h"

std::vector<Card> original_deck;

BlackJackGame game;

bool game_runnning = true;

int main()
{
    srand(time(NULL));

    original_deck = InitOriginalDeck();

    Player player;
    player.balance = 100;

    game.Init(player, ShuffleDeck(original_deck));
    InitGameDisplay(&game);

    game.Start();

    while (game_runnning)
    {
        game.Run();

        if(game.game_done)
        {
            game.ResetGame();
            game.SetDeck(ShuffleDeck(original_deck));
        }

        DisplayGame();
    }

    return 0;
}