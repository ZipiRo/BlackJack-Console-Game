#define USE_GET_INFO

#include "BlackJack.h"

/*
    BLACKJACK USER INTERFACE 

    THIS IS SOME SIMPLE UI FOR THE BLACKJACK ENGINE I CREATED SO THAT YOU CAN ACTUALY PLAY THE GAME 
    YOU CAN MAKE THE UI BETTER IF YOU WANT.

    TO PLAY YOU PLACE A BET AT THE START FROM A BALANCE OF $100 
    THEN THE GAME STARTS YOU CAN GIVE AVAILABLE ACTIONS HIT, STAND, ETC.
    THEN AFTER YOUR TURN THE DEALER STARTS, FINISHES 
    THEN THE RESULTS ARE SHOWN FOR ALL THE HANDS YOU PLAYED THAT ROUND
    AFTER ALL THIS A LOOP STARTS 
*/

/*
    I WANTED TO ADD THIS CHEET SHEET TO SEE HOW EFECTIVE IT IS
    AI HELPED ME WITH THE PARSING OF THE TABLE (I AM TO LAZY FOR THAT)
    AND HELPED ME ON THE FUNCTION  Action GetBestAction(player_hand, dealer_card);
*/

#include <iostream>
#include <thread>
#include <chrono>

BlackJackGame blackjack_game;

std::string action_strings[5] = {"HIT", "STAND", "DOUBLE", "SPLIT"};
std::string result_strings[6] = {"WIN", "LOST", "BUST", "PUSH", "BLACKJACK"};
 
enum Action
{
    H, S, P,
    DH, DS, PH, RH
}; 

std::string recomanded_action_string[8] = 
{
    "HIT",
    "STAND",
    "SPLIT",
    "DOUBLE/HIT",
    "DOUBLE/STAND",
    "SPLIT/HIT",
    "SURRENDER/HIT"
};

const int HARD_START = 0;
const int HARD_END = 9;
const int SOFT_START = 10;
const int SOFT_END = 16;
const int PAIR_START = 17;
const int PAIR_END = 26;

Action cheet_table[27][10] = {
    /* 8  */ {H,  H,  H,  H,  H,  H,  H,  H,  H,  H}, // HARD
    /* 9  */ {H,  DH, DH, DH, DH, H,  H,  H,  H,  H},
    /* 10 */ {DH, DH, DH, DH, DH, DH, DH, DH, H,  H},
    /* 11 */ {DH, DH, DH, DH, DH, DH, DH, DH, DH, DH},
    /* 12 */ {H,  H,  S,  S,  S,  H,  H,  H,  H,  H},
    /* 13 */ {S,  S,  S,  S,  S,  H,  H,  H,  H,  H},
    /* 14 */ {S,  S,  S,  S,  S,  H,  H,  H,  H,  H},
    /* 15 */ {S,  S,  S,  S,  S,  H,  H,  H,  RH, H},
    /* 16 */ {S,  S,  S,  S,  S,  H,  H,  RH, RH, RH},
    /* 17 */ {S,  S,  S,  S,  S,  S,  S,  S,  S,  S}, // END

    /* A,2*/ {H,  H,  H,  DH, DH, H,  H,  H,  H,  H}, // SOFT
    /* A,3*/ {H,  H,  H,  DH, DH, H,  H,  H,  H,  H},
    /* A,4*/ {H,  H,  DH, DH, DH, H,  H,  H,  H,  H},
    /* A,5*/ {H,  H,  DH, DH, DH, H,  H,  H,  H,  H},
    /* A,6*/ {H,  DH, DH, DH, DH, H,  H,  H,  H,  H},
    /* A,7*/ {S,  DS, DS, DS, DS, S,  S,  H,  H,  H},
    /* A,8*/ {S,  S,  S,  S,  S,  S,  S,  S,  S,  S}, // END

    /* 2,2*/ {PH, PH, P,  P,  P,  P,  H,  H,  H,  H}, // PAIR
    /* 3,3*/ {PH, PH, P,  P,  P,  P,  H,  H,  H,  H},
    /* 4,4*/ {H,  H,  H,  PH, PH, H,  H,  H,  H,  H},
    /* 5,5*/ {DH, DH, DH, DH, DH, DH, DH, DH, H,  H},
    /* 6,6*/ {PH, P,  P,  P,  P,  H,  H,  H,  H,  H},
    /* 7,7*/ {P,  P,  P,  P,  P,  P,  H,  H,  H,  H},
    /* 8,8*/ {P,  P,  P,  P,  P,  P,  P,  P,  P,  P},
    /* 9,9*/ {P,  P,  P,  P,  P,  S,  P,  P,  S,  S},
    /*10,10*/{S,  S,  S,  S,  S,  S,  S,  S,  S,  S},
    /* A,A*/ {P,  P,  P,  P,  P,  P,  P,  P,  P,  P} // END
};

Action GetBestAction(const std::vector<Card> &player_hand, const Card &dealer_card)
{
    int hand_power = GetHandPower(player_hand);
    int column = (dealer_card.name == "Ace") ? 9 : (dealer_card.power - 2);

    // CHECKING FOR PAIRS
    if (player_hand.size() == 2 && player_hand[0].power == player_hand[1].power) 
    {
        int pair_value = player_hand[0].power;
        int row;
        
        if (player_hand[0].name == "Ace") row = PAIR_END;
        else row = PAIR_START + (pair_value - 2);          
        
        return cheet_table[row][column];
    }

    // CHECKING FOR SOFT TOTAL
    bool is_soft = false;
    int other_card_value = 0;

    if (player_hand.size() == 2) 
    {
        if (player_hand[0].name == "Ace") { is_soft = true; other_card_value = player_hand[1].power; }
        else if (player_hand[1].name == "Ace") { is_soft = true; other_card_value = player_hand[0].power; }
    }

    if (is_soft) 
    {
        if (other_card_value >= 8) return Action::S;
        int row = SOFT_START + (other_card_value - 2); 
        return cheet_table[row][column];
    }

    // CHECKING THE REST OF THE TOTALS
    int row;
    if (hand_power <= 8) row = HARD_START;
    else if (hand_power >= 17) row = HARD_END;
    else row = HARD_START + hand_power - 8;

    return cheet_table[row][column];
}

void ShowCards(const std::vector<Card> &cards)
{
    for(auto card : cards)
    {
        if(card.faceUp == 0) 
        {
            std::cout << "FaceDown "; 
            continue; 
        
        }
        if(card.name != "Ace") std::cout << card.power << ' ' ;

        std::cout << card.name << '-' ;        
        std::cout << card.type << ' '; 
    }
}

void BetUI(const GameInfo &game_info)
{
    std::system("cls");

    std::cout << "Balance: $" << game_info.balance << '\n';

    float bet_amount = 0;
    while (bet_amount <= 0)
    {
        std::cout << "Bet: ";
        std::cin >> bet_amount;

        if(bet_amount > game_info.balance)
            bet_amount = 0; 
    }
    
    blackjack_game.PlayerBet(bet_amount);
}

void PlayerTurnUI(const GameInfo &game_info)
{
    std::system("cls");

    int player_action = -1;

    std::cout << "Hand #" << game_info.current_hand_index << '\n'; 
    std::cout << "Player Cards: ";
    ShowCards(game_info.current_hand.cards);
    std::cout << " | (" << GetHandPower(game_info.current_hand.cards) << ")" << '\n';
    std::cout << "Dealer Cards: ";
    ShowCards(game_info.dealer_cards);
    std::cout << " | (" << GetHandPower(game_info.dealer_cards) << ")" << '\n';
    std::cout << "=====================================================" << '\n';
    std::cout << "BestAction: " << recomanded_action_string[GetBestAction(game_info.current_hand.cards, game_info.dealer_cards[0])] <<'\n';
    std::cout << "=====================================================" << '\n';
    std::cout << "Actions: ";
    
    for(int i = 0; i < PLAYER_ACTION_COUNT; i++)
    {
        if(blackjack_game.IsActionValid(i))
            std::cout << i + 1 << "-" << action_strings[i] << " ";
    }

    std::cout << '\n';
    std::cout << "Option: ";
    while (!blackjack_game.ApplyPlayerAction(player_action - 1))
    {
        std::cin >> player_action;
    }
}

void DealerTurnUI(const GameInfo &game_info)
{
    std::system("cls");

    int player_action = -1;

    std::cout << "Dealer Cards: ";
    ShowCards(game_info.dealer_cards);
    std::cout << " | (" << GetHandPower(game_info.dealer_cards) << ")" << '\n';
    std::cout << "=====================================================" << '\n';
    std::cout << "Action: " << action_strings[game_info.dealer_action] << '\n';

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void ResultsUI(const ResultGameInfo &result_info)
{
    std::system("cls");

    std::cout << "Balance: $" << result_info.balance << '\n';
    
    std::cout << "=====================================================" << '\n';

    std::cout << "Dealer Cards: ";
    ShowCards(result_info.dealer_cards);
    std::cout << " | (" << GetHandPower(result_info.dealer_cards) << ")" << '\n';

    std::cout << "=====================================================" << '\n';

    for(int i = 0; i < result_info.game_results.size(); i++)
    {
        std::cout << "Hand #" << i + 1 << " | " << result_info.win_amount << "$" << '\n';
        std::cout << "Player Cards: ";
        ShowCards(result_info.player_hands[i].cards);
        std::cout << " | (" << GetHandPower(result_info.player_hands[i].cards) << ")" << '\n';
        std::cout << result_strings[result_info.game_results[i]] << '\n';
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void DisplayGame()
{
    const GameInfo &game_info = blackjack_game.GetGameInfo();

    switch (game_info.state)
    {
    case BETTING:
        BetUI(game_info);
        break;
    case PLAYER_TURN:
        PlayerTurnUI(game_info);
        break;
    case DEALER_TURN:
        DealerTurnUI(game_info);
        break;
    case WAITING:
        ResultsUI(blackjack_game.GetResultsInfo());
        break;
    default:
        break;
    }
}

std::vector<Card> original_deck;
bool game_runnning = true;

int main()
{
    srand(time(NULL));

    original_deck = InitOriginalDeck();

    Player player;
    player.balance = 100;

    blackjack_game.Init(player, ShuffleDeck(original_deck));
    blackjack_game.Start();

    while (game_runnning)
    {
        if(blackjack_game.game_done)
        {
            blackjack_game.ResetGame();
            blackjack_game.SetDeck(ShuffleDeck(original_deck));
        }

        blackjack_game.Run();

        DisplayGame();
    }

    return 0;
}