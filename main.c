#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

struct Card {
    char rank[3];
    char suit[9];
};

struct Player {
    int max_hand_count;
    struct Card* hand;
    int bet;
    float bankroll;
    int* hand_count;
    
};

struct Dealer {
    int max_hand_count;
    struct Card* hand;
    int* hand_count;
};

struct Card* load_cards(struct Card* deck, int deck_size){
    FILE *fptr = fopen("cards.txt", "r");
    if (!fptr) {
        perror("Failed to open cards.txt");
        return NULL;
    }
    char buffer[20];
    int count = 0;
    struct Card* deck_ptr = deck;
    while(fgets(buffer, sizeof(buffer), fptr) != NULL && count < deck_size) {
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        char *rank = strtok(buffer, " ");
        char *suit = strtok(NULL, " ");

        if (rank && suit) {
            strcpy(deck_ptr->rank, rank);
            strcpy(deck_ptr->suit, suit);
            deck_ptr++;
            count++;
        }
    }
    fclose(fptr);
    return deck;
}

void print_deck(struct Card* deck_ptr, int deck_size){
    for (int i = 0; i < deck_size; i++) {
        printf("%s %s\n", deck_ptr->rank, deck_ptr->suit);
        deck_ptr++;
    }
}

struct Card* shuffle_cards(struct Card* deck_ptr, int deck_size) {
    unsigned int seed = time(0);
    struct Card temp;
    //fisher yates shuffle
    for (int i = 0; i < deck_size; i++) {
        int rd_num1 = rand_r(&seed) % deck_size;
        temp = *(deck_ptr + rd_num1);
        *(deck_ptr + rd_num1) = *(deck_ptr + i);
        *(deck_ptr + i) = temp;
    }
    return deck_ptr;
}

void print_hand(struct Card* hand_ptr, int hand_count){
    for (int i = 0; i < hand_count; i++) {
        printf("%s %s\n", hand_ptr->rank, hand_ptr->suit);
        hand_ptr++;
    }
}

void draw_card(struct Card* deck_ptr, int* top_deck_count_ptr, struct Card* hand, int* hand_count) {
    struct Card draw;
    //draw from top of deck
    draw = *(deck_ptr + *top_deck_count_ptr);
    //place in hand
    *(hand + *hand_count) = draw;
    (*hand_count)++;
    (*top_deck_count_ptr)--;
}

void setup_blackjack(struct Card* deck_ptr, struct Player* player, struct Dealer* dealer, int* top_deck_count_ptr) {
    int bet;
    *player->hand_count = 0;
    *dealer->hand_count = 0;
    printf("%s", "Enter how much youd like to bet this round.\n");
    scanf("%d", &bet);
    player->bet = bet;

    //player draws 2 cards
    draw_card(deck_ptr, top_deck_count_ptr, player->hand, player->hand_count);
    draw_card(deck_ptr, top_deck_count_ptr, player->hand, player->hand_count);
    
    printf("%s", "Your hand shows:\n");
    print_hand(player->hand, *player->hand_count);

    //dealer draws 2 cards
    draw_card(deck_ptr, top_deck_count_ptr, dealer->hand, dealer->hand_count);
    draw_card(deck_ptr, top_deck_count_ptr, dealer->hand, dealer->hand_count);

    printf("%s", "The dealer hand has 1 card face up and 1 card face down:\n");
    printf("%s %s\n", dealer->hand->rank, dealer->hand->suit);
    printf("%s %s\n", "hidden", "hidden");
}

int rank_to_int(char rank_str[3]) {
    // If numeric string, use atoi
    if (isdigit(rank_str[0])) {
        return atoi(rank_str);
    }
    if (rank_str[0] == 'A') return 1;
    if (rank_str[0] == 'J') return 10;
    if (rank_str[0] == 'Q') return 10;
    if (rank_str[0] == 'K') return 10;

    return -1; // invalid input

}

int calc_hand_sum(struct Card* hand, int hand_count) {
    int hand_sum = 0;
    struct Card* hand_ptr;
    hand_ptr = hand;
    char rank_str[3];
    int rank_int = 0;
    int ace_count = 0;
    for (int i = 0; i < hand_count; i++){
        strcpy(rank_str, hand_ptr->rank);
        rank_int = rank_to_int(rank_str);
        if (rank_int == -1){
            printf("%s", "invalid rank_str value\n");
            return -1;
        } else if (rank_int == 1) {
            ace_count++;
        }
        hand_sum += rank_int;
        hand_ptr++;
    }
    // Upgrade Aces from 1 to 11 if it doesn't bust
    while (ace_count > 0 && hand_sum + 10 <= 21) {
        hand_sum += 10;
        ace_count--;
    }
    return hand_sum;
}

bool play_blackjack(struct Card* deck_ptr, struct Player* player, struct Dealer* dealer, int* top_deck_count_ptr) {
    int player_hand_sum;
    int dealer_hand_sum;
    int play_again;
    int hit;
    
    player_hand_sum = calc_hand_sum(player->hand, *player->hand_count);
    while (true) {
        if (player_hand_sum == 21) { // play got 21 automatically wins
            float profit = player->bet * 1.5;
            player->bankroll += profit;
            printf("You made $%.1f\n", profit);
            break;
        } else if (player_hand_sum < 21) { // player decides whether to hit ro stay
            printf("%s", "Enter any number for hit or 0 for for stay \n");
            scanf("%d", &hit);
            if(hit){ // hit logic
                //player draws a card
                draw_card(deck_ptr, top_deck_count_ptr, player->hand, player->hand_count);
                printf("%s", "Heres your new hand\n");
                print_hand(player->hand, *player->hand_count);
                player_hand_sum = calc_hand_sum(player->hand, *player->hand_count);

            } else { //stay logic
                //dealer reveals hidden card
                printf("%s", "The dealers disclosed hand is:\n");
                print_hand(dealer->hand, *dealer->hand_count);
                dealer_hand_sum = calc_hand_sum(dealer->hand, *dealer->hand_count);
                
                //if dealer sum is 16 or under they have to draw another card
                if (dealer_hand_sum <= 16){
                    draw_card(deck_ptr, top_deck_count_ptr, dealer->hand, dealer->hand_count);
                    printf("%s", "The dealer got < 16 so they drew a new card. Heres the dealers new hand\n");
                    print_hand(dealer->hand, *dealer->hand_count);
                    dealer_hand_sum = calc_hand_sum(dealer->hand, *dealer->hand_count);
                }
                //dealer bust logic
                if(dealer_hand_sum > 21) {
                    printf("%s", "The dealer busted over 21. You won the amount you bet.\n");
                    player->bankroll += player->bet;
                } else if (player_hand_sum == dealer_hand_sum) {
                    printf("%s", "You tied the dealer. Your bankroll remains the same.\n");
                } else if (player_hand_sum > dealer_hand_sum) {
                    printf("%s", "Your hand is stronger than the dealer. You won the amount you bet.\n");
                    player->bankroll += player->bet;
                }
                else {
                    printf("%s", "Your hand is weaker than the dealer. You lost your bet.\n");
                    player->bankroll -= player->bet;
                }
                break;
                
            }
        }
        else { // player bust logic
            printf("%s", "Sorry you busted and lost your bet\n");
            player->bankroll -= player->bet;
            break;
        }
    }
    printf("%s%f\n", "Your bankroll is now $", player->bankroll);
    printf("%s", "Would you like to play another round? If Yes enter any number or if No enter 0\n");
    scanf("%d", &play_again);
    if (play_again) {
        return true;
    } else {
        return false;
    }
}

int main(int argc, char *argv[]) {
    struct Player* player = malloc(sizeof(struct Player));
    struct Dealer* dealer = malloc(sizeof(struct Dealer));
    player->hand = malloc(player->max_hand_count * sizeof(struct Card)); //needs to be freed still
    dealer->hand = malloc(dealer->max_hand_count * sizeof(struct Card)); // need to be freed still
    player->max_hand_count = 11;
    dealer->max_hand_count = 10;
    int player_hand_count = 0;
    int dealer_hand_count = 0;
    player->hand_count = &player_hand_count;
    dealer->hand_count = &dealer_hand_count;
    bool play_game = true;
    int bankroll;
    const int DECK_SIZE = 52;
    int top_deck_count = DECK_SIZE - 1;
    int* top_deck_count_ptr = &top_deck_count;
    struct Card* deck = malloc(DECK_SIZE * sizeof(struct Card));
    deck = load_cards(deck, DECK_SIZE);
    if (deck != NULL) {
        struct Card* deck_ptr = deck;
        deck_ptr = shuffle_cards(deck_ptr, DECK_SIZE);
        //print_deck(deck_ptr, DECK_SIZE);
        printf("%s", "Enter how much money you are playing with.\n");
        scanf("%d", &bankroll);
        player->bankroll = bankroll;
        while (play_game == true){
            setup_blackjack(deck_ptr, player, dealer, top_deck_count_ptr);
            play_game = play_blackjack(deck_ptr, player, dealer, top_deck_count_ptr);
        }
    }
    free(deck);
    free(player->hand);
    free(dealer->hand);
    return 0;
}
