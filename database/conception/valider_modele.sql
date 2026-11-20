-- Jeu fictif de validation ; rollback final, aucune donnee metier conservee.
whenever sqlerror exit failure rollback
set serveroutput on
set define off
DECLARE
  n NUMBER;
  tests NUMBER:=0;
  PROCEDURE ok(label VARCHAR2) IS BEGIN tests:=tests+1; DBMS_OUTPUT.PUT_LINE('PASS '||label); END;
  PROCEDURE refuse(label VARCHAR2,stmt VARCHAR2,expected NUMBER) IS
    rejected BOOLEAN:=FALSE;
  BEGIN
    BEGIN EXECUTE IMMEDIATE stmt;
    EXCEPTION WHEN OTHERS THEN
      IF SQLCODE<>expected THEN RAISE; END IF;
      rejected:=TRUE;
    END;
    IF NOT rejected THEN RAISE_APPLICATION_ERROR(-20999,'Contrainte absente: '||label); END IF;
    ok(label);
  END;
BEGIN
  IF SYS_CONTEXT('USERENV','CURRENT_SCHEMA')<>'ANIWORLD_S5_MODEL' THEN
    RAISE_APPLICATION_ERROR(-20990,'Test reserve au schema ANIWORLD_S5_MODEL');
  END IF;
  SELECT COUNT(*) INTO n FROM ALL_TABLES WHERE owner='ANIWORLD_S5_MODEL';
  IF n<>23 THEN RAISE_APPLICATION_ERROR(-20991,'Nombre de tables incorrect'); END IF;
  ok('23 tables');
  SELECT COUNT(*) INTO n FROM ALL_OBJECTS WHERE owner='ANIWORLD_S5_MODEL' AND status<>'VALID';
  IF n<>0 THEN RAISE_APPLICATION_ERROR(-20992,'Objets invalides'); END IF;
  ok('Tous les objets compiles');
  INSERT INTO CLIENT(id_client,nom) VALUES(900001,'TEST fictif');
  INSERT INTO ANIMAL(id_animal,id_client,nom,espece,sexe) VALUES(900001,900001,'Animal test','Chat','INCONNU');
  INSERT INTO EMPLOYE(id_employe,nom,prenom,poste) VALUES(900001,'Test','Fictif','Veterinaire');
  INSERT INTO LOCAL(id_local,nom,type_local,capacite) VALUES(900001,'Local test','Soins',1);
  INSERT INTO COMPTE(id_compte,login,role,sel,empreinte) VALUES(900001,'test_s5','STOCK',HEXTORAW('00'),HEXTORAW('00'));
  INSERT INTO SERVICE(id_service,id_animal,libelle,categorie,debut,duree_minutes,tarif_tnd)
    VALUES(900001,900001,'Consultation test','Soins',SYSTIMESTAMP,30,12.345);
  INSERT INTO DOSSIER_MEDICAL(id_dossier_medical,id_animal) VALUES(900001,900001);
  INSERT INTO CONSULTATION(id_consultation,id_dossier_medical,id_service,id_employe,date_consultation)
    VALUES(900001,900001,900001,900001,SYSTIMESTAMP);
  INSERT INTO FACTURE(id_facture,id_consultation,id_client,numero,nom_client_facture)
    VALUES(900001,900001,900001,'TEST-S5','Client fictif');
  INSERT INTO LIGNE_FACTURE VALUES(900001,1,'Prestation test',2,12.345);
  SELECT total_tnd INTO n FROM V_TOTAL_FACTURE WHERE id_facture=900001;
  IF n<>24.690 THEN RAISE_APPLICATION_ERROR(-20993,'Precision monetaire incorrecte'); END IF;
  ok('Facture avec precision TND 3 decimales');
  refuse('Un dossier par animal','INSERT INTO DOSSIER_MEDICAL(id_animal) VALUES(900001)',-1);
  refuse('Une facture par consultation',q'[INSERT INTO FACTURE(id_consultation,id_client,numero,nom_client_facture) VALUES(900001,900001,'TEST-2','Test')]',-1);
  refuse('FK animal obligatoire',q'[INSERT INTO SERVICE(id_animal,libelle,categorie,debut,duree_minutes) VALUES(999999,'Test','Soins',SYSTIMESTAMP,30)]',-2291);
  refuse('Tarif negatif',q'[UPDATE SERVICE SET tarif_tnd=-1 WHERE id_service=900001]',-2290);
  refuse('Duree positive',q'[UPDATE SERVICE SET duree_minutes=0 WHERE id_service=900001]',-2290);
  refuse('Ressources avant demarrage',q'[UPDATE SERVICE SET statut='EN_COURS' WHERE id_service=900001]',-2290);
  refuse('Motif annulation obligatoire',q'[UPDATE SERVICE SET statut='ANNULE' WHERE id_service=900001]',-2290);
  refuse('Paiement date',q'[UPDATE FACTURE SET statut='PAYEE' WHERE id_facture=900001]',-2290);
  refuse('Login insensible a la casse',q'[INSERT INTO COMPTE(login,role,sel,empreinte) VALUES('TEST_S5','ADMIN',HEXTORAW('00'),HEXTORAW('00'))]',-1);
  refuse('Echeance vaccinale coherente',q'[INSERT INTO VACCINATION(id_dossier_medical,id_employe,produit,date_vaccination,prochaine_echeance) VALUES(900001,900001,'Test',DATE '2026-10-14',DATE '2026-10-13')]',-2290);
  INSERT INTO STOCK(id_stock,nom) VALUES(900001,'Stock test');
  INSERT INTO ARTICLE(id_article,reference,designation,categorie,unite) VALUES(900001,'TEST-S5','Article test','Soins','unite');
  INSERT INTO LOT_STOCK(id_lot_stock,id_article,id_stock,numero_lot,quantite) VALUES(900001,900001,900001,'LOT-TEST',5);
  INSERT INTO LOT_STOCK(id_lot_stock,id_article,id_stock,numero_lot,quantite,bloque) VALUES(900002,900001,900001,'LOT-BLOQUE',10,1);
  SELECT disponible INTO n FROM V_STOCK_ARTICLE WHERE id_article=900001;
  IF n<>5 THEN RAISE_APPLICATION_ERROR(-20994,'Stock bloque inclus'); END IF;
  ok('Stock disponible exclut les lots bloques');
  refuse('Pas de stock negatif','UPDATE LOT_STOCK SET quantite=-1 WHERE id_lot_stock=900001',-2290);
  INSERT INTO MOUVEMENT_STOCK(id_mouvement_stock,id_lot_stock,id_compte,type_mouvement,quantite,motif) VALUES(900001,900001,900001,'ENTREE',5,'Test fictif');
  refuse('Mouvement non modifiable','UPDATE MOUVEMENT_STOCK SET quantite=7 WHERE id_mouvement_stock=900001',-20020);
  refuse('Mouvement non supprimable','DELETE FROM MOUVEMENT_STOCK WHERE id_mouvement_stock=900001',-20020);
  INSERT INTO ADOPTANT(id_adoptant,id_client,logement) VALUES(900001,900001,'Maison test');
  INSERT INTO ADOPTION(id_adoption,id_animal,id_adoptant,id_employe) VALUES(900001,900001,900001,900001);
  refuse('Validation adoption complete',q'[UPDATE ADOPTION SET statut='VALIDEE' WHERE id_adoption=900001]',-2290);
  UPDATE ADOPTION SET statut='VALIDEE',date_decision=SYSDATE,pieces_verifiees=1,rencontre_effectuee=1,accord_responsable=1 WHERE id_adoption=900001;
  refuse('Une adoption validee active par animal',q'[INSERT INTO ADOPTION(id_animal,id_adoptant,id_employe,statut,date_decision,pieces_verifiees,rencontre_effectuee,accord_responsable) VALUES(900001,900001,900001,'VALIDEE',SYSDATE,1,1,1)]',-1);
  refuse('Suppression parent referencee','DELETE FROM CLIENT WHERE id_client=900001',-2292);
  DBMS_OUTPUT.PUT_LINE('RESULTAT: '||tests||' controles reussis');
  ROLLBACK;
END;
/
SELECT COUNT(*) AS lignes_test_restantes FROM CLIENT;
-- Attendu : 0. Les sequences IDENTITY peuvent avoir avance ; c'est normal.
