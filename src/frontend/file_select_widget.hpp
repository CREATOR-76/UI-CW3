#pragma once

#include <QtWidgets>

class FileSelectWidget : public QWidget {
  Q_OBJECT

public:
  FileSelectWidget(QWidget *parent = nullptr);

signals:
  void fileSelected(QString &filename);

private:
  QHBoxLayout *layout;
  QLabel *label;
  QString filename;
  QPushButton *load;

  void openCSV();
  void onLoadClicked();

protected:
  void mousePressEvent(QMouseEvent *event);
};
