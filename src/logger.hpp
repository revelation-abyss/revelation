#ifndef REVELATION_LOGGER_HPP
#define REVELATION_LOGGER_HPP

#include "step.hpp"
#include "team.hpp"
#include "player.hpp"
#include "nlohmann/json.hpp"
#include <ostream>
#include <vector>
#include <memory>

using json = nlohmann::json;

json makeStartStateJson(const State& startState, const std::array<Team, 2>& teams);

/**
 * SubLoggers provide one type of logging functionality, e.g. writing to the screen or to a file.
 * A full Logger has zero or more SubLoggers.
 */
class SubLogger {
public:
    virtual ~SubLogger() = default;
    virtual void addStep(const json& step) = 0;
};

class Logger {
    std::vector<std::unique_ptr<SubLogger>> subLoggers;
    const std::string startState; //a JSON dump of the start state
public:
    Logger(const State& startState, const std::array<Team, 2>& teams)
        : startState(makeStartStateJson(startState, teams).dump()) {};
    Logger* liveServer(const char* ipAddress, unsigned short port);
    Logger* logToFile(std::ostream& file);
    void addStep(const uptr<Step>& step);
};

#endif //REVELATION_LOGGER_HPP
