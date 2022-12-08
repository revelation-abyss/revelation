#include "character.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

Character::Character(const ImmutableCharacter& im, char uid):
    uid(uid),
    im(im), HP(im.maxHP), pos(0, 0),
    turnMoved(-1), turnAttacked(-1),
    defShieldHP(0)
{
}

ImmutableCharacter::ImmutableCharacter(const std::string_view& name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
                     uint8_t rng, unsigned netWorth, bool usesArcAttack, const char* flavor)
 : name(name), usesArcAttack(usesArcAttack) {
    this->maxHP = maxHP;
    this->softAtk = softAtk;
    this->hardAtk = hardAtk;
    this->mov = mov;
    this->rng = rng;
    this->netWorth = netWorth;
    this->flavor = flavor;
    this->maxAtk = std::max(softAtk, hardAtk);
}

ImmutableCharacter::~ImmutableCharacter(){
    for(auto effect_ptr : specialAction)
        delete effect_ptr;
}

short Character::takeDmg(bool isHard, short power) {
    (void) isHard; //currently no difference between hard and soft damage
    if(this->defShieldHP > 0) {
        short shielded = std::min(this->defShieldHP, power);
        this->defShieldHP -= shielded;
        power -= shielded;
    }
    this->HP -= power;
    return power;
}

short Character::getAtk(bool isHard, short turnID) const {
    if(this->turnAttacked == turnID - 1) {
        return 10;
    } else if(isHard) {
        return im.hardAtk;
    } else {
        return im.softAtk;
    }
}

void Character::buff() {
    this->defShieldHP = im.maxHP;
}
