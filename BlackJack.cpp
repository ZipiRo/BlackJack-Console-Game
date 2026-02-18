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

using namespace std;

enum WinState {
    Lost,
    Bust,
    Win,
    Push,
    BlackJack
};

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
            this_thread::sleep_for(chrono::milliseconds(300));
            continue; 
        
        }
        if(card.name != "Ace") cout << card.power << ' ' ;

        cout << card.name << '-' ;        
        cout << card.type << ' '; 
        
        this_thread::sleep_for(chrono::milliseconds(300));
    }
}

struct Player
{
    vector<Card> hand[2];
    int using_hand = 0;

    int balance = 0;
};

struct Game {
    std::vector<Card> deck;
    Player dealer;
    Player player;

    int winning_pot = 0;
    int win_result = -1;
    int player_turn = false;
    int dealer_turn = false;
    bool validOption[4] = { true, true, true, false };
    int player_bet = 0;

    void GiveOneCard(Player &to, int faceUp = 1)
    {
        Card draw_card = deck.back();
        deck.pop_back();

        draw_card.faceUp = faceUp;
        to.hand[to.using_hand].push_back(draw_card);
    }

    int HandPower(Player to) 
    {
        int hand_power = 0;
        for(auto card : to.hand[to.using_hand])
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
    
    int IsBlackJack(Player to)
    {
        if(to.hand[to.using_hand].size() < 2) return false;

        bool ace = to.hand[0][0].name == "Ace" || to.hand[0][1].name == "Ace";
        bool ten = to.hand[0][0].power == 10 || to.hand[0][1].power == 10;

        return ace && ten;
    }

    void PlaceBet()
    {
        cout << "Balance: " << player.balance << "$\n";
        this_thread::sleep_for(chrono::milliseconds(300));
        cout << "=========================================\n";
        this_thread::sleep_for(chrono::milliseconds(300));
        cout << "You bet: ";
        
        if(player.balance <= 0)
            throw 100;

        while (true)
        {
            cin >> player_bet;
            if(player_bet <= player.balance && player_bet > 0)
                break;
        }

        player.balance -= player_bet;
        winning_pot = player_bet * 2;
        player_turn = true;
    }

    void Hit(Player &to)
    {
        GiveOneCard(to);
        validOption[DOUBLE] = false;
        validOption[SPLIT] = false;
    }

    void Double(Player &to)
    {
        player.balance -= player_bet;
        winning_pot += player_bet * 2;
        player_bet *= 2;

        GiveOneCard(to);
        player_turn = false;
        dealer_turn = true;
    }

    void Split(Player &to)
    {
        // Needs Work
        to.hand[1].push_back(to.hand[0].back());
        to.hand[0].pop_back();

        GiveOneCard(player);
        validOption[SPLIT] = false;
    }

    void GameView() 
    {
        cout << "Balance: " << player.balance << "$\n";
        this_thread::sleep_for(chrono::milliseconds(100));
        cout << "=========================================\n";
        this_thread::sleep_for(chrono::milliseconds(100));
        cout << "Winning pot: " << winning_pot << "$\n";
        this_thread::sleep_for(chrono::milliseconds(100));
        cout << "=========================================\n";
        this_thread::sleep_for(chrono::milliseconds(100));
        cout << "Dealer (";
        cout << HandPower(dealer); 
        cout << "): "; ShowCards(dealer.hand[dealer.using_hand]); cout << '\n';
        this_thread::sleep_for(chrono::milliseconds(100));
        cout << "Player (";
        cout << HandPower(player); 
        cout << "): "; ShowCards(player.hand[player.using_hand]); cout << '\n';
        cout << "=========================================\n";
    }

    void WinnigView() {
        system("cls");
        cout << "=========================================\n";
        this_thread::sleep_for(chrono::milliseconds(300));
        if(win_result == WinState::Win)
        {
            player.balance += winning_pot;
            cout << "You Won: " << player_bet << "$\n";
            this_thread::sleep_for(chrono::milliseconds(300));
            cout << "Balance: " << player.balance << "$\n";
        }   
        else if(win_result == WinState::Lost)
        {
            cout << "You Lost: " << player_bet << "$\n";
            this_thread::sleep_for(chrono::milliseconds(300));
            cout << "Balance: " << player.balance << "$\n";
        } 
        else if(win_result == WinState::Bust)
        {
            cout << "You Lost (Bust): " << player_bet << "$\n";
            this_thread::sleep_for(chrono::milliseconds(300));
            cout << "Balance: " << player.balance << "$\n";
        }
        else if(win_result == WinState::Push)
        {
            player.balance += player_bet;
            cout << "Push\n";
            this_thread::sleep_for(chrono::milliseconds(300));
            cout << "Balance: " << player.balance << "$\n";
        }
        else if(win_result == WinState::BlackJack)
        {
            player.balance += winning_pot * 1.5;

            cout << "You Won (BlackJack): " << player_bet * 1.5 << "\n";
            this_thread::sleep_for(chrono::milliseconds(300));
            cout << "Balance: " << player.balance << "$\n";
        }
        cout << "=========================================\n";

        dealer.hand[0].clear();
        player.hand[0].clear();
        player.hand[1].clear();
    }

    void PlayerActionView()
    {
        int option = 0; 

        cout << "Action: ";
        this_thread::sleep_for(chrono::milliseconds(200));
        cout << "1-Hit ";
        this_thread::sleep_for(chrono::milliseconds(200));
        cout << "2-Stand ";
        this_thread::sleep_for(chrono::milliseconds(200));
        if (validOption[DOUBLE]) cout << "3-Double ";
        this_thread::sleep_for(chrono::milliseconds(200));
        if (validOption[SPLIT]) cout << "4-Split ";
        cout << "\n=========================================\n";
        this_thread::sleep_for(chrono::milliseconds(300));

        while (true)
        {
            cout << "Option: ";
            cin >> option;
            
            if(option > 0 && option <= 4)
                if(validOption[option - 1])
                    break;
        }

        if(option == HIT+1) Hit(player);
        else if(option == STAND+1) 
        {
            player_turn = false; 
            dealer_turn = true;
            dealer.hand[0][1].faceUp = 1;
        } 
        else if(option == DOUBLE+1) Double(player);
        else if(option == SPLIT+1) Split(player);
    }

    int DealerAction()
    {
        int dealerHand = HandPower(dealer);

        if(dealerHand <= 16) return HIT;
        else return STAND;
    }

    void GameLoop()
    { 
        PlaceBet();
        
        GiveOneCard(player);
        GiveOneCard(dealer);
        GiveOneCard(player);
        GiveOneCard(dealer, 0);

        if(IsBlackJack(dealer))
        {
            dealer.hand[0][1].faceUp = true;
            if(IsBlackJack(player))
                    win_result = WinState::Push;
            else win_result = WinState::Lost;

            system("cls");
            GameView();

            throw 200;
        }

        if(HandPower(player) == 21)
        {
            if(IsBlackJack(dealer))
            {
                dealer.hand[0][1].faceUp = true;
                if(HandPower(dealer) == 21) win_result = WinState::Push;
                    else win_result = WinState::Win;
                
                system("cls");
                GameView();

                throw 200;
            }
            else win_result = WinState::Win;

            if(IsBlackJack(player))
                    win_result = WinState::BlackJack;
                    
            system("cls");
            GameView();

            throw 200;        
        }

        if(player.balance < player_bet) validOption[DOUBLE] = false; 
        if(player.hand[0][0].power == player.hand[0][1].power)
            validOption[SPLIT] = true;

        while (player_turn)
        {
            system("cls");
            GameView();
            PlayerActionView();
            
            if(HandPower(player) >= 21)
            {
                player_turn = false;  
                dealer_turn = true; 
            }
        }

        if(HandPower(player) > 21)
        {
            win_result = WinState::Bust;

            system("cls");
            GameView();

            throw 200;      
        }

        if(HandPower(player) == 21)
        {
            if(IsBlackJack(dealer))
            {
                dealer.hand[0][1].faceUp = true;
                if(HandPower(dealer) == 21) win_result = WinState::Push;
                    else win_result = WinState::Win;
                
                system("cls");
                GameView();

                throw 200;
            }
            else win_result = WinState::Win;

            if(IsBlackJack(player))
                    win_result = WinState::BlackJack;
                
            system("cls");
            GameView();

            throw 200;        
        }

        if(IsBlackJack(dealer))
        {
            win_result = WinState::Lost;

            system("cls");
            GameView();

            throw 200;
        }

        while (dealer_turn)
        {
            system("cls");
            GameView();

            cout << "Dealer Action: ";
            this_thread::sleep_for(chrono::milliseconds(1500));

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

            this_thread::sleep_for(chrono::milliseconds(1500));
        }
        
        this_thread::sleep_for(chrono::milliseconds(1500));
        int dealerHand = HandPower(dealer);

        if(dealerHand > 21)
        {
            win_result = WinState::Win;
            throw 200;
        }

        if(dealerHand == 21)
        {
            win_result = WinState::Lost;
            throw 200;
        }

        if(dealerHand < 21)
        {
            int playerHand = HandPower(player);
            
            if(playerHand < dealerHand)
                win_result = WinState::Lost;
            else if(playerHand == dealerHand)
                win_result = WinState::Push;
            else if(playerHand > dealerHand)
                win_result = WinState::Win;

            throw 200;
        }
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
                this_thread::sleep_for(chrono::milliseconds(300));
                cout << "\nDo you want to play again?\n";
                this_thread::sleep_for(chrono::milliseconds(300));
                cout << "No-0\n";
                this_thread::sleep_for(chrono::milliseconds(300));
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
                this_thread::sleep_for(chrono::milliseconds(300));
                cout << "No-0\n";
                this_thread::sleep_for(chrono::milliseconds(300));
                cout << "Option: ";
                cin >> running;
            }

            player = game.player;
        }
    }
    
    return 0;
}