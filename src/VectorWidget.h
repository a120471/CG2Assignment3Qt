#pragma once

#include <Eigen/Core>
#include <QWidget>
#include <QBoxLayout>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>

namespace ray_tracing {

template<typename Scalar>
class VectorWidget : public QWidget {
public:
  VectorWidget(const std::vector<Scalar> &values, QWidget *parent = 0);
  virtual ~VectorWidget();

  Eigen::Matrix<Scalar, Eigen::Dynamic, 1> GetValue();

private:
  void SetValidator();

  std::vector<QLineEdit*> line_edit_;
};


template<typename Scalar>
VectorWidget<Scalar>::VectorWidget(const std::vector<Scalar> &values,
  QWidget *parent)
  : QWidget(parent) {

  assert(line_edit_.empty());
  auto layout = new QHBoxLayout(this);
  layout->setMargin(0);
  for (auto v : values) {
    auto w = new QLineEdit(QString::number(v), parent);
    layout->addWidget(w);
    line_edit_.emplace_back(w);
  }
  SetValidator();
}

template<typename Scalar>
void VectorWidget<Scalar>::SetValidator() {
  for (auto w : line_edit_) {
    if (std::is_same<Scalar, int32_t>::value) {
      w->setValidator(new QIntValidator(w));
    } else if (std::is_same<Scalar, uint32_t>::value) {
      w->setValidator(new QIntValidator(
        0, std::numeric_limits<uint32_t>::max(), w));
    } else if (std::is_same<Scalar, float>::value) {
      w->setValidator(new QDoubleValidator(w));
    } else if (std::is_same<Scalar, double>::value) {
      w->setValidator(new QDoubleValidator(w));
    } else {
      throw std::runtime_error("Unsopported VectorWidget scalar type");
    }
  }
}

template<typename Scalar>
VectorWidget<Scalar>::~VectorWidget() {
  for (auto w : line_edit_) {
    if (w) {
      w->deleteLater();
    }
  }
}

template<typename Scalar>
Eigen::Matrix<Scalar, Eigen::Dynamic, 1> VectorWidget<Scalar>::GetValue() {
  Eigen::Matrix<Scalar, Eigen::Dynamic, 1> ret(line_edit_.size());
  for (auto i = 0; i < line_edit_.size(); ++i) {
    ret(i) = (Scalar)(line_edit_[i]->text().toFloat());
  }
  return ret;
}

}
