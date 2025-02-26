#include <Windows.h>
#include <glad/glad.h>
#include <cstring>

#include "../../entity.h"
#include "../../dummy.h"
#include "../../../toolbox/vector.h"
#include "spiderweb.h"
#include "../../../models/texturedmodel.h"
#include "../../../loading/objLoader.h"
#include "../../../loading/levelloader.h"
#include "../../../main/main.h"
#include "../../../collision/collisionmodel.h"
#include "../../../collision/collisionchecker.h"
#include "../../../toolbox/maths.h"

#include <list>

std::list<TexturedModel*> SPIDERWEB::models;
CollisionModel* SPIDERWEB::cmBase;

SPIDERWEB::SPIDERWEB()
{

}

void SPIDERWEB::cleanUp()
{
    if (collideModelTransformed != nullptr)
    {
        CollisionChecker::deleteCollideModel(collideModelTransformed);
        collideModelTransformed = nullptr;
    }

    if (web != nullptr)
    {
        Global::deleteTransparentEntity(web);
        web = nullptr;
    }
}

SPIDERWEB::SPIDERWEB(char data[32], bool useDefaultValues)
{
    std::memcpy(rawData, data, 32);

    ID = data[1];

    signed short rX;
    signed short rY;
    signed short rZ;

    char* ptr = (char*)(&rX);
    memset(ptr, data[3], 1);
    memset(ptr+1, data[2], 1);

    ptr = (char*)(&rY);
    memset(ptr, data[5], 1);
    memset(ptr+1, data[4], 1);

    ptr = (char*)(&rZ);
    memset(ptr, data[7], 1);
    memset(ptr+1, data[6], 1);

    rotationX = (int)rX;
    rotationY = (int)rY;
    rotationZ = (int)rZ;

    char* x = (char*)&position.x;
    x[3] = data[8];
    x[2] = data[9];
    x[1] = data[10];
    x[0] = data[11];

    char* y = (char*)&position.y;
    y[3] = data[12];
    y[2] = data[13];
    y[1] = data[14];
    y[0] = data[15];

    char* z = (char*)&position.z;
    z[3] = data[16];
    z[2] = data[17];
    z[1] = data[18];
    z[0] = data[19];
    
    float var1;
    char* v1 = (char*)&var1;
    v1[3] = data[20];
    v1[2] = data[21];
    v1[1] = data[22];
    v1[0] = data[23];

    scaleX = var1 + 1.0f; 
    scaleY = var1 + 1.0f;
    scaleZ = var1 + 1.0f;

    if (useDefaultValues)
    {
        scaleX = 1.0f; 
        scaleY = 1.0f;
        scaleZ = 1.0f;
    }

    visible = false;
    baseColour.set(1, 1, 1);
    updateTransformationMatrixZXY();

    collideModelOriginal = SPIDERWEB::cmBase;
    collideModelTransformed = SPIDERWEB::cmBase->duplicateMe();
    collideModelTransformed->parent = this;
    CollisionChecker::addCollideModel(collideModelTransformed);
    updateCollisionModelZXY();

    web = new Dummy(&SPIDERWEB::models); INCR_NEW("Entity");
    web->visible = true;
    Global::addTransparentEntity(web);
    web->setPosition(&position);
    web->setRotation(rotationX, rotationY, rotationZ);
    web->setScale(scaleX, scaleX, scaleX);
    web->updateTransformationMatrixZXY();
}

bool SPIDERWEB::isSA2Object()
{
    return true;
}

void SPIDERWEB::step()
{
    if (Global::selectedSA2Object == this)
    {
        web->baseColour.set(1.75f, 1.75f, 1.75f);
    }
    else
    {
        web->baseColour.set(1.0f, 1.0f, 1.0f);
    }
}

std::list<TexturedModel*>* SPIDERWEB::getModels()
{
    return &SPIDERWEB::models;
}

void SPIDERWEB::loadStaticModels()
{
    if (SPIDERWEB::models.size() > 0)
    {
        return;
    }

    #ifdef DEV_MODE
    std::fprintf(stdout, "Loading SPIDERWEB static models...\n");
    #endif

    loadModel(&SPIDERWEB::models, Global::dirProgRoot + "res/Models/LevelObjects/PyramidCave/Spiderweb/", "Spiderweb");

    if (SPIDERWEB::cmBase == nullptr)
    {
        SPIDERWEB::cmBase = loadCollisionModel(Global::dirProgRoot + "res/Models/LevelObjects/PyramidCave/Spiderweb/", "Spiderweb");
    }
}

void SPIDERWEB::deleteStaticModels()
{
    #ifdef DEV_MODE
    std::fprintf(stdout, "Deleting SPIDERWEB static models...\n");
    #endif

    Entity::deleteModels(&SPIDERWEB::models);
    Entity::deleteCollisionModel(&SPIDERWEB::cmBase);
}

void SPIDERWEB::updateValue(int btnIndex)
{
    char buf[128];
    std::string text = buf;

    switch (btnIndex)
    {
    case 0:
    {
        try
        {
            //we are going to change into a new object.
            int newid = std::stoi(text);

            if (newid != ID)
            {
                char data[32] = {0};
                data[ 1] = (char)newid;

                data[ 8] = *(((char*)&position.x)+3);
                data[ 9] = *(((char*)&position.x)+2);
                data[10] = *(((char*)&position.x)+1);
                data[11] = *(((char*)&position.x)+0);
                data[12] = *(((char*)&position.y)+3);
                data[13] = *(((char*)&position.y)+2);
                data[14] = *(((char*)&position.y)+1);
                data[15] = *(((char*)&position.y)+0);
                data[16] = *(((char*)&position.z)+3);
                data[17] = *(((char*)&position.z)+2);
                data[18] = *(((char*)&position.z)+1);
                data[19] = *(((char*)&position.z)+0);

                SA2Object* newObject = LevelLoader::newSA2Object(Global::levelID, newid, data, true);
                if (newObject != nullptr)
                {
                    newObject->lvlLineNum = lvlLineNum;
                    Global::addEntity(newObject);
                    Global::selectedSA2Object = newObject;
                    newObject->updateEditorWindows();
                    Global::redrawWindow = true;
                    cleanUp();
                    Global::deleteEntity(this);
                }
            }
        }
        catch (...) { }
        break;
    }

    case 2:
    {
        try
        {
            float newX = std::stof(text);
            position.x = newX;
            break;
        }
        catch (...) { break; }
    }

    case 3:
    {
        try
        {
            float newY = std::stof(text);
            position.y = newY;
            break;
        }
        catch (...) { break; }
    }

    case 4:
    {
        try
        {
            float newZ = std::stof(text);
            position.z = newZ;
            break;
        }
        catch (...) { break; }
    }

    case 5:
    {
        try
        {
            int newRotX = std::stoi(text);
            rotationX = newRotX;
            break;
        }
        catch (...) { break; }
    }

    case 6:
    {
        try
        {
            int newRotY = std::stoi(text);
            rotationY = newRotY;
            break;
        }
        catch (...) { break; }
    }

    case 7:
    {
        try
        {
            int newRotZ = std::stoi(text);
            rotationZ = newRotZ;
            break;
        }
        catch (...) { break; }
    }

    case 8:
    {
        try
        {
            float newVar1 = std::stof(text);
            scaleX = newVar1;
            scaleY = newVar1;
            scaleZ = newVar1;
            break;
        }
        catch (...) { break; }
    }
    
    default: break;
    }

    updateTransformationMatrixZXY();
    updateCollisionModelZXY();
    web->setPosition(&position);
    web->setRotation(rotationX, rotationY, rotationZ);
    web->setScale(scaleX, scaleX, scaleX);
    web->updateTransformationMatrixZXY();
    Global::redrawWindow = true;
}

void SPIDERWEB::updateEditorWindows()
{




    updateTransformationMatrixZXY();
    updateCollisionModelZXY();
}

void SPIDERWEB::fillData(char data[32])
{
    data[1] = (char)ID;

    data[2] = (char)((rotationX >> 8) & 0xFF);
    data[3] = (char)((rotationX >> 0) & 0xFF);
    data[4] = (char)((rotationY >> 8) & 0xFF);
    data[5] = (char)((rotationY >> 0) & 0xFF);
    data[6] = (char)((rotationZ >> 8) & 0xFF);
    data[7] = (char)((rotationZ >> 0) & 0xFF);

    char* ptr = (char*)(&position.x);
    data[ 8] = (char)(*(ptr + 3));
    data[ 9] = (char)(*(ptr + 2));
    data[10] = (char)(*(ptr + 1));
    data[11] = (char)(*(ptr + 0));

    ptr = (char*)(&position.y);
    data[12] = (char)(*(ptr + 3));
    data[13] = (char)(*(ptr + 2));
    data[14] = (char)(*(ptr + 1));
    data[15] = (char)(*(ptr + 0));

    ptr = (char*)(&position.z);
    data[16] = (char)(*(ptr + 3));
    data[17] = (char)(*(ptr + 2));
    data[18] = (char)(*(ptr + 1));
    data[19] = (char)(*(ptr + 0));

    float var1 = scaleX - 1.0f;
    ptr = (char*)(&var1);
    data[20] = (char)(*(ptr + 3));
    data[21] = (char)(*(ptr + 2));
    data[22] = (char)(*(ptr + 1));
    data[23] = (char)(*(ptr + 0));
}
