#ifndef QFILEREQUESTER_H
#define QFILEREQUESTER_H


#include <QWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

class QFileRequester : public QWidget
{
Q_OBJECT
public:
    explicit QFileRequester(QString str, QWidget *aParent);
    void setText(QString str);
    QString text();
signals:
    void changed();
private:
    QHBoxLayout *hbox;
    QPushButton *btn;
    QLineEdit *prop;
private slots:
    void openDialog();
    void textChanged();

};
#endif // QFILEREQUESTER_H
