#pragma once
#include <QIcon>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QTableWidget>

namespace Design {
QLabel *label(const QString &text, const char *role = nullptr, QWidget *parent = nullptr);
QPushButton *button(const QString &text, const QString &iconName = {}, const char *role = nullptr);
QFrame *card(const char *name = nullptr);
QIcon icon(const QString &name, const QColor &color = QColor("#58796A"));
QPixmap brand(const QString &name, const QSize &size);
QWidget *badge(const QString &status);
QString money(double amount);
void setupTable(QTableWidget *table, const QStringList &headers);
void applyTheme();
QStringList categories();
QStringList statuses();
}
