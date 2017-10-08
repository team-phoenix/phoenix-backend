#pragma once

#include "corevariable.h"
#include "libretro.h"

class Test_VariableModel {
    VariableModel *model;

private slots:

    void init() {
        model = new VariableModel;
    }

    void cleanup() {
        delete model;
    }

    void should_throwExceptionFor() {

        retro_variable variable;
        variable.key = "";
        variable.value = nullptr;

        model->insert( variable );

    }

};
