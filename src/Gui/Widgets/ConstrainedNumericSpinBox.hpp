#pragma once
#include <Gui/RaGui.hpp>

#include <QWidget>

#include <Gui/Widgets/QtTypeWrapper.hpp>

namespace Ra {
namespace Gui {
namespace Widgets {

/**
 * \brief Constrained input spin box.
 * The constraint to apply to any input value is verified using the user-define predicate associated
 * to the object. \tparam T Type of the constrained value to input
 */
template <typename T>
class ConstrainedNumericSpinBox : public QtSpinBox::getType<T>::Type
{
  public:
    using BaseWidget = typename QtSpinBox::getType<T>::Type;
    using Predicate  = std::function<bool( T )>;
    using BaseWidget::BaseWidget;
    QValidator::State validate( QString& input, int& ) const override;

    /// Set the predicate to evaluate
    inline void setPredicate( Predicate p ) { m_p = p; }
    /**
     * \brief Validate a value
     * \param s
     * \return true if the value is valid according to the stored predicate, false otherwise.
     */
    inline bool isValid( T s ) const { return m_p( s ); }

  private:
    Predicate m_p = []( T ) { return true; };
};

} // namespace Widgets
} // namespace Gui
} // namespace Ra

#include <Gui/Widgets/ConstrainedNumericSpinBox.inl>
