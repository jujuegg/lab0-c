#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "mcts.h"
#include "util.h"

struct node {
    int move;
    char player;
    int n_visits;
    //  double score;
    uint64_t score;
    struct node *parent;
    struct node *children[N_GRIDS];
};

static struct node *new_node(int move, char player, struct node *parent)
{
    struct node *node = malloc(sizeof(struct node));
    node->move = move;
    node->player = player;
    node->n_visits = 0;
    node->score = 0;
    node->parent = parent;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

static void free_node(struct node *node)
{
    for (int i = 0; i < N_GRIDS; i++)
        if (node->children[i])
            free_node(node->children[i]);
    free(node);
}

static uint64_t fixed_power_int(uint64_t x,
                                unsigned int frac_bits,
                                unsigned int n)
{
    uint64_t result = 1ULL << frac_bits;

    if (n) {
        for (;;) {
            if (n & 1) {
                result *= x;
                result += 1ULL << (frac_bits - 1);
                result >>= frac_bits;
            }
            n >>= 1;
            if (!n)
                break;
            x *= x;
            x += 1ULL << (frac_bits - 1);
            x >>= frac_bits;
        }
    }
    return result;
}

uint64_t fixed_log(uint64_t x)
{
    uint64_t fixed_x = x << FIXED_SCALING_BITS;

    if (x == 0)
        return UINT64_MAX;

    if (x == 1)
        return 0ULL;

    uint64_t result = 0;

    for (int i = 1; i <= 16; ++i) {
        if (i % 2 == 0) {
            result -= fixed_power_int(fixed_x, FIXED_SCALING_BITS, i) / i;
        } else {
            result += fixed_power_int(fixed_x, FIXED_SCALING_BITS, i) / i;
        }
        fixed_x = (fixed_x * (x << FIXED_SCALING_BITS)) >> FIXED_SCALING_BITS;
    }

    return result;
}

uint64_t fixed_sqrt(uint64_t x)
{
    if (x <= 1)
        return x;

    uint64_t low = 0ULL;
    uint64_t high = x;
    uint64_t precision = 1ULL << (FIXED_SCALING_BITS - 2);

    while (high - low > precision) {
        uint64_t mid = (low + high) / 2;
        uint64_t square = (mid * mid) >> FIXED_SCALING_BITS;

        if (square < x) {
            low = mid;
        } else {
            high = mid;
        }
    }

    return (low + high) / 2;
}

static inline uint64_t uct_score(int n_total, int n_visits, uint64_t score)
{
    if (n_visits == 0)
        return UINT64_MAX;

    uint64_t result = score << FIXED_SCALING_BITS /
                                   (uint64_t) (n_visits << FIXED_SCALING_BITS);
    uint64_t tmp =
        EXPLORATION_FACTOR *
        fixed_sqrt(fixed_log(n_total << FIXED_SCALING_BITS) / n_visits);
    tmp >>= FIXED_SCALING_BITS;

    return result + tmp;
}

static struct node *select_move(struct node *node)
{
    struct node *best_node = NULL;
    uint64_t best_score = 0ULL;
    for (int i = 0; i < N_GRIDS; i++) {
        if (!node->children[i])
            continue;
        uint64_t score = uct_score(node->n_visits, node->children[i]->n_visits,
                                   node->children[i]->score);
        if (score > best_score) {
            best_score = score;
            best_node = node->children[i];
        }
    }
    return best_node;
}

static uint64_t simulate(char *table, char player)
{
    char current_player = player;
    char temp_table[N_GRIDS];
    memcpy(temp_table, table, N_GRIDS);
    while (1) {
        char win;
        int *moves = available_moves(temp_table);
        if (moves[0] == -1) {
            free(moves);
            break;
        }
        int n_moves = 0;
        while (n_moves < N_GRIDS && moves[n_moves] != -1)
            ++n_moves;
        int move = moves[rand() % n_moves];
        free(moves);
        temp_table[move] = current_player;
        if ((win = check_win(temp_table)) != ' ')
            return calculate_win_value(win, player);
        current_player ^= 'O' ^ 'X';
    }
    return (uint64_t) (1ULL << (FIXED_SCALING_BITS - 1));
}

static void backpropagate(struct node *node, uint64_t score)
{
    while (node) {
        node->n_visits++;
        node->score += score;
        node = node->parent;
        score = 1 - score;
    }
}

static void expand(struct node *node, char *table)
{
    int *moves = available_moves(table);
    int n_moves = 0;
    while (n_moves < N_GRIDS && moves[n_moves] != -1)
        ++n_moves;
    for (int i = 0; i < n_moves; i++) {
        node->children[i] = new_node(moves[i], node->player ^ 'O' ^ 'X', node);
    }
    free(moves);
}

int mcts(char *table, char player)
{
    char win;
    struct node *root = new_node(-1, player, NULL);
    for (int i = 0; i < ITERATIONS; i++) {
        struct node *node = root;
        char temp_table[N_GRIDS];
        memcpy(temp_table, table, N_GRIDS);
        while (1) {
            if ((win = check_win(temp_table)) != ' ') {
                uint64_t score =
                    calculate_win_value(win, node->player ^ 'O' ^ 'X');
                backpropagate(node, score);
                break;
            }
            if (node->n_visits == 0) {
                uint64_t score = simulate(temp_table, node->player);
                backpropagate(node, score);
                break;
            }
            if (node->children[0] == NULL)
                expand(node, temp_table);
            node = select_move(node);
            assert(node);
            temp_table[node->move] = node->player ^ 'O' ^ 'X';
        }
    }
    struct node *best_node = NULL;
    int most_visits = -1;
    for (int i = 0; i < N_GRIDS; i++) {
        if (root->children[i] && root->children[i]->n_visits > most_visits) {
            most_visits = root->children[i]->n_visits;
            best_node = root->children[i];
        }
    }
    int best_move = best_node->move;
    free_node(root);
    return best_move;
}
