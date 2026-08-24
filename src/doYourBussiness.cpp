#include "flow.h"

void doYourBusiness(){
    t->Label("default")
        .At(ANY_TICK)
            .Do([](World* world){
                new Tnt(world, FIXED, 1, 80,
                0, 0, 0,
                0, 0, 0
                );
            })
            .Quit();
}