#ifndef REVELATION_UNITSREPOSITORY_H
#define REVELATION_UNITSREPOSITORY_H

#include "unique_character.hpp"
#include "team.hpp"
#include "visitor.hpp"
#include "constants.hpp"
#include <iosfwd>
#include <unordered_map>
#include <string_view>
#include <array>

using TeamId = std::string;
using CharacterId = std::string; //TODO OPTIMIZE these could be string_views pointing to the character's name
using CharacterRef = const ImmutableCharacter*;

class UnitsRepository {
public:
    using TeamList = std::unordered_map<TeamId, const Team>;
    using CharacterList = std::unordered_map<CharacterId, const ImmutableCharacter>;
private:
    TeamList teams;
    CharacterList characters;
    TeamList::iterator addTeamWithoutName(const std::array<CharacterRef, ARMY_SIZE>& cters, Generator& gen);
public:
    const TeamList& getTeams() const { return teams; }
    const CharacterList& getCharacters() const { return characters; }

    template<typename... Args>
    CharacterRef addCharacter(Args&&... args){
        ImmutableCharacter chr(std::forward<Args>(args)...);
        auto [ iter, success ] = characters.emplace(std::make_pair(chr.name, std::move(chr)));
        if(not success) return nullptr;
        else return &iter->second;
    }
    std::pair<TeamList::iterator, bool> p_createTeam(const std::array<CharacterRef, ARMY_SIZE>& cters, const std::string_view& name);
    std::pair<TeamList::iterator, bool> p_createTeam(const std::array<CharacterId, ARMY_SIZE>& names, const std::string_view& name);
    std::pair<TeamList::iterator, bool> p_createTeam(WriterVisitor& visitor);
    template<typename... Args>
    inline const Team* createTeam(Args&&... args){
        auto [ iter, success ] = p_createTeam(std::forward<Args>(args)...);
        if(not success) return nullptr;
        else return &iter->second;
    }
    inline void deleteTeam(TeamList::iterator iter){
        teams.erase(iter);
    }
    TeamList::iterator mkRandom(Generator& generator, unsigned short int nbUnits = ARMY_SIZE);
    TeamList::iterator mkRandomWithPreexistingCharacters(Generator& generator);
    void mkDefaultTeams();
    void readFile(std::istream& file);
};

#endif //REVELATION_UNITSREPOSITORY_H
