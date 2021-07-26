#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
    // FIXME: Implement the .wait() functionallity
    std::unique_lock<std::mutex> lck(_mtx); // unique_lock necessary, because we need to use the wait functionallity here
    _cond.wait(lck, [this](){return !_queue.empty(); }); // lock the thread until the queue is not empty and then pull the next traffic light phase

    // Work with .back() and .pop_back() to access the first inserted element of the queue
    T result = std::move(_queue.back());
    _queue.pop_back();  
    
    return result; 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mtx); // locks until the end of the scope
    // Work with .push_front(T) to insert at the front of the queue
    _queue.emplace_front(msg); 
    _cond.notify_one(); // free the condition variable for one thread
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red; // Default Light phase is red
    _cycleDuration = calculateRandomPhase(); 
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true){ // The infinite while-loop is necessary, since we can receive a TrafficLightPhase::red from the _phaseQueue 
        //std::chrono::milliseconds(10); // To reduce the load on the processor
        TrafficLightPhase current_phase = _phaseQueue.receive(); 
        if (current_phase == TrafficLightPhase::green){
            return; 
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    // init stop watch
    lastUpdate = std::chrono::system_clock::now();    
    while (true){
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        std::cout << "This is timeSinceLastUpdate: " << timeSinceLastUpdate << "\nAnd this is _cycleDuration: " << _cycleDuration << "\n"; // Debug  
        if (timeSinceLastUpdate >= _cycleDuration) // do the update of the light phase if enough time has passed
        {
            // toggle the traffic light
            if (_currentPhase == TrafficLightPhase::red){
                _currentPhase = TrafficLightPhase::green; 
            } else {
                _currentPhase = TrafficLightPhase::red;
            }

            // send an update message to the message queue
            TrafficLightPhase tmp_phase = _currentPhase; // default copy assignment operator
            _phaseQueue.send(std::move(tmp_phase)); // move the current traffic light phase to the MessageQueue<TrafficLightPhase>

            // update _cycleDuration for the next traffic light phase
            _cycleDuration = calculateRandomPhase(); 
        
            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();
        }
        // no else block, since the while(true) loop realizes the else block implicitly
    }
}

double TrafficLight::calculateRandomPhase(){
    /*
    // old code
    // calculates a random double number between 4 and 6 in order to calculate the next duration of the traffic light phase
    double valMin = 4; 
    double valMax = 6; 
    double randNum = (double)rand() / RAND_MAX; // random number between 0 and 1
    double random_num = valMin + randNum * (valMax - valMin); // min value with the offset of a random propotion of the given number interval
    */
    
    /*
     * returns the cycle duration between 4000 and 6000 milliseconds (which is 4 to 6 seconds, but we want to be consistent with milliseconds within the whole code base)
     */

    std::random_device rd; 
    std::mt19937 eng(rd()); 

    std::uniform_int_distribution<> distr(4000, 6000); 

    int cycle_dur = distr(eng); 
    //std::cout << "Cycle duration: " << cycle_dur << "\n"; // DEBUG
    return  cycle_dur; 
}