#include <glad/glad.h>

#include "../loading/objLoader.h"
#include "entity.h"
#include "../models/texturedmodel.h"
#include "stage.h"
#include "../loading/loader.h"
#include "../main/main.h"

#include <list>

std::list<TexturedModel*> Stage::models;

Stage::Stage()
{
	position.x = 0;
	position.y = 0;
	position.z = 0;
	rotationX = 0;
	rotationY = 0;
	rotationZ = 0;
	scale = 1;
	visible = true;
	
	updateTransformationMatrix();
}

void Stage::step()
{

}

std::list<TexturedModel*>* Stage::getModels()
{
	return &Stage::models;
}

void Stage::loadModels(const char* objFolder, const char* objFilename)
{
    if (Stage::models.size() == 0)
	{
		#ifdef DEV_MODE
		std::fprintf(stdout, "Loading Stage static models...\n");
		#endif

		loadModel(&Stage::models, objFolder, objFilename);
	}
}

void Stage::deleteStaticModels()
{
	#ifdef DEV_MODE
	std::fprintf(stdout, "Deleting Stage static models...\n");
	#endif

	Entity::deleteModels(&Stage::models);
}
