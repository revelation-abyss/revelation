#include "network_agent.hpp"
#include "spectator.hpp"
#include "server.hpp"
#include "semaphore.hpp"
#include <algorithm>

NetworkAgent::NetworkAgent(uint myId, Spectator* sender)
: StepByStepAgent(myId), sender(sender)
{
}

std::vector<std::unique_ptr<NetworkAgent>> NetworkAgent::makeAgents(unsigned short nb, ServerRoom &room, unsigned int startingId){
    std::cout << "(main) make agents\n";
    std::vector<Spectator*> notConnectedAgents(nb);
    Semaphore semaphore(0);
    for(int i = 0; i<nb; i++){
        room.expectNewAgent(i+1, notConnectedAgents[i], &semaphore);
    }
    std::cout << "(main) wait for agents...\n";
    semaphore.acquire(nb); //waits until all nb threads have released the semaphore once
    std::cout << "(main) found agents!\n";
    std::vector<std::unique_ptr<NetworkAgent>> retVal;
    for(int i = 0; i<nb; i++) {
        auto agent = std::make_unique<NetworkAgent>(startingId + i, notConnectedAgents[i]);
        retVal.push_back(std::move(agent));
    }
    return retVal;
}

uint NetworkAgent::input(uint min, uint max) {
    while(true){
        std::string str;
        try {
            str = sender->get();
        } catch(DisconnectedException&){
            throw AgentSurrenderedException(myId);
        }
        try {
            long int_val = std::stol(str);
            if (int_val >= 0) {
                uint uint_val = static_cast<uint>(int_val);
                if (min <= uint_val and uint_val <= max) return uint_val;
            }
        } catch(std::invalid_argument&){ }
        sender->send(std::make_shared<std::string>("!Wrong value"));
    }
}

void NetworkAgent::addOption(const std::string_view& option, int) {
    optionList += '"';
    optionList += option;
    optionList += "\",";
}

void NetworkAgent::closeOptionList(const std::string_view& message){
    auto message_summary = std::make_shared<std::string>("[");
    optionList.swap(*message_summary);
    *message_summary += '"';
    *message_summary += message;
    *message_summary += "\"]";
    sender->send(message_summary);
}
