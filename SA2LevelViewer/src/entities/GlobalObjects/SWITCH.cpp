#include <glad/glad.h>
#include <cstring>
#include <Windows.h>

#include "../entity.h"
#include "../../toolbox/vector.h"
#include "switch.h"
#include "../../models/texturedmodel.h"
#include "../../loading/objLoader.h"
#include "../../loading/levelloader.h"
#include "../../main/main.h"
#include "../../collision/collisionmodel.h"
#include "../../collision/collisionchecker.h"
#include "../../toolbox/maths.h"
#include "../dummy.h"
#include "../unknown.h"

#include <list>



std::list<TexturedModel*> SWITCH::models;
CollisionModel* SWITCH::cmBase;

SWITCH::SWITCH()
{

}

void SWITCH::cleanUp()
{
    if (collideModelTransformed != nullptr)
    {
        CollisionChecker::deleteCollideModel(collideModelTransformed);
        collideModelTransformed = nullptr;
    }
    
    unhighlight();
}

SWITCH::SWITCH(char data[32], bool useDefaultValues)
{
    std::memcpy(rawData, data, 32);

    ID = data[1];

    signed short rX;
    signed short rY;
    signed short rZ;

    char* ptr = (char*)(&rX);
    memset(ptr, data[3], 1);
    memset(ptr + 1, data[2], 1);

    ptr = (char*)(&rY);
    memset(ptr, data[5], 1);
    memset(ptr + 1, data[4], 1);

    ptr = (char*)(&rZ);
    memset(ptr, data[7], 1);
    memset(ptr + 1, data[6], 1);

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

    char* v1 = (char*)&scaleX;
    v1[3] = data[20];
    v1[2] = data[21];
    v1[1] = data[22];
    v1[0] = data[23];
    type = (int)scaleX;

    char* v2 = (char*)&scaleY;
    v2[3] = data[24];
    v2[2] = data[25];
    v2[1] = data[26];
    v2[0] = data[27];
    triggerID = (int)scaleY;

    float var3;
    char* v3 = (char*)&var3;
    v3[3] = data[28];
    v3[2] = data[29];
    v3[1] = data[30];
    v3[0] = data[31];
    activeFrames = var3;

    if (useDefaultValues)
    {
        rotationX = 0;
        rotationY = 0;
        rotationZ = 0;
        type = 0;
        triggerID = 0;
        activeFrames = 0.0f;
    }

    scaleX = 1;
    scaleY = 1;
    scaleZ = 1;
    updateID();
    visible = true;
    baseColour.set(1, 1, 1);
    updateTransformationMatrixYXZ();

    collideModelOriginal = SWITCH::cmBase;
    collideModelTransformed = SWITCH::cmBase->duplicateMe();
    collideModelTransformed->parent = this;
    CollisionChecker::addCollideModel(collideModelTransformed);
    updateCollisionModelYXZ();
}

int SWITCH::getSwitchID() {
    return triggerID;
}

bool SWITCH::isSA2Object()
{
    return true;
}

void SWITCH::step()
{
    if (Global::selectedSA2Object == this)
    {
        baseColour.set(1.75f, 1.75f, 1.75f);
    }
    else
    {
        baseColour.set(1.0f, 1.0f, 1.0f);
        if (linkedObjects.size() > 0)
        {
            unhighlight();
            Global::redrawWindow = true;
        }
    }
}

std::list<TexturedModel*>* SWITCH::getModels()
{
    return &SWITCH::models;
}

void SWITCH::loadStaticModels()
{
    if (SWITCH::models.size() > 0)
    {
        return;
    }

    #ifdef DEV_MODE
    std::fprintf(stdout, "Loading SWITCH static models...\n");
    #endif

    loadModel(&SWITCH::models, Global::dirProgRoot + "res/Models/GlobalObjects/Switch/", "Switch");

    if (SWITCH::cmBase == nullptr)
    {
        SWITCH::cmBase = loadCollisionModel(Global::dirProgRoot + "res/Models/GlobalObjects/Switch/", "Switch");
    }
}

void SWITCH::deleteStaticModels()
{
#ifdef DEV_MODE
    std::fprintf(stdout, "Deleting SWITCH static models...\n");
#endif

    Entity::deleteModels(&SWITCH::models);
    Entity::deleteCollisionModel(&SWITCH::cmBase);
}

void SWITCH::updateValue(int btnIndex)
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
                char data[32] = { 0 };
                data[1] = (char)newid;

                data[8] = *(((char*)&position.x) + 3);
                data[9] = *(((char*)&position.x) + 2);
                data[10] = *(((char*)&position.x) + 1);
                data[11] = *(((char*)&position.x) + 0);
                data[12] = *(((char*)&position.y) + 3);
                data[13] = *(((char*)&position.y) + 2);
                data[14] = *(((char*)&position.y) + 1);
                data[15] = *(((char*)&position.y) + 0);
                data[16] = *(((char*)&position.z) + 3);
                data[17] = *(((char*)&position.z) + 2);
                data[18] = *(((char*)&position.z) + 1);
                data[19] = *(((char*)&position.z) + 0);

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
                    return;
                }
            }
            break;
        }
        catch (...) { break; }
    }

    case 2:
    {
        try
        {
            float newX = std::stof(text);
            position.x = newX;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
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
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
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
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
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
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
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
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
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
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 8:
    {
        try
        {
            int newVar1 = std::stoi(text);
            type = newVar1;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 9:
    {
        try
        {
            int newVar2 = std::stoi(text);
            triggerID = newVar2;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            updateID();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    case 10:
    {
        try
        {
            float newVar3 = std::stof(text);
            activeFrames = newVar3;
            updateTransformationMatrixYXZ();
            updateCollisionModelYXZ();
            Global::redrawWindow = true;
            break;
        }
        catch (...) { break; }
    }

    default: break;
    }

    highlight();
}

void SWITCH::updateID() {
    linkedObjects.clear();
    /*for (SA2Object* object in Global::Entities) {
        if([object is connected with a switch] && object.switchID == triggerID){
            linkedObjects.push_back(object);
        }    
    }*/
}

void SWITCH::highlight() {
    for (SA2Object* object : linkedObjects)
    {
        object->baseColour.set(1.25f, 1.25f, 1.75f);
    }
}

void SWITCH::unhighlight() {
    for(SA2Object* object : linkedObjects)
    {
        object->baseColour.set(1.00f, 1.00f, 1.00f);
    }
}

void SWITCH::updateEditorWindows()
{




    updateTransformationMatrixYXZ();
    updateCollisionModelYXZ();
    highlight();
}

void SWITCH::fillData(char data[32])
{
    data[1] = (char)ID;

    data[2] = (char)((rotationX >> 8) & 0xFF);
    data[3] = (char)((rotationX >> 0) & 0xFF);
    data[4] = (char)((rotationY >> 8) & 0xFF);
    data[5] = (char)((rotationY >> 0) & 0xFF);
    data[6] = (char)((rotationZ >> 8) & 0xFF);
    data[7] = (char)((rotationZ >> 0) & 0xFF);

    char* ptr = (char*)(&position.x);
    data[8] = (char)(*(ptr + 3));
    data[9] = (char)(*(ptr + 2));
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

    int var1 = type;
    ptr = (char*)(&var1);
    data[20] = (char)(*(ptr + 3));
    data[21] = (char)(*(ptr + 2));
    data[22] = (char)(*(ptr + 1));
    data[23] = (char)(*(ptr + 0));

    int var2 = triggerID;
    ptr = (char*)(&var2);
    data[24] = (char)(*(ptr + 3));
    data[25] = (char)(*(ptr + 2));
    data[26] = (char)(*(ptr + 1));
    data[27] = (char)(*(ptr + 0));

    float var3 = activeFrames;
    ptr = (char*)(&var3);
    data[28] = (char)(*(ptr + 3));
    data[29] = (char)(*(ptr + 2));
    data[30] = (char)(*(ptr + 1));
    data[31] = (char)(*(ptr + 0));
}
