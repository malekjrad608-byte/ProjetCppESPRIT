-- Script de creation de la base de donnees GestionService
-- MySQL / MariaDB

CREATE DATABASE IF NOT EXISTS gestion_service
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE gestion_service;

CREATE TABLE IF NOT EXISTS employees (
    id        INT AUTO_INCREMENT PRIMARY KEY,
    firstName VARCHAR(100) NOT NULL,
    lastName  VARCHAR(100) NOT NULL,
    poste     VARCHAR(100),
    phone     VARCHAR(30),
    email     VARCHAR(150)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS services (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    name        VARCHAR(200) NOT NULL,
    description TEXT,
    employeeId  INT,
    dateTime    DATETIME,
    status      VARCHAR(50) DEFAULT 'Planifie',
    FOREIGN KEY (employeeId) REFERENCES employees(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS schedules (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    employeeId  INT NOT NULL,
    date        DATE NOT NULL,
    shift       VARCHAR(30) NOT NULL,
    note        TEXT,
    FOREIGN KEY (employeeId) REFERENCES employees(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Donnees de test
INSERT INTO employees (firstName, lastName, poste, phone, email) VALUES
    ('Sophie', 'Dubois', 'Technicien', '0612345678', 'sophie.dubois@example.com'),
    ('Pierre', 'Martin', 'Responsable', '0698765432', 'pierre.martin@example.com'),
    ('Claire', 'Moreau', 'Ingenieur', '0654321098', 'claire.moreau@example.com');

INSERT INTO services (name, description, employeeId, dateTime, status) VALUES
    ('Maintenance equipement', 'Reparation reguliere de l equipement', 1, NOW(), 'En cours'),
    ('Formation employes', 'Seance d integration nouveaux employes', 3, DATE_ADD(NOW(), INTERVAL 1 DAY), 'Planifie'),
    ('Audit informatique', 'Verification systemes et securite', 2, DATE_SUB(NOW(), INTERVAL 2 DAY), 'Termine');

INSERT INTO schedules (employeeId, date, shift, note) VALUES
    (1, CURDATE(), 'Matin', 'Poste habituel'),
    (2, CURDATE(), 'Apres-midi', ''),
    (3, CURDATE(), 'Nuit', 'Remplacement'),
    (1, DATE_ADD(CURDATE(), INTERVAL 1 DAY), 'Matin', ''),
    (2, DATE_ADD(CURDATE(), INTERVAL 1 DAY), 'Matin', '');