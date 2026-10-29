-- =====================================================================
--  ANIWORLD - BASE DE DONNEES COMMUNE (modele physique Oracle)
--  A executer par CHAQUE membre de l'equipe, avec le meme user (rawand),
--  dans SQL Developer : ouvrir ce fichier puis F5 (Executer le script).
--
--  Modele relationnel (MLD) issu du diagramme E/A :
--    SERVICES        (ID_SERV, CLIENT, CATEGORIE, STATUT, DATE_HEURE, DUREE, TARIF, PRESTATION)
--    EMPLOYES        (ID_EMP, NOM, PRENOM, CIN, ..., #ID_SERV)      effectue : SERVICE (1,n) - EMPLOYE (1,1)
--    LOCAUX          (ID_LOCAL, NOM_LOCAL, TYPE_LOCAL, ..., #ID_SERV) se deroule : SERVICE (0,n) - LOCAL (1,1)
--    STOCK           (ID_ARTICLE, ARTICLE, NOM_ARTICLE, ..., #ID_LOCAL) contient : LOCAL (0,n) - STOCK (1,1)
--    DOSSIER_MEDICAL (ID_DOSS, DATE_CONSULTATION, ..., #ID_EMP)      realise : EMPLOYE (0,n) - MEDICAL (1,1)
--  Regle : cote (1,1) -> cle etrangere NOT NULL dans cette table.
-- =====================================================================

-- 0) Nettoyage : supprime les anciennes versions de ces tables si elles existent
BEGIN
  FOR t IN (SELECT table_name FROM user_tables
            WHERE table_name IN ('DOSSIER_MEDICAL', 'STOCK', 'LOCAUX', 'EMPLOYES', 'SERVICES')) LOOP
    EXECUTE IMMEDIATE 'DROP TABLE "' || t.table_name || '" CASCADE CONSTRAINTS PURGE';
  END LOOP;
END;
/

-- 1) SERVICES
CREATE TABLE SERVICES (
    ID_SERV       NUMBER(10)     CONSTRAINT PK_SERVICES PRIMARY KEY,
    CLIENT        VARCHAR2(100)  NOT NULL,
    CATEGORIE     VARCHAR2(50)   NOT NULL,
    STATUT        VARCHAR2(30)   NOT NULL,
    DATE_HEURE    DATE           NOT NULL,
    DUREE         NUMBER(5)      CHECK (DUREE > 0),                 -- en minutes
    TARIF         NUMBER(10,2)   CHECK (TARIF >= 0),
    PRESTATION    VARCHAR2(200)
);

-- 2) EMPLOYES  (chaque employe effectue 1 service)
CREATE TABLE EMPLOYES (
    ID_EMP          NUMBER(10)     CONSTRAINT PK_EMPLOYES PRIMARY KEY,
    NOM             VARCHAR2(50)   NOT NULL,
    PRENOM          VARCHAR2(50)   NOT NULL,
    CIN             VARCHAR2(8)    NOT NULL UNIQUE,
    DATE_NAISS      DATE,
    ADRESSE         VARCHAR2(150),
    COURRIEL        VARCHAR2(100),
    TEL             VARCHAR2(15),
    POSTE           VARCHAR2(50),
    COMPETENCES     VARCHAR2(250),
    DATE_EMBAUCHE   DATE,
    TYPE_CONTRAT    VARCHAR2(20),
    SALAIRE         NUMBER(10,2)   CHECK (SALAIRE >= 0),
    CHARGE          NUMBER(5),
    DISPONIBLE      NUMBER(1)      DEFAULT 1 CHECK (DISPONIBLE IN (0, 1)),
    DISPO_REPOS     VARCHAR2(50),
    ID_SERV         NUMBER(10)     NOT NULL,
    CONSTRAINT FK_EMPLOYES_SERVICES FOREIGN KEY (ID_SERV) REFERENCES SERVICES (ID_SERV)
);

-- 3) LOCAUX  (chaque local accueille 1 service)
CREATE TABLE LOCAUX (
    ID_LOCAL          NUMBER(10)     CONSTRAINT PK_LOCAUX PRIMARY KEY,
    NOM_LOCAL         VARCHAR2(150)  NOT NULL UNIQUE,
    TYPE_LOCAL        VARCHAR2(100)  NOT NULL,
    ETAGE             NUMBER(3)      DEFAULT 0 NOT NULL,             -- 0 = rez-de-chaussee
    CAPACITE          NUMBER(6)      NOT NULL CHECK (CAPACITE >= 1),
    SPECIALITE        VARCHAR2(250),
    HORS_SERVICE      NUMBER(1)      DEFAULT 0 NOT NULL CHECK (HORS_SERVICE IN (0, 1)),
    -- colonnes utilisees par les metiers du module Locaux (occupation + suivi du nettoyage)
    OCCUPATION        NUMBER(6)      DEFAULT 0 NOT NULL,
    NETTOYAGE         NUMBER(1)      DEFAULT 0 NOT NULL CHECK (NETTOYAGE IN (0, 1, 2)), -- propre / a nettoyer / en cours
    DERNIER_NETTOYAGE VARCHAR2(35),
    SALE_DEPUIS       VARCHAR2(35),
    ID_SERV           NUMBER(10)     NOT NULL,
    CONSTRAINT CK_LOCAUX_OCCUPATION CHECK (OCCUPATION BETWEEN 0 AND CAPACITE),
    CONSTRAINT FK_LOCAUX_SERVICES FOREIGN KEY (ID_SERV) REFERENCES SERVICES (ID_SERV)
);

-- 4) STOCK  (chaque article est range dans 1 local)
CREATE TABLE STOCK (
    ID_ARTICLE      NUMBER(10)     CONSTRAINT PK_STOCK PRIMARY KEY,
    ARTICLE         VARCHAR2(50)   NOT NULL,                        -- categorie de l'article
    NOM_ARTICLE     VARCHAR2(100)  NOT NULL,
    LOT             VARCHAR2(30),
    QUANTITE_DISP   NUMBER(8)      DEFAULT 0 NOT NULL CHECK (QUANTITE_DISP >= 0),
    UNITE           VARCHAR2(20),
    SEUIL_MIN       NUMBER(8)      DEFAULT 0 CHECK (SEUIL_MIN >= 0),
    DATE_ECHEANCE   DATE,
    ID_LOCAL        NUMBER(10)     NOT NULL,
    CONSTRAINT FK_STOCK_LOCAUX FOREIGN KEY (ID_LOCAL) REFERENCES LOCAUX (ID_LOCAL)
);

-- 5) DOSSIER_MEDICAL  (chaque dossier est realise par 1 employe)
CREATE TABLE DOSSIER_MEDICAL (
    ID_DOSS            NUMBER(10)     CONSTRAINT PK_DOSSIER_MEDICAL PRIMARY KEY,
    DATE_CONSULTATION  DATE           NOT NULL,
    MOTIF              VARCHAR2(200),
    DIAGNOSTIC         VARCHAR2(500),
    ID_ANIMAL          NUMBER(10)     NOT NULL,
    NOM_ANIMAL         VARCHAR2(50)   NOT NULL,
    ESPECE             VARCHAR2(50),
    RACE               VARCHAR2(50),
    AGE                NUMBER(3)      CHECK (AGE >= 0),
    SEXE               VARCHAR2(1)    CHECK (SEXE IN ('M', 'F')),
    PROPRIETAIRE       VARCHAR2(100),
    ID_EMP             NUMBER(10)     NOT NULL,
    CONSTRAINT FK_DOSSIER_EMPLOYES FOREIGN KEY (ID_EMP) REFERENCES EMPLOYES (ID_EMP)
);

-- 6) Donnees de depart : 2 services (necessaires pour ajouter des locaux et des employes)
INSERT INTO SERVICES VALUES (1, 'Clinique ANIWORLD', 'Consultation', 'ACTIF',
                             TO_DATE('2026-10-29 09:00', 'YYYY-MM-DD HH24:MI'), 30, 40, 'Consultation generale');
INSERT INTO SERVICES VALUES (2, 'Clinique ANIWORLD', 'Chirurgie', 'ACTIF',
                             TO_DATE('2026-10-29 10:00', 'YYYY-MM-DD HH24:MI'), 120, 250, 'Chirurgie programmee');

COMMIT;

-- 7) Verification : les 5 tables doivent apparaitre
SELECT table_name FROM user_tables ORDER BY table_name;
