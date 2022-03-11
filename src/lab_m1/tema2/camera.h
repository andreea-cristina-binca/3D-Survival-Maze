#pragma once

#include "utils/glm_utils.h"
#include "utils/math_utils.h"


namespace myNamespace
{
	class Camera
	{
	public:
		Camera()
		{
			position = glm::vec3(0, 2, 5);
			forward = glm::vec3(0, 0, -1);
			up = glm::vec3(0, 1, 0);
			right = glm::vec3(1, 0, 0);
			distanceToTarget = 2;
			pitch = 0;
		}

		Camera(const glm::vec3& position, const glm::vec3& center, const glm::vec3& up)
		{
			Set(position, center, up);
		}

		~Camera()
		{
		}

		// Update camera
		void Set(const glm::vec3& position, const glm::vec3& center, const glm::vec3& up)
		{
			this->position = position;
			forward = glm::normalize(center - position);
			right = glm::cross(forward, up);
			this->up = glm::cross(right, forward);
		}

		void MoveForward(float distance)
		{
			// Translates the camera using the `dir` vector computed from `forward`
			glm::vec3 dir = glm::normalize(glm::vec3(forward.x, 0, forward.z));
			position += dir * distance;
		}

		void TranslateForward(float distance)
		{
			// Translate the camera using the `forward` vector
			position += forward * distance;
		}

		void TranslateUpward(float distance)
		{
			// Translate the camera using the `up` vector
			position += up * distance;
		}

		void TranslateRight(float distance)
		{
			// Translate the camera using the `right` vector
			position += glm::normalize(right) * distance;
		}

		void RotateFirstPerson_OX(float angle)
		{
			if ((pitch + angle) < 1.5f && (pitch + angle) > -1.5f) {
				// Compute the new `forward` and `up` vectors
				glm::vec4 new_fwd = glm::rotate(glm::mat4(1.0f), angle, right) * glm::vec4(forward, 1);
				forward = glm::normalize(glm::vec3(new_fwd));

				up = glm::cross(right, forward);

				// Adjust the angle
				pitch += angle;
			}
		}

		void RotateFirstPerson_OY(float angle)
		{
			// Compute the new `forward`, `up` and `right` vectors
			glm::vec4 new_fwd = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) * glm::vec4(forward, 1);
			forward = glm::normalize(glm::vec3(new_fwd));

			glm::vec4 new_rgt = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) * glm::vec4(right, 1);
			right = glm::normalize(glm::vec3(new_rgt));

			up = glm::cross(right, forward);
		}

		void RotateFirstPerson_OZ(float angle)
		{
			// Compute the new `right` and `up` vectors
			glm::vec4 new_rgt = glm::rotate(glm::mat4(1.0f), angle, forward) * glm::vec4(right, 1);
			right = glm::normalize(glm::vec3(new_rgt));

			up = glm::cross(right, forward);
		}

		void RotateThirdPerson_OX(float angle)
		{
			if ((pitch + angle) < 0.5f && (pitch + angle) > -1.5f) {
				// Rotate the camera in third-person mode around the OX axis
				TranslateForward(distanceToTarget);
				RotateFirstPerson_OX(angle);
				TranslateForward(-distanceToTarget);
			}
		}

		void RotateThirdPerson_OY(float angle)
		{
			// Rotate the camera in third-person mode around the OY axis
			TranslateForward(distanceToTarget);
			RotateFirstPerson_OY(angle);
			TranslateForward(-distanceToTarget);
		}

		void RotateThirdPerson_OZ(float angle)
		{
			// Rotate the camera in third-person mode around the OZ axis
			TranslateForward(distanceToTarget);
			RotateFirstPerson_OZ(angle);
			TranslateForward(-distanceToTarget);
		}

		glm::mat4 GetViewMatrix()
		{
			// Returns the view matrix
			return glm::lookAt(position, position + forward, up);
		}

		glm::vec3 GetTargetPosition()
		{
			return position + forward * distanceToTarget;
		}

	public:
		float distanceToTarget;
		glm::vec3 position;
		glm::vec3 forward;
		glm::vec3 right;
		glm::vec3 up;
		float pitch;
	};
}   // namespace implemented
