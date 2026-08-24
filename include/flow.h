#pragma once
#include <iostream>
#include "timeline.h"
#include "entities/entity.h"
#include "entities/tnt.h"
#include "entities/pearl.h"
#include "world.h"
#include "entry.h"

extern Timeline* t;
extern Executor* executor;


void doYourBusiness();

inline void initFlow(World* world_){
    t = new Timeline(world_);
    doYourBusiness();
    executor = new Executor(world_,t->getEntries(),t->getLabelMap());
}