#ifndef PREFS_H
#define PREFS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class Prefs;
}

class Prefs : public QDialog
{
    Q_OBJECT

public:
    explicit Prefs(QWidget *parent = nullptr);
    ~Prefs();

private slots:
    void on_pb_apply_clicked();
    void on_tb_timeColor_clicked();
    void on_tb_dateColor_clicked();
    void on_tb_hourColor_clicked();
    void on_tb_minuteColor_clicked();


signals:
    void updateSettings(void);

private:
    Ui::Prefs *ui;
    QString m_timecolor;
    QString m_datecolor;
    QString m_hourcolor;
    QString m_minutecolor;
    QString m_textfont;
};

#endif // PREFS_H
