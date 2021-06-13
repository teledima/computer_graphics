#include "arcball_camera.h"

ArcballCamera::ArcballCamera(const glm::vec3 & center, const glm::vec3 & upVector, float radius, float minRadius, float azimuthAngle, float polarAngle)
	:center_(center), upVector_(upVector), radius_(radius), minRadius_(minRadius), azimuthAngle_(azimuthAngle), polarAngle_(polarAngle)
{
}

void ArcballCamera::rotateAzimuth(const float radians)
{
	azimuthAngle_ += radians;

	// Keep azimuth angle within range <0..2PI) - it's not necessary, just to have it nicely output
	constexpr float fullCircle = 2.0f * glm::pi<float>();
	azimuthAngle_ = fmodf(azimuthAngle_, fullCircle);
	if (azimuthAngle_ < 0.0f) {
		azimuthAngle_ = fullCircle + azimuthAngle_;
	}
}

void ArcballCamera::rotatePolar(const float radians)
{
	polarAngle_ += radians;

	// Check if the angle hasn't exceeded quarter of a circle to prevent flip, add a bit of epsilon like 0.001 radians
	constexpr float polarCap = glm::pi<float>() / 2.0f - 0.001f;
	if (polarAngle_ > polarCap) {
		polarAngle_ = polarCap;
	}

	if (polarAngle_ < -polarCap) {
		polarAngle_ = -polarCap;
	}
}

glm::mat4 ArcballCamera::getViewMatrix() const
{
	return glm::lookAt(getEye(), center_, upVector_);
}

glm::vec3 ArcballCamera::getEye() const
{
	// Calculate sines / cosines of angles
	const float sineAzimuth = sin(azimuthAngle_);
	const float cosineAzimuth = cos(azimuthAngle_);
	const float sinePolar = sin(polarAngle_);
	const float cosinePolar = cos(polarAngle_);

	// Calculate eye position out of them
	const float x = center_.x + radius_ * cosinePolar * cosineAzimuth;
	const float y = center_.y + radius_ * sinePolar;
	const float z = center_.z + radius_ * cosinePolar * sineAzimuth;

	return glm::vec3(x, y, z);
}

glm::vec3 ArcballCamera::getViewPoint() const
{
	return center_;
}

glm::vec3 ArcballCamera::getUpVector() const
{
	return upVector_;
}

float ArcballCamera::getAzimuthAngle() const
{
	return azimuthAngle_;
}

float ArcballCamera::getPolarAngle() const
{
	return polarAngle_;
}

float ArcballCamera::getRadius() const
{
	return radius_;
}
