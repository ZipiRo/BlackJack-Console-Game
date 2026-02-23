# BlackJack (C++)

A simple Blackjack engine written in C++.

This project implements the full logic of a Blackjack game with support
for betting, splitting, doubling, and a clean state-based architecture
that can be used for:

-   Console UI
-   Custom UI
-   AI simulations
-   Reinforcement learning experiments

------------------------------------------------------------------------

## Game Overview

Blackjack is a card game where the goal is to reach **21** (or as close
as possible) without going over.

### Players

-   **Dealer**
-   **Player**

------------------------------------------------------------------------

## Game Flow

1.  The player places a bet.
2.  The dealer deals:
    -   2 cards to the player (face up)
    -   2 cards to himself (one face up, one face down)
3.  The player chooses one of the available actions.
4.  After the player finishes, the dealer plays.
5.  The round is resolved and payouts are applied.

------------------------------------------------------------------------

## Player Actions

Depending on the situation, the player may choose:

-   **HIT**\
    Draw one additional card.

-   **STAND**\
    Keep current hand and end turn.

-   **DOUBLE**\
    Double the bet amount and receive exactly one more card, then stand.

-   **SPLIT**\
    If the two initial cards have the same value:

    -   Split them into two separate hands.
    -   Each hand keeps the same bet.
    -   The dealer adds one card to each hand.
    -   You may split only once.

------------------------------------------------------------------------

## Dealer Rules

After the player stands:

-   Dealer reveals the hidden card.
-   Dealer must **HIT** until hand value is **≥ 16**.
-   Dealer then **STANDS**.

------------------------------------------------------------------------

## Possible Outcomes

### WIN

-   Player hand ≤ 21
-   Dealer busts OR dealer hand is lower than player

**Payout:** `bet amount`

------------------------------------------------------------------------

### BLACKJACK

-   Player receives **Ace + 10-value card** as the initial two cards

**Payout:** `bet amount * 1.5`

------------------------------------------------------------------------

### LOST

-   Dealer wins with a higher valid hand

------------------------------------------------------------------------

### BUST

-   Player hand value \> 21

------------------------------------------------------------------------

### PUSH

-   Player and dealer have the same hand value

**Payout:** Bet is returned

------------------------------------------------------------------------

## Architecture

The game is implemented as a state-driven engine.

Game states include: - Betting - Player turn - Dealer turn - Results

The engine enforces rules internally and can be used without a UI.

------------------------------------------------------------------------

## Optional Game Info API

To access structured game information (for UI or AI usage), enable:

``` cpp
#define USE_GET_INFO
```

This allows you to retrieve: - Current game state - Player information -
Dealer information - Valid actions - Round results

You can use this to: - Build a custom UI - Run simulations - Train an AI
agent - Log statistics
