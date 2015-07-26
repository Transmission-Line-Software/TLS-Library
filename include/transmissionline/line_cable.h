// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#ifndef TLSLIBRARIES_TRANSMISSIONLINE_LINECABLE_H_
#define TLSLIBRARIES_TRANSMISSIONLINE_LINECABLE_H_

/// \par OVERVIEW
///
/// This struct models a transmission cable line section (multiple spans), where
/// the cable is strung from one terminal (dead end) attachment to another.
///
/// \par CONSTRAINT
///
/// This class is defined by the cable, and constraint that it is tensioned to.

#include <list>
#include <string>

#include "base/vector.h"
#include "transmissionline/cable.h"
#include "transmissionline/cable_constraint.h"

struct LineCable {
 public:
  /// \brief Default constructor.
  LineCable();

  /// \brief Destructor.
  ~LineCable();

  /// \brief Validates member variables.
  /// \param[in] is_included_warnings
  ///   A flag that tightens the acceptable value range.
  /// \param[in,out] messages_error
  ///   A list of detailed error messages. If this is provided, any validation
  ///   errors will be appended to the list.
  /// \return A boolean value indicating status of member variables.
  bool Validate(const bool& is_included_warnings = true,
                std::list<std::string>* messages_error = nullptr) const;

  /// \var cable_
  ///   The cable.
  Cable cable;

  /// \var constraint_
  ///   The constraint the cable is tensioned to.
  CableConstraint constraint;

  /// \var spacing_attachments_ruling_span_
  ///   The attachment spacing for the ruling span geometry.
  Vector3d spacing_attachments_ruling_span;
};

#endif // TLSLIBRARIES_TRANSMISSIONLINE_LINECABLE_H_