#include <iostream>
#include <variant>
#include <string>

// Define states with call operator
struct IdleState {
    void operator()() const { std::cout << "Handling Idle State\n"; }
};

struct RunningState {
    void operator()() const { std::cout << "Handling Running State\n"; }
};

struct ErrorState {
    std::string errorMessage;
    void operator()() const { std::cout << "Handling Error State: " << errorMessage << '\n'; }
};

// Define the state machine
using State = std::variant<IdleState, RunningState, ErrorState>;

void processState(const State& state) {
    std::visit([](const auto& s) { s(); }, state);
}

int main() {
    State currentState = IdleState{};
    processState(currentState); // Outputs: Handling Idle State

    currentState = RunningState{};
    processState(currentState); // Outputs: Handling Running State

    currentState = ErrorState{"Something went wrong"};
    processState(currentState); // Outputs: Handling Error State: Something went wrong

    return 0;
}
