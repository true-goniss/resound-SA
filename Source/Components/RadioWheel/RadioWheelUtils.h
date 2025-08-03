#pragma once

/*
	gon_iss (c) 2025
*/

#include "RadioWheelParams.h"

float GetAngle(float x1, float y1, float x2, float y2) {
	return atan2(-(y2 - y1), x2 - x1) * (180.0f / M_PI); // Negative for inverted Y-axis
}

float NormalizeAngle(float angle) {
	while (angle < 0.0f) angle += 360.0f;
	while (angle >= 360.0f) angle -= 360.0f;
	return angle;
}

float CalculateCircleRadius(float iconSize, float spacing, int quantity) {
	if (quantity <= 0) return 0.0f;

	float totalCircumference = quantity * (iconSize + spacing);
	float calculatedRadius = totalCircumference / (2.0f * M_PI);

	float minRadius = RsGlobal.maximumHeight / minRadiusScreenHeightRelation;

	return max(minRadius, calculatedRadius);
}

int GetSelectedSectorCircular(
	float mouseX, float mouseY, float centerX, float centerY, float radius,
	int numIcons, float& lastMouseAngle, int selectedIndex, float startAngleDegrees = 270.0f,
	float angleThreshold = 15.0f) {

	if (numIcons <= 0) return 0; // Invalid input

	float deltaX = mouseX - centerX;
	float deltaY = mouseY - centerY;

	float distanceSquared = deltaX * deltaX + deltaY * deltaY;
	float radiusSquared = radius * radius;
	float nearCenterRadiusSquared = radiusSquared / 2;
	float atTheCenterRadiusSquared = radiusSquared / 6;

	if (distanceSquared <= atTheCenterRadiusSquared) {
		return selectedIndex;
	}

	// Check if the mouse is near the center of the circle
	if (distanceSquared <= nearCenterRadiusSquared) {
		// If the mouse is near the center, snap to the circle

		float angle = atan2(deltaY, deltaX); // Calculate angle from center to mouse

		// Snap to circle
		mouseX = centerX + radius * 10 * cos(angle); 
		mouseY = centerY + radius * 10 * sin(angle);

	}
	// Calculate the mouse angle relative to the center
	float currentMouseAngle = GetAngle(centerX, centerY, mouseX, mouseY);
	currentMouseAngle = NormalizeAngle(currentMouseAngle - startAngleDegrees);

	// Determine if the mouse has moved enough to trigger a sector change
	float angleDifference = NormalizeAngle(currentMouseAngle - lastMouseAngle);
	if (fabs(angleDifference) < angleThreshold) {
		return selectedIndex; // No significant movement
	}

	// Calculate the angle step for each sector
	float angleStep = 360.0f / numIcons;

	// Determine the selected sector based on the current angle
	selectedIndex = static_cast<int>(currentMouseAngle / angleStep);

	lastMouseAngle = currentMouseAngle;

	return selectedIndex;
}