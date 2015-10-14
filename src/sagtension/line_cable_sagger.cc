// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#include "models/sagtension/line_cable_sagger.h"

#include "models/sagtension/line_cable_reloader.h"

LineCableSagger::LineCableSagger() {
  line_cable_ = nullptr;

  index_constraint_controlling_ = -9999;
  is_updated_linecable_constraint_limit_ = false;
}

LineCableSagger::~LineCableSagger() {
}

double LineCableSagger::CapacityAllowable(const int& index) const {
  // updates class if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  CableConstraint constraint = constraints_design_.at(index);
  Catenary3d catenary = catenaries_constraints_actual_.at(index);

  double capacity_allowable = -999999;
  if (constraint.type_limit
      == CableConstraint::LimitType::kHorizontalTension) {
    capacity_allowable = catenary.tension_horizontal() / constraint.limit;
  } else if (constraint.type_limit
             == CableConstraint::LimitType::kCatenaryConstant) {
    capacity_allowable = catenary.Constant() / constraint.limit;
  } else if (constraint.type_limit
             == CableConstraint::LimitType::kSupportTension) {
    capacity_allowable = catenary.TensionMax() / constraint.limit;
  }

  return capacity_allowable;
}

double LineCableSagger::CatenaryConstantActual(const int& index) const {
  // updates class if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  Catenary3d catenary = catenaries_constraints_actual_.at(index);
  return catenary.Constant();
}

int LineCableSagger::IndexConstraintControlling() const {
  // updates class if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -9999;
    }
  }

  return index_constraint_controlling_;
}

double LineCableSagger::LimitConstraintSaggedLineCable() const {
  // updates class if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  return line_cable_->constraint.limit;
}

double LineCableSagger::TensionHorizontalActual(const int& index) const {
  // updates class if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  Catenary3d catenary = catenaries_constraints_actual_.at(index);
  return catenary.tension_horizontal();
}

double LineCableSagger::TensionSupportActual(const int& index) const {
  // updates class if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  Catenary3d catenary = catenaries_constraints_actual_.at(index);
  return catenary.TensionMax();
}

bool LineCableSagger::Validate(const bool& is_included_warnings,
                std::list<std::string>* messages_error) const {
  bool is_valid = true;

  // validates constraints-design
  for (auto iter = constraints_design_.cbegin();
       iter != constraints_design_.cend(); iter++) {
    const CableConstraint& constraint = *iter;
    if (constraint.Validate(is_included_warnings, messages_error) == false) {
      is_valid = false;
    }
  }

  // validates line cable
  if (line_cable_ == nullptr) {
    is_valid = false;
    if (messages_error != nullptr) {
      messages_error->push_back("LINE CABLE SAGGER - Invalid line cable");
    }
  } else {
    if (line_cable_->Validate(is_included_warnings, messages_error) == false) {
      is_valid = false;
    }
  }

  return is_valid;
}

std::vector<CableConstraint> LineCableSagger::constraints_design() const {
  return constraints_design_;
}

LineCable* LineCableSagger::line_cable() const {
  return line_cable_;
}

void LineCableSagger::set_constraints_design(
      const std::vector<CableConstraint> constraints_design) {
  constraints_design_ = constraints_design;

  is_updated_linecable_constraint_limit_ = false;
}

void LineCableSagger::set_line_cable(LineCable* line_cable) {
  line_cable_ = line_cable;

  is_updated_linecable_constraint_limit_ = false;
}

bool LineCableSagger::IsUpdated() const {
  if (is_updated_linecable_constraint_limit_ == true) {
    return true;
  } else {
    return false;
  }
}

bool LineCableSagger::Update() const {
  // updates line cable constraint limit
  if (is_updated_linecable_constraint_limit_ == false) {

    is_updated_linecable_constraint_limit_ = UpdateLineCableConstraintLimit();
    if (is_updated_linecable_constraint_limit_ == false) {
      return false;
    }

    is_updated_linecable_constraint_limit_ =
        UpdateCatenariesConstraintsActual();
    if (is_updated_linecable_constraint_limit_ == false) {
      return false;
    }
  }

  // if it reaches this point, update was successful
  return true;
}

/// This function reloads each design constraint at the line cable constraint
/// loading and flags the constraint with the lowest reloaded horizontal tension
/// as the controlling constraint.
bool LineCableSagger::UpdateControllingConstraintIndex() const {
  // checks for multiple constraints to determine if further analysis is
  // required
  if (constraints_design_.size() == 1) {
    index_constraint_controlling_ = 1;
    return true;
  }

  // reloads the design constraint at the line cable constraint weathercase and
  // condition
  LineCableReloader reloader;
  reloader.set_weathercase_reloaded(line_cable_->constraint.case_weather);
  reloader.set_condition_reloaded(line_cable_->constraint.condition);
  reloader.set_length_unloaded_unstretched_adjustment(0);

  // reloads each constraint in the collection and looks for the lowest
  // reloaded horizontal tension
  double tension_horizontal_controlling = 999999;
  index_constraint_controlling_ = -9999;

  for (auto iter = constraints_design_.cbegin();
       iter != constraints_design_.cend(); iter++) {
    const CableConstraint& constraint = *iter;

    // creates a line cable with the design constraint and updates reloader
    LineCable line_cable = LineCable(*line_cable_);
    line_cable.constraint = constraint;
    reloader.set_line_cable(&line_cable);

    // solves for the reloaded horizontal tension, and compares to previous
    // values to determine controlling constraint
    const double tension_horizontal = reloader.TensionHorizontal();
    if (tension_horizontal == -999999) {
      return false;
    }

    if (tension_horizontal < tension_horizontal_controlling) {
      tension_horizontal_controlling = tension_horizontal;
      index_constraint_controlling_ = iter - constraints_design_.cbegin();
    }
  }

  return true;
}

/// This function reloads the controlling design constraint at the line cable
/// constraint loading. The reloaded value is the new constraint limit.
bool LineCableSagger::UpdateLineCableConstraintLimit() const {
  // updates controlling constraint index
  if (UpdateControllingConstraintIndex() == false) {
    return false;
  }

  // reloads controlling constraint at the line cable constraint weathercase and
  // condition
  LineCableReloader reloader;
  reloader.set_weathercase_reloaded(line_cable_->constraint.case_weather);
  reloader.set_condition_reloaded(line_cable_->constraint.condition);
  reloader.set_length_unloaded_unstretched_adjustment(0);

  // creates a line cable with the controlling design constraint and updates
  // reloader
  LineCable line_cable = LineCable(*line_cable_);
  line_cable.constraint = constraints_design_.at(index_constraint_controlling_);
  reloader.set_line_cable(&line_cable);

  // updates line cable constraint limit
  if (reloader.Validate(false, nullptr) == false) {
    return false;
  }

  const Catenary3d catenary = reloader.CatenaryReloaded();
  CableConstraint constraint = line_cable_->constraint;

  if (line_cable_->constraint.type_limit
      == CableConstraint::LimitType::kHorizontalTension) {
    constraint.limit = catenary.tension_horizontal();
  } else if (line_cable_->constraint.type_limit
      == CableConstraint::LimitType::kCatenaryConstant) {
    constraint.limit = catenary.Constant();
  } else if (line_cable_->constraint.type_limit
      == CableConstraint::LimitType::kSupportTension) {
    constraint.limit = catenary.TensionMax();
  }
  line_cable_->constraint = constraint;

  return true;
}

bool LineCableSagger::UpdateCatenariesConstraintsActual() const {
  // builds a reloader with the reloaded case set to each design constraint
  // loading
  LineCableReloader reloader;
  reloader.set_length_unloaded_unstretched_adjustment(0);
  reloader.set_line_cable(line_cable_);

  catenaries_constraints_actual_.clear();
  for (auto iter = constraints_design_.cbegin();
      iter != constraints_design_.cend(); iter++) {
    const CableConstraint constraint = *iter;

    // updates reloader with design constraint loading
    reloader.set_weathercase_reloaded(constraint.case_weather);
    reloader.set_condition_reloaded(constraint.condition);

    // gets reloaded catenary and adds to collection
    if (reloader.Validate(false, nullptr) == false) {
      return false;
    }
    Catenary3d catenary = reloader.CatenaryReloaded();
    catenaries_constraints_actual_.push_back(catenary);
  }

  return true;
}
