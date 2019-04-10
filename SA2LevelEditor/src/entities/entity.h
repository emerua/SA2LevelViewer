#ifndef ENTITIES_H
#define ENTITIES_H

class TexturedModel;
class CollisionModel;

#include <list>
#include "../toolbox/vector.h"
#include "../toolbox/matrix.h"
#include <string>


class Entity
{
protected:
	static void deleteModels(std::list<TexturedModel*>* modelsToDelete);
	static void deleteCollisionModel(CollisionModel** colModelToDelete);

public:
	Vector3f position;
	float rotationX, rotationY, rotationZ;
	float scale;
	bool visible;
	Vector3f baseColour;
	Matrix4f transformationMatrix;

	Entity();
	Entity(Vector3f* initialPosition);
	virtual ~Entity();

	virtual void step();

	void increasePosition(float dx, float dy, float dz);

	void increaseRotation(float dx, float dy, float dz);

	virtual std::list<TexturedModel*>* getModels();

	void setPosition(Vector3f* newPosition);
	void setPosition(float newX, float newY, float newZ);

	void setBaseColour(float red, float green, float blue);

	Matrix4f* getTransformationMatrix();

	void updateTransformationMatrix();

	void updateTransformationMatrix(float scaleX, float scaleY, float scaleZ);

	void updateTransformationMatrixSADX();
};
#endif
