#pragma once

namespace Zero
{

void BuildOrthographicTransformZero(Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance);
void BuildOrthographicTransformGl(Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance);
void BuildOrthographicTransformDx(Mat4& matrix, float verticalSize, float aspectRatio, float nearDistance, float farDistance);

void BuildPerspectiveTransformZero(Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance);
void BuildPerspectiveTransformGl(Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance);
void BuildPerspectiveTransformDx(Mat4& matrix, float verticalFov, float aspectRatio, float nearDistance, float farDistance);

} // namespace Zero
