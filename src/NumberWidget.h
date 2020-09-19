#pragma once

#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>

namespace ray_tracing {

template<typename Scalar>
class NumberWidget : public QLineEdit {
public:
  NumberWidget(Scalar value, QWidget *parent = 0);
  virtual ~NumberWidget() = default;

  Scalar GetValue();

private:
  void SetValidator();
};


template<typename Scalar>
NumberWidget<Scalar>::NumberWidget(Scalar value, QWidget *parent)
  : QLineEdit(parent) {

  SetValidator();
}

template<typename Scalar>
void NumberWidget<Scalar>::SetValidator() {
  if (std::is_same<Scalar, int32_t>::value) {
    this->setValidator(new QIntValidator(this));
  } else if (std::is_same<Scalar, uint32_t>::value) {
    this->setValidator(new QIntValidator(
      0, std::numeric_limits<uint32_t>::max(), this));
  } else if (std::is_same<Scalar, float>::value) {
    this->setValidator(new QDoubleValidator(this));
  } else if (std::is_same<Scalar, double>::value) {
    this->setValidator(new QDoubleValidator(this));
  } else {
    throw std::runtime_error("Unsopported NumberWidget scalar type");
  }
}

template<typename Scalar>
Scalar NumberWidget<Scalar>::GetValue() {
  Scalar ret = (Scalar)(this->text().toFloat());
  return ret;
}

}
