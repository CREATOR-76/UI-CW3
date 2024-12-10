#include "file_select_widget.hpp"
#include <qpushbutton.h>

FileSelectWidget::FileSelectWidget(QWidget *parent) {
  layout = new QHBoxLayout(this);

  label = new QLabel();
  label->setText("Select File");
  layout->addWidget(label);

  load = new QPushButton();
  load->setText("load data");
  layout->addWidget(load);

  label->setToolTip("select a CSV File");
  load->setToolTip("load data");

  connect(load, &QPushButton::clicked, this, &FileSelectWidget::onLoadClicked);
}

void FileSelectWidget::openCSV() {
  filename = QFileDialog::getOpenFileName(this, "Select a CSV file", QString(),
                                          "CSV files (*.csv);;All Files (*)");

  if (!filename.isEmpty()) {
    // get last 20 characters of filename
    label->setText(filename.mid(filename.length() - 20, 20));
  }
}

void FileSelectWidget::mousePressEvent(QMouseEvent *event) {
  if (label->geometry().contains(event->pos()))
    openCSV();
  else
    QWidget::mousePressEvent(event);
}

void FileSelectWidget::onLoadClicked() { emit fileSelected(filename); }
