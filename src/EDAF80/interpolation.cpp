#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	glm::mat3x2 points = glm::mat3x2(p0.x, p1.x, p0.y, p1.y, p0.z, p1.z);
	glm::mat2 interp = glm::mat2(1.0f, -1.0f, 0.0f, 1.0f);
	glm::vec2 x_vec = glm::vec2(1.0f, x);
	glm::vec2 intermediate = x_vec * interp;
	glm::vec3 res = intermediate * points;
	//! \todo Implement this function
	return res;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	glm::mat3x4 points = glm::mat3x4(p0.x, p1.x, p2.x, p3.x,
		p0.y, p1.y, p2.y, p3.y,
		p0.z, p1.z, p2.z, p3.z);
	glm::mat4 catmull = glm::mat4(0.0f, -t, 2.0f*t, -t,
		1.0f, 0.0f, t - 3.0f, 2.0f - t,
		0.0f, t, 3.0f - 2.0f*t, t - 2.0f,
		0.0f, 0.0f, -t, t);
	glm::vec4 x_vec = glm::vec4(1.0f, x, x*x, x*x*x);
	//glm::mat3x4 intermediate = catmull * points;
	//glm::vec3 res = x_vec * intermediate;
	//! \todo Implement this function
	return x_vec*catmull*points;
}
