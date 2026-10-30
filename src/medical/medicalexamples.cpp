#include "medicalrecord.h"
QVector<MedicalRecord> MedicalRecord::examples()
{
    QVector<MedicalRecord> records;
    auto add = [&records](const QString &animalId, const QString &name, const QString &species,
                      const QString &breed, int age, const QString &sex, const QString &owner,
                      const QString &recordId, const QDate &date, const QString &reason,
                      const QString &diagnosis, const QString &treatment,
                      const QString &vaccinations, const QString &allergies, double weight,
                      const QString &observations, const QString &vaccStatus) {
        AnimalInfo a;
        a.animalId = animalId; a.name = name; a.species = species; a.breed = breed;
        a.age = age; a.sex = sex; a.owner = owner;
        MedicalInfo m;
        m.recordId = recordId; m.consultationDate = date; m.reason = reason;
        m.diagnosis = diagnosis; m.treatment = treatment; m.vaccinations = vaccinations;
        m.allergies = allergies; m.weight = weight; m.observations = observations;
        m.vaccinationStatus = vaccStatus;
        records.append(MedicalRecord(a, m));
    };

    add("ANI-001", "Max", "Chien", "Golden Retriever", 5, "Mâle", "Jean Dupont",
        "MED-001", QDate(2026, 9, 12),
        "Visite de routine", "Animal en bonne santé",
        "Aucun traitement nécessaire",
        "Rage, Parvovirose, Distemper", "Aucune connue", 32.5,
        "Excellent état général. Poids stable.",
        "À jour");
    add("ANI-002", "Luna", "Chat", "Siamois", 3, "Femelle", "Marie Martin",
        "MED-002", QDate(2026, 9, 10),
        "Perte d'appétit", "Gastrite légère",
        "Médicament anti-acide, régime alimentaire doux",
        "Rage, Typhus, Coryza", "Poisson", 4.2,
        "Amélioration observée après 48h de traitement.",
        "À jour");
    add("ANI-003", "Rocky", "Chien", "Bulldog", 7, "Mâle", "Pierre Bernard",
        "MED-003", QDate(2026, 9, 8),
        "Boiterie patte arrière", "Arthrose débutante",
        "Anti-inflammatoires, suppléments articulaires",
        "Rage, Parvovirose", "Aucune connue", 28.0,
        "Surveillance recommandée. Exercice modéré.",
        "À renouveler");
    add("ANI-004", "Mimi", "Chat", "Européen", 2, "Femelle", "Sophie Petit",
        "MED-004", QDate(2026, 9, 5),
        "Vaccination annuelle", "Examen préventif",
        "Vaccination Rage, Typhus, Coryza",
        "Rage, Typhus, Coryza", "Aucune connue", 3.8,
        "Vaccination effectuée sans réaction.",
        "À jour");
    add("ANI-005", "Bella", "Lapin", "Lop", 4, "Femelle", "Lucas Dubois",
        "MED-005", QDate(2026, 9, 3),
        "Problèmes dentaires", "Malocclusion dentaire",
        "Lime dentaire, régime alimentaire riche en fibres",
        "Myxomatose", "Aucune connue", 2.1,
        "Suivi dentaire recommandé tous les 3 mois.",
        "À renouveler");
    add("ANI-006", "Simba", "Chat", "Maine Coon", 6, "Mâle", "Emma Laurent",
        "MED-006", QDate(2026, 8, 28),
        "Toux persistante", "Asthme félin",
        "Corticoïdes inhalés, environnement sans poussière",
        "Rage, Typhus, Coryza", "Acariens", 7.5,
        "Condition stable sous traitement.",
        "À jour");
    add("ANI-007", "Toby", "Chien", "Labrador", 8, "Mâle", "Thomas Moreau",
        "MED-007", QDate(2026, 8, 25),
        "Examen de routine", "Surpoids modéré",
        "Régime hypocalorique, exercice quotidien",
        "Rage, Parvovirose, Distemper", "Aucune connue", 42.0,
        "Perte de poids recommandée (objectif: 35kg).",
        "À jour");
    add("ANI-008", "Coco", "Oiseau", "Perroquet", 12, "Femelle", "Chloe Roux",
        "MED-008", QDate(2026, 8, 20),
        "Plumes anormales", "Carence en vitamines",
        "Suppléments vitaminiques, alimentation variée",
        "Aucune requise", "Aucune connue", 0.45,
        "Plumes en amélioration après 2 semaines de traitement.",
        "À jour");
    return records;
}
