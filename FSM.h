/**
* FSM implements a generic finite state machine.
*
*
* Example:
* -------
*
* 
*   //   +---+            +---+            +---+
*   //   | A | -- 'a' --> | B | -- 'b' --> | C |
*   //   +---+            +---+            +---+
*   //
*
*   // FSM states:
*   enum class States : uint8_t { A, B, C };
*
*   // FSM triggers:
*   enum class Triggers : uint8_t { a, b };
*
*   // define an FSM
*                                         // origin state, destination state , trigger     , action
*   FSM<States, States::A, Triggers> fsm{ { States::A    , States::B         , Triggers::a , []() { std::cout << "perform action when transitioning from A to B.\n"; } },
*                                         { States::B    , States::C         , Triggers::b , []() { std::cout << "perform action when transitioning from B to C.\n"; } } };
*
*   // start executing the FSM
*   assert(fsm.IsInitial());
*   bool success = fsm.Execute(Triggers::a);
*   assert(States::B == fsm.GetState());
*   assert(success == true);
*   success = fsm.Execute(Triggers::b);
*   assert(States::C == fsm.GetState());
*   assert(success == true);
*   fsm.SetState(States::A);
*   assert(fsm.IsInitial());
*
* Dan Israel Malta
**/

// Includes
#include<functional>
#include<map>
#include<vector>

// type traits
namespace {
    // test if an object is iterate-able
    template<typename T, typename = void> struct is_iterate_able : std::false_type {};
    template<typename T> struct is_iterate_able<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> : std::true_type {};
};

/**
* \brief A generic finite state machine (FSM) implementation.
*
* @param {State,   in} finite state machine states
* @param {State,   in} finite state machine initial state
* @param {Trigger, in} finite state machine triggers
*/
template<class State, State Initial, class Trigger> class FSM {
        
    // public structures
    public:

        // an object defining a transition (between two states, with triggers, guards and operation)
        struct Trans {
            State m_originState,            // origin state
                  m_destinationState;       // destination state
            Trigger m_trigger;              // trigger
            std::function<void()> m_action; // an action function
        };

    // properties
    private:
        State m_currentState;                               // current state
        std::map<State, std::vector<Trans>> m_transitions;  // each state holds a vector of transitions            

    // API
    public:

        // default constructor
        explicit constexpr FSM() : m_currentState(Initial), m_transitions() {}

        // construct from collections
        explicit constexpr FSM(const std::vector<Trans>& xi_transitions)                           { AddTransitions(xi_transitions); }
        explicit constexpr FSM(std::initializer_list<Trans> xi_transitions)                        { AddTransitions(xi_transitions); }
        template<std::size_t N> explicit constexpr FSM(const std::array<Trans, N>& xi_transitions) { AddTransitions(xi_transitions); }      

        // add a collection of transitions to the FSM (collection should be iterate-able and include only 'Trans' objects)
        template<typename Collection, typename std::enable_if<is_iterate_able<Collection>::value>::type* = nullptr> 
        void AddTransitions(const Collection& xi_collection) noexcept {
            for (const auto& c : xi_collection) {
                m_transitions[c.m_originState].emplace_back(c);
            }
        }

        // get current state
        State GetState() const noexcept { return m_currentState; }

        // set current state
        void SetState(const State xi_state) noexcept { m_currentState = xi_state; }

        // test if current state is initial state
        bool IsInitial() const noexcept { return (m_currentState == Initial); }

        /**
        * \brief execute a given trigger according to FSM semantics
        * 
        * @param {Trigger,  in}  FSM trigger
        * @param {bool,     out} true if transition to destination state occurred, otherwise - false
        **/
        bool Execute(Trigger xi_trigger) noexcept {
            // are there any transitions from current state?
            const auto state_transitions = m_transitions.find(m_currentState);

            // exit if current state doesn't have a transition
            if(state_transitions == m_transitions.end()) return false;

            // iterate the transitions and perform the first one which adheres the trigger and guard
            for(const auto& transition : state_transitions->second) {
    
                // if trigger doesn't match, continue
                if (xi_trigger != transition.m_trigger) continue;

                // if action exists - execute it                
                if (transition.m_action) transition.m_action();             

                // update state
                m_currentState = transition.m_destinationState;
                               
                // transition finished              
                return true;
            }

            // failed
            return false;
        }
};
