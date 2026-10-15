-- ANIWORLD - table LOCAUX (a executer dans SQL Developer, connecte avec l'utilisateur "aniworld")
-- Si la table existe deja, ne rien faire (ou DROP TABLE LOCAUX avant).

CREATE TABLE LOCAUX (
    id_local     NUMBER(10)     PRIMARY KEY,
    nom_local    VARCHAR2(60)   NOT NULL,
    type_local   VARCHAR2(30)   NOT NULL,
    emplacement  VARCHAR2(30),
    capacite     NUMBER(3)      DEFAULT 1 NOT NULL,
    statut       VARCHAR2(20)   DEFAULT 'AVAILABLE' NOT NULL,
    description  CLOB,
    CONSTRAINT uq_locaux_nom      UNIQUE (nom_local),
    CONSTRAINT ck_locaux_capacite CHECK (capacite BETWEEN 1 AND 50),
    CONSTRAINT ck_locaux_statut   CHECK (statut IN ('AVAILABLE', 'OCCUPIED', 'MAINTENANCE'))
);

-- Test rapide
SELECT * FROM LOCAUX;
