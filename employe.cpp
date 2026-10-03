#include "employe.h"

Employe::Employe() {}

double Employe::tauxCharge() const
{
    if (m_dispoHebdo <= 0) return 0.0;
    return m_chargeHebdo / m_dispoHebdo;
}

bool Employe::depasseSeuil(double seuil) const
{
    return tauxCharge() > seuil;
}