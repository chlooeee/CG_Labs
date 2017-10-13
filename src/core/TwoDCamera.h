#pragma once

#include "TRSTransform.h"
#include "InputHandler.h"

#include <glm/glm.hpp>

#include <iostream>

template<typename T, glm::precision P>
class TwoDCamera : public FPSCameraf
{
public:
	TwoDCamera(T fovy, T aspect, T nnear, T nfar) : FPSCameraf(fovy, aspect, nnear, nfar) {};

public:
	void Update(double dt, InputHandler &ih) override;
};

#include "TwoDCamera.inl"

typedef TwoDCamera<float, glm::defaultp> TwoDCameraf;
typedef TwoDCamera<double, glm::defaultp> TwoDCamerad;
