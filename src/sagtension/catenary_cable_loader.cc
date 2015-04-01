// This is free and unencumbered software released into the public domain.
// For more information, please refer to <http://unlicense.org/>

#include "sagtension/catenary_cable_loader.h"

CatenaryCableLoader::CatenaryCableLoader() {
  is_updated_catenary_ = false;
}

CatenaryCableLoader::~CatenaryCableLoader() {
}

double CatenaryCableLoader::LengthLoaded() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  return strainer_cable_.LengthFinish();
}

double CatenaryCableLoader::LoadAverage() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  return strainer_cable_.load_finish();
}

double CatenaryCableLoader::LoadAverageCore() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  return strainer_cable_.LoadFinishCore();
}

double CatenaryCableLoader::LoadAverageShell() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  return strainer_cable_.LoadFinishShell();
}

double CatenaryCableLoader::StrainTransitionLoad() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  return strainer_cable_.StrainTransitionLoad();
}

double CatenaryCableLoader::StrainTransitionThermal() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }

  return strainer_cable_.StrainTransitionThermal();
}

double CatenaryCableLoader::TensionHorizontalCatenary() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }


}

double CatenaryCableLoader::TensionHorizontalCatenaryCore() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }


}

double CatenaryCableLoader::TensionHorizontalCatenaryShell() const {

  // update class, if necessary
  if (IsUpdated() == false) {
    if (Update() == false) {
      return -999999;
    }
  }


}

double CatenaryCableLoader::length_unloaded() const {
  return strainer_cable_.length_start();
}

double CatenaryCableLoader::load_stretch() const {
  return strainer_cable_.load_stretch();
}

void CatenaryCableLoader::set_cable(const Cable& cable) {

  strainer_cable_.set_cable(cable);

  is_updated_catenary_ = false;
}

void CatenaryCableLoader::set_length_unloaded(const double& length_unloaded) {

  strainer_cable_.set_length_start(length_unloaded);

  is_updated_catenary_ = false;
}

void CatenaryCableLoader::set_load_stretch(const double& load_stretch) {

  strainer_cable_.set_load_stretch(load_stretch);

  is_updated_catenary_ = false;
}

void CatenaryCableLoader::set_spacing_endpoints_catenary(
    const Vector3d& spacing_endpoints) {

  catenary_.set_spacing_endpoints(spacing_endpoints);

  is_updated_catenary_ = false;
}

void CatenaryCableLoader::set_state_loaded(
    const CableStrainerState& state_loaded) {

  strainer_cable_.set_state_finish(state_loaded);

  is_updated_catenary_ = false;
}

void CatenaryCableLoader::set_state_unloaded(
    const CableStrainerState& state_unloaded) {

  strainer_cable_.set_state_start(state_unloaded);

  is_updated_catenary_  = false;
}

void CatenaryCableLoader::set_temperature_stretch(
    const double& temperature_stretch) {

  strainer_cable_.set_temperature_stretch(temperature_stretch);

  is_updated_catenary_ = false;
}

void CatenaryCableLoader::set_weight_unit_catenary(
    const Vector3d& weight_unit) {

  catenary_.set_weight_unit(weight_unit);

  is_updated_catenary_ = false;
}

Vector3d CatenaryCableLoader::spacing_endpoints_catenary() const {
  return catenary_.spacing_endpoints();
}

CableStrainerState CatenaryCableLoader::state_loaded() const {
  return strainer_cable_.state_finish();
}

CableStrainerState CatenaryCableLoader::state_unloaded() const {
  return strainer_cable_.state_start();
}

double CatenaryCableLoader::temperature_stretch() const {
  return strainer_cable_.temperature_stretch();
}

Vector3d CatenaryCableLoader::weight_unit_catenary() const {
  return catenary_.weight_unit();
}

bool CatenaryCableLoader::IsUpdated() const {

  if (is_updated_catenary_ == true) {
    return true;
  } else {
    return false;
  }
}

double CatenaryCableLoader::LengthDifference(
    const double& tension_horizontal) const {

  // update the catenary and cable strainer load
  catenary_.set_tension_horizontal(tension_horizontal);
  strainer_cable_.set_load_finish(catenary_.TensionAverage());

  // catenary length
  const double length_catenary = catenary_.Length();

  // cable length
  const double length_cable = strainer_cable_.LengthFinish();

  return length_catenary - length_cable;
}

bool CatenaryCableLoader::Update() const {

  // update catenary
  if (is_updated_catenary_ == false) {

    is_updated_catenary_ = UpdateTensionHorizontal();
    if (is_updated_catenary_ == false) {
      return false;
    }
  }

  // if it reaches this point, update was successful
  return true;
}

bool CatenaryCableLoader::UpdateTensionHorizontal() const {

  // x = horizontal tension
  // y = length difference  i.e.(catenary length - cable length)

  // iterative routine to determine solution
  // solution reached when y = 0
  const double target_solution = 0;

  // declare and initialize left point
  // lowest acceptable value for catenary
  Point2d point_left;
  point_left.x = catenary_.ConstantMinimum(
      catenary_.spacing_endpoints().Magnitude());
  point_left.y = LengthDifference(point_left.x);

  // declare and initialize right point
  // 2x value of lowest point
  Point2d point_right;
  point_right.x = 2 * point_left.x;
  point_right.y = LengthDifference(point_right.x);

  // declare and initialize current point
  Point2d point_current;

  // iteration
  int iter = 0;
  const int iter_max = 100;
  while (0.01 < abs(point_left.x - point_right.x) && (iter < iter_max)) {

    // current point horizontal tension using left and right points
    if ((point_left.y > target_solution)
        && (target_solution > point_right.y)) {

      // average of left and right points
      point_current.x = (point_left.x + point_right.x) / 2;

    } else {

      // calculate tangent line between points, extrapolate using line
      double slope_line = (point_right.y - point_left.y)
          / (point_right.x - point_left.x);

      point_current.x = point_left.x + ((target_solution - point_left.y)
          / slope_line);

    }

    // current point length difference
    point_current.y = LengthDifference(point_current.x);

    // current point is left of left/right points
    if (point_current.x < point_left.x) {

      point_right.x = point_left.x;
      point_right.y = point_left.y;

      point_left.x = point_current.x;
      point_left.y = point_current.y;

    // current point is between left/right points
    } else if ((point_left.x < point_current.x) &&
        (point_current.x < point_right.x)) {

      if (point_current.y < target_solution) {
        point_right.x = point_current.x;
        point_right.y = point_current.y;
      } else if (target_solution < point_current.y) {
        point_left.x = point_current.x;
        point_left.y = point_current.y;
      }

    // current point is right of left/right points
    } else if (point_right.x < point_current.x) {

      point_left.x = point_right.x;
      point_left.y = point_right.y;

      point_right.x = point_current.x;
      point_right.y = point_current.y;

    }
  }

  // return success status and cache result
  if (iter < iter_max) {
    return true;
  } else {
    return false;
  }
}
