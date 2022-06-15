#ifndef REVELATION_STATE_H
#define REVELATION_STATE_H

#include "constants.hpp"
#include "deck.hpp"

class Character; class Player;

class State {
    Character*[2][FULL_BOARD_WIDTH] board;
    Character*[2][NB_CHARACTERS] units;
    uint8_t[2] nbAliveUnits;
    Player*[2] players;
    Deck resDeck;
    uint8_t iActive;
    Timestep timestep;

    State(Character*[2][FULL_BOARD_WIDTH] board, Character*[2][NB_CHARACTERS] units, uint8_t[2] nbAliveUnits, Player*[2] players, Deck resDeck, uint8_t iActive, Timestep timestep):
    iActive(0)
    {
        /*nbAliveUnits = [ len(army) for army in units ]*/
    }
};

#endif //REVELATION_STATE_H
