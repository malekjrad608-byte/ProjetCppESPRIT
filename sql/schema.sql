-- ============================================================================
-- Table MEDICAL : STRUCTURE RÉELLE de la base (déjà créée, NE PAS la recréer).
-- L'application s'adapte à cette structure ; aucune modification n'est nécessaire.
--
--   ID_DOSSIER         NUMBER        (clé primaire)  affiché "MED-012" dans l'application
--   DATE_CONSULTATION  DATE
--   MOTIF              VARCHAR2(200)
--   DIAGNOSTIC         CLOB
--   REF_ANIMAL         NUMBER                        affiché "ANI-005" dans l'application
--   NOM_ANIMAL, ESPECE, RACE  VARCHAR2
--   AGE, POIDS         NUMBER (entiers)
--   PROPRIETAIRE       VARCHAR2
--   ID_EMPLOYES        NUMBER NOT NULL               (table EMPLOYES)
--
-- OPTIONNEL (non obligatoire) : pour conserver le sexe et le traitement après
-- redémarrage de l'application (sinon ils restent en mémoire seulement) :
-- ALTER TABLE MEDICAL ADD (SEXE VARCHAR2(10), TRAITEMENT VARCHAR2(500));
-- ============================================================================


-- ---------------------------------------------------------------------------
-- DM-I1 : Rappels vaccinaux
-- Une ligne = un vaccin fait à un animal. Pour chaque (animal, vaccin) la ligne
-- la plus récente donne la prochaine échéance (DATE_RAPPEL).
-- ---------------------------------------------------------------------------
CREATE TABLE VACCINATION (
    ID_VACCIN         VARCHAR2(20) PRIMARY KEY,
    ID_ANIMAL         VARCHAR2(20) NOT NULL,
    NOM_ANIMAL        VARCHAR2(100),
    ESPECE            VARCHAR2(50),
    PROPRIETAIRE      VARCHAR2(100),
    TEL_PROPRIETAIRE  VARCHAR2(30),      -- numéro pour les SMS (sera repris du module Clients plus tard)
    NOM_VACCIN        VARCHAR2(100) NOT NULL,
    DATE_VACCIN       DATE NOT NULL,
    DATE_RAPPEL       DATE NOT NULL,
    DATE_SMS          DATE               -- dernier SMS réellement envoyé
);
CREATE INDEX IDX_VACCINATION_ANIMAL ON VACCINATION (ID_ANIMAL);


-- ---------------------------------------------------------------------------
-- Rendez-vous + SMS automatiques (rappel la veille, "manqué" le lendemain)
-- STATUT : Prévu / Venu / Manqué / Annulé
-- ---------------------------------------------------------------------------
CREATE TABLE RENDEZVOUS (
    ID_RDV            VARCHAR2(20) PRIMARY KEY,
    ID_ANIMAL         VARCHAR2(20) NOT NULL,
    NOM_ANIMAL        VARCHAR2(100),
    PROPRIETAIRE      VARCHAR2(100),
    TEL_PROPRIETAIRE  VARCHAR2(30),
    MOTIF             VARCHAR2(200),
    DATE_RDV          DATE NOT NULL,          -- date + heure
    STATUT            VARCHAR2(20) DEFAULT 'Prévu',
    DATE_SMS_RAPPEL   DATE,                   -- SMS de rappel déjà envoyé (une seule fois)
    DATE_SMS_MANQUE   DATE                    -- SMS "rendez-vous manqué" déjà envoyé (une seule fois)
);
CREATE INDEX IDX_RDV_ANIMAL ON RENDEZVOUS (ID_ANIMAL);
