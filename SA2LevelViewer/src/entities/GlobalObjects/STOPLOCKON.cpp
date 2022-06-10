#include <glad/glad.h>
#include <cstring>
#include <Windows.h>

#include "../entity.h"
#include "../../toolbox/vector.h"
#include "stoplockon.h"
#include "../../models/texturedmodel.h"
#include "../../loading/objLoader.h"
#include "../../loading/levelloader.h"
#include "../../main/main.h"
#include "../../collision/collisionmodel.h"
#include "../../collision/collisionchecker.h"
#include "../../toolbox/maths.h"
#include "../../toolbox/hex.h"
#include "../dummy.h"

#include <list>



std::list<TexturedModel*> STOPLOCKON::models;
CollisionModel* STOPLOCKON::cmBase;

STOPLOCKON::STOPLOCKON()
{

}

STOPLOCKON::STOPLOCKON(char data[32], bool useDefaultValues)
{
    std::memcpy(rawData, data, 32);

    ID = data[1];

    signed short rX = 0;
    signed short rY = 0;
    signed short rZ = 0;

    char* ptr = (char*)(&rX);
    memset(ptr, data[3], 1);
    memset(ptr+1, data[2], 1);

    ptr = (char*)(&rY);
    memset(ptr, data[5], 1);
    memset(ptr+1, data[4], 1);

    ptr = (char*)(&rZ);
    memset(ptr, data[7], 1);
    memset(ptr+1, data[6], 1);

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

    char* v1 = (char*)&var1;
    v1[3] = data[20];
    v1[2] = data[21];
    v1[1] = data[22];
    v1[0] = data[23];

    char* v2 = (char*)&var2;
    v2[3] = data[24];
    v2[2] = data[25];
    v2[1] = data[26];
    v2[0] = data[27];

    char* v3 = (char*)&var3;
    v3[3] = data[28];
    v3[2] = data[29];
    v3[1] = data[30];
    v3[0] = data[31];

    if (useDefaultValues)
    {
        var1 = 10.0f;
        var2 = 10.0f;
        var3 = 10.0f;
    }

    scaleX = 0;
    scaleY = 0;
    scaleZ = 0;
    rotationX = 0;
    rotationY = 0;
    rotationZ = 0; 
    visible = true;
    updateTransformationMatrixZY();

    collideModelOriginal = STOPLOCKON::cmBase;
    collideModelTransformed = STOPLOCKON::cmBase->duplicateMe();
    collideModelTransformed->parent = this;
    CollisionChecker::addCollideModel(collideModelTransformed);
    updateCollisionModelZY(var1, var2, var3);

    box = nullptr;
    box = new Dummy(&STOPLOCKON::models); INCR_NEW("Entity");
    box->setPosition(&position);
    box->setRotation(0, 0, 0);
    box->setScale(var1, var2, var3);
    box->visible = true;
    box->baseColour.set(0, 1, 1);
    box->updateTransformationMatrixZY();
    Global::addTransparentEntity(box);
}

void STOPLOCKON::cleanUp()
{
    if (collideModelTransformed != nullptr)
    {
        CollisionChecker::deleteCollideModel(collideModelTransformed);
        collideModelTransformed = nullptr;
    }
    
    if (box != nullptr)
    {
        Global::deleteTransparentEntity(box);
        box = nullptr;
    }
}

bool STOPLOCKON::isSA2Object()
{
    return true;
}

void STOPLOCKON::step()
{
    if (Global::selectedSA2Object == this)
    {
        box->baseColour.set(0.75f, 1.75f, 1.75f);
    }
    else
    {
        box->baseColour.set(0.0f, 1.0f, 1.0f);
    }
}

std::list<TexturedModel*>* STOPLOCKON::getModels()
{
    return &STOPLOCKON::models;
}

void STOPLOCKON::loadStaticModels()
{
    if (STOPLOCKON::models.size() > 0)
    {
        return;
    }

    #ifdef DEV_MODE
    std::fprintf(stdout, "Loading STOPLOCKON static models...\n");
    #endif

    loadModel(&STOPLOCKON::models, Global::dirProgRoot + "res/Models/GlobalObjects/LightVolumes/", "LightBox");

    if (STOPLOCKON::cmBase == nullptr)
    {
        STOPLOCKON::cmBase = loadCollisionModel(Global::dirProgRoot + "res/Models/GlobalObjects/LightVolumes/", "LightBox");
    }
}

void STOPLOCKON::deleteStaticModels()
{
    #ifdef DEV_MODE
    std::fprintf(stdout, "Deleting STOPLOCKON static models...\n");
    #endif

    Entity::deleteModels(&STOPLOCKON::models);
    Entity::deleteCollisionModel(&STOPLOCKON::cmBase);
}

void STOPLOCKON::updateValue(int btnIndex)
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
                    cleanUp();
                    Global::redrawWindow = true;
                    Global::deleteEntity(this);
                    return;
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

    case 8:
    {
        try
        {
            float newVar1 = std::stof(text);
            var1 = newVar1;
            break;
        }
        catch (...) { break; }
    }

    case 9:
    {
        try
        {
            float newVar2 = std::stof(text);
            var2 = newVar2;
            break;
        }
        catch (...) { break; }
    }

    case 10:
    {
        try
        {
            float newVar3 = std::stof(text);
            var3 = newVar3;
            break;
        }
        catch (...) { break; }
    }
    
    default: break;
    }

    updateCollisionModelZY(var1, var2, var3);
    Global::redrawWindow = true;
    box->setPosition(&position);
    box->setScale(var1, var2, var3);
    box->updateTransformationMatrixZY();
}

void STOPLOCKON::updateEditorWindows()
{




    updateCollisionModelZY(var1, var2, var3);
    box->setPosition(&position);
    box->setScale(var1, var2, var3);
    box->updateTransformationMatrixZY();
}

void STOPLOCKON::fillData(char data[32])
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

    ptr = (char*)(&var1);
    data[20] = (char)(*(ptr + 3));
    data[21] = (char)(*(ptr + 2));
    data[22] = (char)(*(ptr + 1));
    data[23] = (char)(*(ptr + 0));

    ptr = (char*)(&var2);
    data[24] = (char)(*(ptr + 3));
    data[25] = (char)(*(ptr + 2));
    data[26] = (char)(*(ptr + 1));
    data[27] = (char)(*(ptr + 0));

    ptr = (char*)(&var3);
    data[28] = (char)(*(ptr + 3));
    data[29] = (char)(*(ptr + 2));
    data[30] = (char)(*(ptr + 1));
    data[31] = (char)(*(ptr + 0));
}
