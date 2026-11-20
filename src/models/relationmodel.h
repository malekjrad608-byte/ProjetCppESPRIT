#pragma once
#include "modelrows.h"
class RelationModel {
public:
    enum class Entity { Employees, Services, Locals };
    static ModelRows afficher(Entity entity);
};
