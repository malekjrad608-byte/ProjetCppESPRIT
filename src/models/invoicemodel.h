#pragma once
#include "service.h"
#include <functional>
class InvoiceModel {
public:
    using Renderer=std::function<bool(const Service &,QString *)>;
    static bool loadStored(const Service &source,Service *invoice,QString *error);
    static bool emitDocument(const Service &source,bool demo,const Renderer &render,QString *error);
};
