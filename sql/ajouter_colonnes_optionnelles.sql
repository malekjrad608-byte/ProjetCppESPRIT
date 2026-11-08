-- ============================================================================
-- OPTIONNEL : colonnes supplémentaires de la table MEDICAL
-- À lancer UNE SEULE FOIS dans SQL Developer, seulement si tu veux que ces informations
-- soient ENREGISTRÉES dans Oracle (sinon elles restent en mémoire jusqu'à la fermeture
-- de l'application). L'application détecte automatiquement les colonnes présentes.
-- Ce script ajoute des colonnes ; il ne modifie ni ne supprime rien d'existant.
-- ============================================================================
ALTER TABLE MEDICAL ADD (
    SEXE               VARCHAR2(10),
    TRAITEMENT         VARCHAR2(500),
    VACCINATIONS       VARCHAR2(200),
    ALLERGIES          VARCHAR2(200),
    OBSERVATIONS       VARCHAR2(500),
    STATUT_VACCINATION VARCHAR2(30),
    TELEPHONE          VARCHAR2(20)
);
-- Vérification :
-- DESC MEDICAL;
-- Fermer puis relancer l'application après l'exécution.
