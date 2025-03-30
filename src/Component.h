//
// Created by lukas on 31.03.2025.
//

#ifndef COMPONENT_H
#define COMPONENT_H



class Component {
public:
    virtual void update() = 0;
    virtual ~Component() = default;
};



#endif //COMPONENT_H
