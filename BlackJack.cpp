#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <chrono>
#include <thread>

#define HIT 0
#define STAND 1
#define DOUBLE 2
#define SPLIT 3

#define NO_MONEY 100
#define GAME_DONE 200

#define BLACK_JACK 1
#define WIN 2
#define LOST 3
#define PUSH 4

#define BUST 5
#define LESS 6

using namespace std;

string result_strings[10] = {"", "BLACK-JACK", "WIN", "LOST", "PUSH", "BUST"};

string card_type[4] = {"Hearts", "Tiles", "Clovers", "Pikes"};
string card_name[13] = {"Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King"};

struct Card {
    string type;
    string name ;
    int power;
    int faceUp = false;
};

vector<Card> original_deck;

void InitOriginalDeck()
{    
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 13; j++)
        {
            int power = j + 1;
            if(power > 10) power = 10;

            Card card = {card_type[i], card_name[j], power};
            original_deck.push_back(card);
        }
    }
}

vector<Card> SlowShuffleDeck(std::vector<Card> deck) 
{
    vector<Card> result_deck;
    bool usedCards[52] = {};

    while (result_deck.size() < deck.size())
    {
        int number;
        bool found = false;
        while (!found)
        {
            found = true;
            number = rand() % deck.size();
            if(usedCards[number]) found = false;
                else usedCards[number] = true;
        }

        result_deck.push_back(deck[number]);
    }

    return result_deck;
}

vector<Card> ShuffleDeck(std::vector<Card> deck) 
{
    for (int i = deck.size() - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        std::swap(deck[i], deck[j]);
    }

    return deck;
}

void ShowCards(vector<Card> cards)
{
    for(auto card : cards)
    {
        if(card.faceUp == 0) 
        {
            cout << "FaceDown "; 
            continue; 
        
        }
        if(card.name != "Ace") cout << card.power << ' ' ;

        cout << card.name << '-' ;        
        cout << card.type << ' '; 
    }
}

struct Hand
{
    vector<Card> cards;
    int result = -1;
    int dealer_result = -1;
    int bet_amount = 0; 
    bool open = false; 
    bool validOption[4] = { true, true, true, true };
    bool split = false;

    vector<Card> &operator() (){
        return cards;
    }
};

const int MAX_HANDS = 50;

struct Player
{
    vector<Hand> hand = vector<Hand>(MAX_HANDS);
    int using_hand = 0;
    int open_hands = 1;

    int balance = 0;
};

struct Game {
    std::vector<Card> deck;
    Player dealer;
    Player player;

    int dealer_turn = false;
    bool splited = false;

    void GiveOneCard(Player &to, int faceUp = 1, int hand = -1)
    {
        if(hand == -1) hand = to.using_hand;
        Card draw_card = deck.back();
        deck.pop_back();

        draw_card.faceUp = faceUp;
        to.hand[hand]().push_back(draw_card);
    }

    int HandPower(Player &to, int hand = -1) 
    {
        if(hand == -1) hand = to.using_hand;
        int hand_power = 0;
        for(auto card : to.hand[hand]())
        {
            if(!card.faceUp) continue;
            
            if(card.name == "Ace")
            {
                if(hand_power + 11 > 21) hand_power += card.power;
                    else hand_power += 11;
            }
            else hand_power += card.power;
        }

        return hand_power;
    }

    bool AnyHandOpen(Player &of)
    {
        for(auto hand : of.hand)
            if(hand.open)
                return true;
        
        return false;
    }
    
    int IsBlackJack(Player &to, int hand = -1)
    {
        if(hand == -1) hand = to.using_hand;
        if(to.hand[to.using_hand]().size() < 2) return false;

        bool ace = to.hand[hand]()[0].name == "Ace" || to.hand[hand]()[1].name == "Ace";
        bool ten = to.hand[hand]()[0].power == 10 || to.hand[hand]()[1].power == 10;

        return ace && ten;
    }

    void Hit(Player &to)
    {
        to.hand[to.using_hand].validOption[DOUBLE] = false;
        to.hand[to.using_hand].validOption[SPLIT] = false;

        GiveOneCard(to);
    }

    void Double(Player &to)
    {
        to.balance -= to.hand[to.using_hand].bet_amount;
        to.hand[to.using_hand].bet_amount *= 2;

        GiveOneCard(to);
    }

    void Split(Player &to)
    {
        to.hand[to.using_hand].validOption[SPLIT] = false;
        int hand_bet_amount = to.hand[to.using_hand].bet_amount;
        
        to.open_hands++;
        int splited_hand = to.open_hands - 1; 
        to.balance -= hand_bet_amount; 
        to.hand[splited_hand].bet_amount = hand_bet_amount;
        to.hand[splited_hand]().push_back(to.hand[to.using_hand]()[1]);
        to.hand[splited_hand].open = true;
        to.hand[splited_hand].split = true;


        to.hand[to.using_hand]().pop_back();

        GiveOneCard(to, 1, to.using_hand);
        GiveOneCard(to, 1, splited_hand);

        splited = true;
    }

    void GameView() 
    {
        cout << "Balance: " << player.balance << "$\n";
        cout << "=========================================\n";
        cout << "Hand #" << player.using_hand  << " | Winning pot: " << player.hand[player.using_hand].bet_amount * 2 << "$\n";
        cout << "=========================================\n";
        cout << "Dealer (";
        cout << HandPower(dealer); 
        cout << "): "; ShowCards(dealer.hand[dealer.using_hand]()); cout << '\n';
        cout << "Player (";
        cout << HandPower(player); 
        cout << "): "; ShowCards(player.hand[player.using_hand]()); cout << '\n';
        cout << "=========================================\n";
    }

    void ResolveWinningHands(Player &of)
    {
        int win_result = -1;
        int dealer_result = dealer.hand[0].result;
        int dealer_hand_power = HandPower(dealer);
        
        for(int i = 0; i < of.open_hands; i++)
        {
            int player_result = of.hand[i].result;
            int player_hand_power = HandPower(of, i);
            
            if(player_result == dealer_result)
            {
                if(player_result == LESS)
                {
                    if(player_hand_power < dealer_hand_power)
                        win_result = LOST;
                    else if(player_hand_power > dealer_hand_power)
                        win_result = WIN;
                    else win_result = PUSH;
                }
                else if(player_result == BUST) win_result = BUST;
                else win_result = PUSH;
            }
            else if(dealer_result == BLACK_JACK) win_result = LOST;
            else if(player_result == BLACK_JACK) win_result = BLACK_JACK;
            else if(player_result == WIN) win_result = WIN;
            else if(player_result == BUST) win_result = BUST;
            else if(player_result == LESS)
            {
                if(dealer_result == BUST) win_result = WIN;
                else if (dealer_result == WIN) win_result = LOST;
            }

            of.hand[i].result = win_result;
            
            if(dealer_result == BLACK_JACK) of.hand[i].dealer_result = BLACK_JACK; 
            else if(win_result == LOST || win_result == BUST) of.hand[i].dealer_result = WIN;
            else if(win_result == WIN || win_result == BLACK_JACK) of.hand[i].dealer_result = LOST;
            else of.hand[i].dealer_result = PUSH;
        }
    }

    void WinnigView() {
        system("cls");
        
        ResolveWinningHands(player);

        int total_winings = 0;
        
        cout << "=========================================\n";
        for(int i = 0; i < player.open_hands; i++)
        {   
            int result = player.hand[i].result;
            int dealer_result = player.hand[i].dealer_result;
            int bet_amount = player.hand[i].bet_amount;
            int player_hand_power = HandPower(player, i);

            cout << "HAND #" << i << " | BET AMOUNT: $" << bet_amount << '\n';

            cout << "DEALER: HAND-POWER: " << HandPower(dealer) << " (" << result_strings[dealer_result] << ")" << '\n';
            cout << "PLAYER: HAND-POWER: " << player_hand_power << " (" << result_strings[result] << ")" << '\n';
            
            if(result == BLACK_JACK)
            {
                cout << "---------------BLACK-JACK----------------\n";
                cout << "HAND WON: +$" << (bet_amount * 2) * 1.5 << '\n';
                total_winings += (bet_amount * 2) * 1.5;
            }
            else if(result == WIN)
            {
                cout << "HAND WON: +$" << bet_amount * 2 << '\n';
                total_winings += bet_amount * 2; 
            }
            else if(result == LOST)
                cout << "HAND LOST: -$" << bet_amount << '\n';
            else if(result == BUST)
                cout << "HAND BUST: -$" << bet_amount << '\n';
            else if(result == PUSH) 
            {
                cout << "HAND PUSH: +$" << bet_amount << '\n';
                total_winings += bet_amount;            
            }
                
            cout << "=========================================\n";

            player.hand[i] = Hand();
        }
        cout << "TOTAL WINNINGS: $" << total_winings << '\n'; 
        player.balance += total_winings;
        cout << "BALANCE: $" << player.balance << '\n';
    
        cout << "=========================================\n";
    }

    int PlayerOptionView()
    {       
        int option = 0; 

        cout << "Option: ";
        cout << "1-Hit ";
        cout << "2-Stand ";
        if (player.hand[player.using_hand].validOption[DOUBLE]) cout << "3-Double ";
        if (player.hand[player.using_hand].validOption[SPLIT]) cout << "4-Split ";
        cout << "\n=========================================\n";

        while (true)
        {
            cout << "Option: ";
            cin >> option;
            
            if(option > 0 && option <= 4)
                if(player.hand[player.using_hand].validOption[option - 1])
                    break;
        }

        return option;
    }

    int DealerAction()
    {
        int dealerHand = HandPower(dealer);

        if(dealerHand <= 16) return HIT;
        else return STAND;
    }

    void PlaceBet()
    {
        cout << "Balance: " << player.balance << "$\n";
        cout << "=========================================\n";
        cout << "You bet: ";
        
        if(player.balance <= 0)
            throw NO_MONEY;

        int player_bet = 0;
        while (true)
        {
            cin >> player_bet;
            if(player_bet <= player.balance && player_bet > 0)
                break;
        }

        player.balance -= player_bet;
        player.hand[player.using_hand].bet_amount = player_bet;
        player.hand[player.using_hand].open = true;
    }

    void GameLoop()
    { 
        GiveOneCard(player);
        GiveOneCard(dealer);
        GiveOneCard(player);
        GiveOneCard(dealer, 0);

        if(IsBlackJack(dealer))
        {
            dealer.hand[0]()[1].faceUp = true;
            dealer.hand[0].result = BLACK_JACK;

            // HERE SHOULD BE SOME KIND OF INSURANCE I THINK
        }

        for(int i = 0; i < player.open_hands; i++)
        {
            if(IsBlackJack(player, i))
            {
                player.hand[i].result = BLACK_JACK;
                player.hand[i].open = false;
            }
            else if(HandPower(player, i) == 21)
            {
                player.hand[i].result = WIN;  
                player.hand[i].open = false;
            }
        }
        
        if(player.open_hands < 2 && 
            dealer.hand[0].result == BLACK_JACK)
        {
            system("cls");
            GameView();

            throw GAME_DONE;
        }

        if(!AnyHandOpen(player))
        {
            system("cls");
            GameView();

            throw GAME_DONE;
        }

        bool hand_closed;
        bool using_splitted_hand = false;
        int last_hand_used;
        while(player.using_hand < player.open_hands)
        {
            if(!player.hand[player.using_hand].open) 
            {
                player.using_hand++;
                continue;
            }

            if(player.hand[player.using_hand]()[0].power == player.hand[player.using_hand]()[1].power)
                        player.hand[player.using_hand].validOption[SPLIT] = true;
            else player.hand[player.using_hand].validOption[SPLIT] = false;

            if(player.balance < player.hand[player.using_hand].bet_amount)
            {
                player.hand[player.using_hand].validOption[DOUBLE] = false; 
                player.hand[player.using_hand].validOption[SPLIT] = false; 
            }

            if(player.hand[player.using_hand].split)
                player.hand[player.using_hand].validOption[SPLIT] = false; 

            hand_closed = false;

            while (!hand_closed)
            {
                system("cls");
                GameView();

                int option = PlayerOptionView();
                
                if(option == HIT+1) Hit(player);
                else if(option == DOUBLE+1) Double(player);
                else if(option == SPLIT+1) Split(player);

                if(HandPower(player) >= 21 || option == STAND+1 || option == DOUBLE+1)
                    hand_closed = true; 
            }

            int hand_power = HandPower(player);

            if(hand_power > 21) player.hand[player.using_hand].result = BUST;
            else if(hand_power == 21) player.hand[player.using_hand].result = WIN;
            else if(hand_power < 21) player.hand[player.using_hand].result = LESS;
            
            player.hand[player.using_hand].open = false;
            
            if(using_splitted_hand)
            {
                player.using_hand = last_hand_used;
                using_splitted_hand = false;
            }

            if(splited) 
            {
                last_hand_used = player.using_hand;
                player.using_hand = player.open_hands - 1; 
                splited = false;
                using_splitted_hand = true;
            }
            else player.using_hand++;
        }

        dealer_turn = true;
        dealer.hand[0]()[1].faceUp = true;
        if(HandPower(dealer) == 21)
        {
            dealer.hand[0].result = WIN;
            
            system("cls");
            GameView();

            throw GAME_DONE;
        }

        while (dealer_turn)
        {
            system("cls");
            GameView();

            cout << "Dealer Action: ";

            int dealer_option = DealerAction();
            if(dealer_option == HIT) 
            {
                cout << "Hit\n";
                Hit(dealer);
            }
            else if(dealer_option == STAND) 
            {
                cout << "Stand\n";
                dealer_turn = false;
            }   

        }
        
        int dealerHand = HandPower(dealer);
        if(dealerHand > 21) dealer.hand[0].result = BUST;
        if(dealerHand == 21) dealer.hand[0].result = WIN;
        if(dealerHand < 21) dealer.hand[0].result = LESS;

        throw GAME_DONE;
    }
};

int main() {
    InitOriginalDeck(); 

    srand(time(0));

    int running = 1;
    Player player;
    player.balance = 5;

    while (running)
    {
        system("cls");
        Game game{original_deck};
        game.deck = SlowShuffleDeck(game.deck);
        game.player = player;

        try
        {
            game.GameLoop();
        }
        catch(const int code) {
            if(code == NO_MONEY)
            {
                system("cls");
                cout << "=========================================\n";
                cout << "You don't have any more money.\n";
                cout << "\nDo you want to play again?\n";
                cout << "No-0\n";
                cout << "Option: ";
                cin >> running; 
                cout << '\n';

                if(running)
                {
                    cout << "Add more money: "; 
                    cin >> game.player.balance;
                }
            }
            if(code == GAME_DONE)
            {
                game.WinnigView();

                cout << "Do you want to place another bet?\n";
                cout << "No-0\n";
                cout << "Option: ";
                cin >> running;
            }

            player = game.player;
            player.using_hand = 0;
            player.open_hands = 1;
        }
    }
    
    return 0;
}