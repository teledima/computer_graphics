#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ArcballCamera {
public:
	ArcballCamera(const glm::vec3& center, const glm::vec3& upVector, float radius, float minRadius, float azimuthAngle, float polarAngle);

    void rotateAzimuth(const float radians);
    void rotatePolar(const float radians);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getEye() const;
    glm::vec3 getViewPoint() const;
    glm::vec3 getUpVector() const;
    glm::vec3 getNormalizedViewVector() const;
    float getAzimuthAngle() const;
    float getPolarAngle() const;
    float getRadius() const;

private:
    glm::vec3 center_; // Center of the orbit camera sphere (the point upon which the camera looks)
    glm::vec3 upVector_; // Up vector of the camera
    float radius_; // Radius of the orbit camera sphere
    float minRadius_; // Minimal radius of the orbit camera sphere (cannot fall below this value)
    float azimuthAngle_; // Azimuth angle on the orbit camera sphere
    float polarAngle_; // Polar angle on the orbit camera sphere
};