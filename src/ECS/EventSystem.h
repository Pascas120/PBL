//
// Created by lukas on 06.05.2025.
//

#ifndef EVENTSYSTEM_H
#define EVENTSYSTEM_H



#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <iostream>

// Bazowa klasa zdarzenia
class Event {
public:
    virtual ~Event() = default;
};


using EventListener = std::function<void(const Event&)>;


class EventSystem {
private:
    std::unordered_map<std::type_index, std::vector<EventListener>> listeners;
    std::queue<std::unique_ptr<Event>> eventQueue;

public:
    EventSystem() = default;
    EventSystem(const EventSystem&) = delete; // Wyłączenie kopiowania
    EventSystem& operator=(const EventSystem&) = delete; // Wyłączenie przypisania
    EventSystem(EventSystem&&) = default; // Przenoszenie dozwolone
    EventSystem& operator=(EventSystem&&) = default;
    template<typename EventType>
    void registerListener(EventListener listener) {
        listeners[std::type_index(typeid(EventType))].push_back(std::move(listener));
    }

    template<typename EventType>
    void triggerEvent(const EventType& event) {
        auto it = listeners.find(std::type_index(typeid(EventType)));
        if (it != listeners.end()) {
            for (auto& listener : it->second) {
                listener(event);
            }
        }
    }

    template<typename EventType, typename... Args>
    void queueEvent(Args&&... args) {
        eventQueue.push(std::make_unique<EventType>(std::forward<Args>(args)...));
    }

    template<typename EventType>
    void queueEvent(EventType event) {
        eventQueue.push(std::make_unique<EventType>(std::move(event)));
    }

    void processEvents() {
        while (!eventQueue.empty()) {
            auto event = std::move(eventQueue.front());
            eventQueue.pop();

            auto it = listeners.find(std::type_index(typeid(*event)));
            if (it != listeners.end()) {
                for (auto& listener : it->second) {
                    listener(*event);
                }
            }
        }
    }
};


#endif //EVENTSYSTEM_H
