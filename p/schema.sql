-- Table MEDICAL attendue par l'application (Oracle).
CREATE TABLE MEDICAL (
    ID_DOSSIER         VARCHAR2(20) PRIMARY KEY,
    DATE_CONSULTATION  DATE,
    MOTIF              VARCHAR2(500),
    DIAGNOSTIC         VARCHAR2(500),
    ID_ANIMAL          VARCHAR2(20),
    NOM_ANIMAL         VARCHAR2(100),
    ESPECE             VARCHAR2(50),
    RACE               VARCHAR2(100),
    AGE                NUMBER(3),
    POIDS              NUMBER(7,2),
    PROPRIETAIRE       VARCHAR2(100),
    ID_EMPLOYES        VARCHAR2(20)
);
