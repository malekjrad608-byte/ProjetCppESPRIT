#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include "../models/animal.h"
#include "../models/medicalrecord.h"
#include "../models/medicalanalytics.h"

class MedicalRecordsWidget : public QWidget {
    Q_OBJECT
public:
    explicit MedicalRecordsWidget(QWidget *parent = nullptr);
    void refresh();

signals:
    void recordsChanged();

private slots:
    void onAddAnimal();
    void onEditAnimal();
    void onDeleteAnimal();
    void onViewAnimal();
    void onAddMedicalRecord();
    void onAddVaccination();
    void onSearch();
    void onFilterChanged();
    void onSortChanged();
    void onExportPDF();
    void onExportExcel();
    void onAnimalSelected();
    void showDashboard();
    void showAnimalsList();
    void showAnimalDetails(int animalId);
    void updateNavigationButtons();
    void applyStyles();

private:
    void setupUi();
    void setupDashboard();
    void setupAnimalsList();
    void setupAnimalDetails();
    QLabel *createStatCard(const QString &label, const QString &value, const QString &icon);
    void updateAnalytics();
    void populateAnimalsTable();
    void populateMedicalRecordsTable(int animalId);
    void populateVaccinationsTable(int animalId);
    void applyFilters();
    Animal getSelectedAnimal() const;
    void updateButtons();

    QStackedWidget *m_stack;
    QWidget *m_dashboardPage;
    QWidget *m_animalsListPage;
    QWidget *m_animalDetailsPage;

    // Dashboard
    QLabel *m_totalAnimals;
    QLabel *m_totalRecords;
    QLabel *m_totalVaccinations;
    QLabel *m_vaccinationsDueSoon;
    QLabel *m_vaccinationsOverdue;
    QLabel *m_totalCost;

    // Animals list
    QTableWidget *m_animalsTable;
    QLineEdit *m_searchInput;
    QComboBox *m_speciesFilter;
    QComboBox *m_sortCombo;

    // Animal details
    QLabel *m_animalName;
    QLabel *m_animalInfo;
    QTableWidget *m_medicalRecordsTable;
    QTableWidget *m_vaccinationsTable;

    QList<Animal> m_animals;
    QList<Animal> m_filteredAnimals;
    Animal m_currentAnimal;
    MedicalAnalytics m_analytics;
};
