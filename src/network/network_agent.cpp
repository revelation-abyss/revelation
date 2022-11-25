#include "network_agent.hpp"
#include "spectator.hpp"
#include "server.hpp"
#include "semaphore.hpp"
#include <algorithm>

NetworkAgent::NetworkAgent(uint myId, std::shared_ptr<Session> session)
: StepByStepAgent(myId), sender(std::move(session))
{
}

std::vector<std::unique_ptr<NetworkAgent>> NetworkAgent::makeAgents(unsigned short nb, ServerRoom& room, unsigned int startingId){
    using namespace std::chrono_literals;

    std::cout << "(main) make agents\n";
    std::vector<std::shared_ptr<Session>> sessions(nb);
    for(int i = 0; i<nb; i++){
        auto session = room.addSession(i+startingId+1);
        sessions[i] = session;
    }
    std::cout << "(main) wait for agents...\n";
    std::vector<std::unique_ptr<NetworkAgent>> retVal;
    for(const auto& session : sessions) {
        session->awaitReconnect();
    }
    for(int i = 0; i<nb; i++) {
        retVal.emplace_back(
            std::make_unique<NetworkAgent>(i+startingId, sessions[i])
        );
    }
    std::cout << "(main) found agents!\n";
    return retVal;
}

// Adapted from https://en.cppreference.com/w/cpp/string/basic_string/replace
void replace_all(std::string& inout, char what, std::string_view with){
    std::string::size_type pos = 0;
    while(true){
        pos = inout.find(what, pos);
        if(pos == inout.npos) break;
        inout.replace(pos, 1, with);
        pos += with.length();
    }
}

uint NetworkAgent::choose(const OptionList& options, const std::string_view& message) {
    std::string optionList = std::string("[");
    for(const auto& [ keys, optionText ] : options){
        optionList += '"';
        optionList += optionText;
        optionList += "\",";
    }
    optionList += '"';
    optionList += message;
    optionList += "\"]";
    auto message_shared = std::make_shared<const std::string>(std::move(optionList));
show_options:
    sender->send_sync(message_shared);

#define SHARE(str) std::make_shared<std::string>(str)

    input_loop:
        std::string str;
        try {
            str = sender->get_sync();
        } catch(DisconnectedException&){
            throw AgentSurrenderedException(myId);
        }
        if(str == "?") goto show_options;
        if(str == StepByStepAgent::SURRENDER) throw AgentSurrenderedException(myId);
        auto [ value, success ] = StepByStepAgent::inputValidation( options, str );
        if(not success){
            replace_all(str, '\\', "\\\\");
            replace_all(str, '"', "\\\""); //Escape the string to make it valid json
            sender->send_sync(SHARE(std::string("\"!Wrong value: `") + str + "`\""));
            goto input_loop;
        }
    sender->send_sync(SHARE("\"Ok\""));
    return value;
}
